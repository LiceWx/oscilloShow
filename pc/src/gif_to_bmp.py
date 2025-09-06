#!/usr/bin/env python3

import os
import sys
import struct
from PIL import Image

def gif_to_bmp_frames(gif_path, output_dir="D:/OscilloProj/frames"):
    """Convert GIF to BMP frames"""
    
    # Create output directory
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    
    # Create SDfiles directory if it doesn't exist
    if not os.path.exists("D:/OscilloProj/SDfiles"):
        os.makedirs("D:/OscilloProj/SDfiles")
    
    try:
        # Open GIF file
        gif = Image.open(gif_path)
        
        # Get frame duration and calculate FPS
        try:
            frame_duration = gif.info.get('duration', 100)  # Default 100ms if not specified
            fps = 1000.0 / frame_duration  # Convert from ms to fps
        except:
            fps = 10.0  # Default fps
        
        frame_count = 0
        
        # Extract each frame
        while True:
            try:
                # Convert frame to RGB if necessary
                frame = gif.convert('RGB')
                
                # Save as BMP
                frame_filename = os.path.join(output_dir, f"frame_{frame_count:04d}.bmp")
                frame.save(frame_filename, "BMP")
                
                frame_count += 1
                
                # Move to next frame
                gif.seek(gif.tell() + 1)
                
            except EOFError:
                # End of GIF
                break
        
        # Console output (normal format)
        print(f"Extracted {frame_count} frames to {output_dir}/")
        print(f"Frame rate: {fps:.2f} fps")
        print(f"Total frames: {frame_count}")
        
        # Write binary data to SDfiles/info.txt
        with open("D:/OscilloProj/SDfiles/info.txt", "wb") as f:
            # Write FPS as 16-bit binary (little endian)
            fps_int = int(fps * 100)  # Convert to integer
            f.write(struct.pack('<H', fps_int))
            
            # Write total frame count as 16-bit binary (little endian)
            f.write(struct.pack('<H', frame_count))
            
            # Write framesize as 16-bit binary (little endian) - placeholder, will be updated by C++ code
            framesize_placeholder = 0  # Will be updated later by the C++ processing
            f.write(struct.pack('<H', framesize_placeholder))
        
        return frame_count
        
    except Exception as e:
        print(f"Error: {e}")
        return 0
    
    finally:
        if 'gif' in locals():
            gif.close()

def main():
    if len(sys.argv) != 2:
        print("Usage: python gif_to_bmp.py <gif_file>")
        sys.exit(1)
    
    gif_file = sys.argv[1]
    
    if not os.path.exists(gif_file):
        print(f"Error: File {gif_file} not found")
        sys.exit(1)
    
    if not gif_file.lower().endswith('.gif'):
        print("Error: Input file must be a GIF")
        sys.exit(1)
    
    gif_to_bmp_frames(gif_file)

if __name__ == "__main__":
    main()
