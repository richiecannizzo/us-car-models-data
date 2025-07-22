#!/usr/bin/env python3
"""
Convert a Demucs model to TorchScript format for use with StemMixer VST3 plugin.

This script loads a pre-trained Demucs model and traces it to create a TorchScript
version that can be used by the C++ plugin.

Requirements:
    pip install torch demucs

Usage:
    python convert_demucs_to_torchscript.py --model htdemucs --output htdemucs_traced.pt
"""

import argparse
import torch
import torch.nn as nn
from demucs.pretrained import get_model
from demucs.apply import apply_model
import warnings

class DemucsWrapper(nn.Module):
    """Wrapper around Demucs model to ensure fixed input/output shapes."""
    
    def __init__(self, model):
        super().__init__()
        self.model = model
        self.sample_rate = 44100
        self.chunk_length = 44100  # 1 second at 44.1kHz
        
    def forward(self, x):
        """
        Forward pass with fixed shapes.
        
        Input: [1, 2, 44100] - batch_size=1, channels=2, samples=44100
        Output: [1, 4, 2, 44100] - batch_size=1, stems=4, channels=2, samples=44100
        """
        # Ensure input is the right shape
        assert x.shape == (1, 2, self.chunk_length), f"Expected input shape (1, 2, {self.chunk_length}), got {x.shape}"
        
        # Apply the model
        with torch.no_grad():
            # Demucs expects [batch, channels, samples]
            separated = apply_model(self.model, x, device=x.device, progress=False, num_workers=0)
            
            # separated shape: [batch, stems, channels, samples]
            # Ensure output shape is correct
            batch_size, n_stems, n_channels, n_samples = separated.shape
            assert batch_size == 1, f"Expected batch size 1, got {batch_size}"
            assert n_stems == 4, f"Expected 4 stems, got {n_stems}"
            assert n_channels == 2, f"Expected 2 channels, got {n_channels}"
            assert n_samples == self.chunk_length, f"Expected {self.chunk_length} samples, got {n_samples}"
            
        return separated

def main():
    parser = argparse.ArgumentParser(description='Convert Demucs model to TorchScript')
    parser.add_argument('--model', type=str, default='htdemucs', 
                       help='Demucs model name (default: htdemucs)')
    parser.add_argument('--output', type=str, default='htdemucs_traced.pt',
                       help='Output TorchScript file (default: htdemucs_traced.pt)')
    parser.add_argument('--device', type=str, default='cpu',
                       help='Device to use (cpu/cuda, default: cpu)')
    
    args = parser.parse_args()
    
    print(f"Loading Demucs model: {args.model}")
    
    # Load the pre-trained model
    try:
        model = get_model(args.model)
        model.eval()
        print(f"✅ Model loaded successfully")
    except Exception as e:
        print(f"❌ Failed to load model: {e}")
        return 1
    
    # Move to specified device
    device = torch.device(args.device)
    model = model.to(device)
    
    # Create wrapper
    wrapped_model = DemucsWrapper(model)
    wrapped_model.eval()
    
    print(f"Wrapping model for fixed input/output shapes...")
    
    # Create example input (1 second of stereo audio at 44.1kHz)
    sample_rate = 44100
    chunk_length = 44100  # 1 second
    example_input = torch.randn(1, 2, chunk_length, device=device)
    
    print(f"Creating example input: {example_input.shape}")
    
    # Test the wrapped model
    print("Testing wrapped model...")
    try:
        with torch.no_grad():
            output = wrapped_model(example_input)
            print(f"✅ Test successful - Output shape: {output.shape}")
            
            # Verify output shape
            expected_shape = (1, 4, 2, chunk_length)
            if output.shape != expected_shape:
                print(f"❌ Unexpected output shape: {output.shape}, expected: {expected_shape}")
                return 1
                
    except Exception as e:
        print(f"❌ Test failed: {e}")
        return 1
    
    # Trace the model
    print("Tracing model to TorchScript...")
    try:
        with warnings.catch_warnings():
            warnings.simplefilter("ignore")
            traced_model = torch.jit.trace(wrapped_model, example_input)
        print(f"✅ Model traced successfully")
    except Exception as e:
        print(f"❌ Tracing failed: {e}")
        return 1
    
    # Test the traced model
    print("Testing traced model...")
    try:
        with torch.no_grad():
            traced_output = traced_model(example_input)
            print(f"✅ Traced model test successful - Output shape: {traced_output.shape}")
    except Exception as e:
        print(f"❌ Traced model test failed: {e}")
        return 1
    
    # Save the traced model
    print(f"Saving traced model to: {args.output}")
    try:
        traced_model.save(args.output)
        print(f"✅ Model saved successfully")
    except Exception as e:
        print(f"❌ Failed to save model: {e}")
        return 1
    
    # Verify the saved model can be loaded
    print("Verifying saved model...")
    try:
        loaded_model = torch.jit.load(args.output, map_location=device)
        with torch.no_grad():
            test_output = loaded_model(example_input)
            print(f"✅ Saved model verification successful - Output shape: {test_output.shape}")
    except Exception as e:
        print(f"❌ Saved model verification failed: {e}")
        return 1
    
    print(f"""
🎉 Success! TorchScript model created: {args.output}

Model Details:
- Input shape: [1, 2, {chunk_length}] (1 second stereo at 44.1kHz)
- Output shape: [1, 4, 2, {chunk_length}] (4 stems: vocals, drums, bass, other)
- Device: {device}
- File size: {torch.load(args.output, map_location='cpu').code.__sizeof__() / 1024 / 1024:.1f} MB (code only)

Usage:
1. Copy {args.output} to your StemMixer Assets/ directory
2. Build and run the StemMixer VST3 plugin
3. The plugin will automatically load this model for real-time stem separation

Note: Make sure to place the model file in the correct location for the plugin to find it.
""")
    
    return 0

if __name__ == '__main__':
    exit(main())