# import serial
# import time
# from datetime import datetime
# import numpy as np
# from PIL import Image

# def rgb565_to_rgb888(data):
#     """
#     Convert RGB565 data to RGB888 format.
#     Each pixel in RGB565 is 2 bytes (16 bits) and will be converted to 3 bytes (24 bits) in RGB888.
#     """
#     # Ensure we have an even number of bytes
#     if len(data) % 2 != 0:
#         data = data[:-1]
    
#     # Convert bytes to 16-bit integers
#     pixels = np.frombuffer(data, dtype=np.uint16)
    
#     # Extract RGB components
#     r = ((pixels & 0xF800) >> 11).astype(np.uint8)
#     g = ((pixels & 0x07E0) >> 5).astype(np.uint8)
#     b = (pixels & 0x001F).astype(np.uint8)
    
#     # Scale to 8-bit per channel
#     r = (r * 255 // 31).astype(np.uint8)
#     g = (g * 255 // 63).astype(np.uint8)
#     b = (b * 255 // 31).astype(np.uint8)
    
#     return np.stack([r, g, b], axis=1)

# def capture_rgb565_from_serial(port='/dev/tty.wchusbserial210', baudrate=250000, timeout=5, width=320, height=240):
#     """
#     Captures RGB565 image data from serial port, converts it to RGB888, and saves as JPEG.
    
#     Args:
#         port (str): Serial port name
#         baudrate (int): Baud rate of the serial connection
#         timeout (int): Time in seconds to wait for more data before saving
#         width (int): Width of the image in pixels
#         height (int): Height of the image in pixels
#     """
#     # Calculate expected data size (2 bytes per pixel for RGB565)
#     expected_size = width * height * 2
    
#     # Open serial connection
#     ser = serial.Serial(port, baudrate, timeout=1)
#     print(f"Opening {port} at {baudrate} baud...")
    
#     try:
#         # Clear any existing data
#         ser.reset_input_buffer()
        
#         # Initialize variables
#         image_data = bytearray()
#         last_read_time = time.time()
#         expected_length = None
        
#         print("Waiting for image data...")
        
#         # First, get the expected image length
#         while expected_length is None:
#             if ser.in_waiting:
#                 try:
#                     line = ser.readline().decode().strip()
#                     if line:
#                         expected_length = int(line)
#                         print(f"Expected image length: {expected_length} bytes")
#                         if expected_length != expected_size:
#                             print(f"Warning: Expected size ({expected_length}) differs from calculated size ({expected_size})")
#                         break
#                 except (ValueError, UnicodeDecodeError):
#                     continue

#         # Now capture the image data
#         while True:
#             if ser.in_waiting:
#                 chunk = ser.read(ser.in_waiting)
#                 if chunk:
#                     image_data.extend(chunk)
#                     last_read_time = time.time()
#             else:
#                 if time.time() - last_read_time > timeout:
#                     break
            
#             time.sleep(0.01)
        
#         # Verify we have complete data
#         if len(image_data) < expected_size:
#             print(f"Warning: Received incomplete data. Got {len(image_data)} bytes, expected {expected_size}")
#             # Pad with zeros if needed
#             image_data.extend(b'\x00' * (expected_size - len(image_data)))
#         elif len(image_data) > expected_size:
#             print(f"Warning: Received extra data. Truncating to {expected_size} bytes")
#             image_data = image_data[:expected_size]
        
#         # Convert RGB565 to RGB888
#         rgb888_data = rgb565_to_rgb888(image_data)
        
#         # Reshape the array to image dimensions
#         rgb_image = rgb888_data.reshape((height, width, 3))
        
#         # Create PIL Image
#         image = Image.fromarray(rgb_image, 'RGB')
        
#         # Generate filename with timestamp
#         timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
#         filename = f"captured_image_{timestamp}.jpg"
        
#         # Save the image
#         image.save(filename, 'JPEG')
        
#         print(f"Image saved as {filename}")
#         print(f"Image size: {len(image_data)} bytes")
#         if expected_length:
#             print(f"Expected size was: {expected_length} bytes")
        
#     except Exception as e:
#         print(f"Error: {str(e)}")
    
#     finally:
#         # Close the serial connection
#         ser.close()
#         print("Serial connection closed")

# if __name__ == "__main__":
#     # You may need to adjust these parameters to match your setup
#     PORT = '/dev/tty.wchusbserial210'  # Change this to match your serial port
#     BAUDRATE = 250000  # Make sure this matches your Arduino's baud rate
#     WIDTH = 640  # Set your image width
#     HEIGHT = 480  # Set your image height
    
#     capture_rgb565_from_serial(PORT, BAUDRATE, width=WIDTH, height=HEIGHT)
    
    
    
    
    
    
    
    
    
import serial
import time
from datetime import datetime
import numpy as np
from PIL import Image


def capture_jpeg_from_serial(port='/dev/tty.wchusbserial210', baudrate=250000, timeout=5):
    """
    Captures JPEG data from serial port and saves it when there's a pause in transmission.
    
    Args:
        port (str): Serial port name (e.g., 'COM3' on Windows, '/dev/ttyUSB0' on Linux)
        baudrate (int): Baud rate of the serial connection
        timeout (int): Time in seconds to wait for more data before saving
    """
    # Open serial connection
    ser = serial.Serial(port, baudrate, timeout=1)  # Using 1 second timeout for readline
    print(f"Opening {port} at {baudrate} baud...")
    
    try:
        # Clear any existing data
        ser.reset_input_buffer()
        
        # Initialize variables
        image_data = bytearray()
        last_read_time = time.time()
        expected_length = None
        
        print("Waiting for image data...")
        
        # First, get the expected image length
        while expected_length is None:
            if ser.in_waiting:
                try:
                    line = ser.readline().decode().strip()
                    if line:
                        expected_length = int(line)
                        print(f"Expected image length: {expected_length} bytes")
                        break
                except (ValueError, UnicodeDecodeError):
                    continue

        # Now capture the image data
        while True:
            if ser.in_waiting:
                # Read available data
                chunk = ser.read(ser.in_waiting)
                if chunk:
                    image_data.extend(chunk)
                    last_read_time = time.time()
            else:
                # Check if we've had no data for the timeout period
                if time.time() - last_read_time > timeout:
                    break
            
            # Optional: Add a small delay to prevent CPU hogging
            time.sleep(0.01)
        
        # Verify JPEG markers
        if len(image_data) >= 2:
            # Check for JPEG start marker (FFD8)
            if image_data[0:2] != b'\xFF\xD8':
                print("Warning: JPEG start marker not found")
            
            # Check for JPEG end marker (FFD9)
            if image_data[-2:] != b'\xFF\xD9':
                print("Warning: JPEG end marker not found")
        
        # Generate filename with timestamp
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"captured_image_{timestamp}.jpg"
        
        # Save the image data
        with open(filename, 'wb') as f:
            f.write(image_data)
        
        # Convert image to numpy array
        image_array = np.array(Image.open(filename))
        
        # Calculate the mean values for each channel
        mean_r = np.mean(image_array[:, :, 0])
        mean_g = np.mean(image_array[:, :, 1])
        mean_b = np.mean(image_array[:, :, 2])
        
        # Calculate the overall mean of all channels
        target_mean = np.mean([mean_r, mean_g, mean_b])
        
        # Calculate adjustment factors
        r_factor = target_mean / mean_r if mean_r > 0 else 1
        g_factor = target_mean / mean_g if mean_g > 0 else 1
        b_factor = target_mean / mean_b if mean_b > 0 else 1
        
        # Apply adjustments with clipping to prevent overflow
        image_array[:, :, 0] = np.clip(image_array[:, :, 0] * r_factor, 0, 255)
        image_array[:, :, 1] = np.clip(image_array[:, :, 1] * g_factor, 0, 255)
        image_array[:, :, 2] = np.clip(image_array[:, :, 2] * b_factor, 0, 255)
        
        # Convert back to image and save
        new_image = Image.fromarray(image_array.astype('uint8'))
        new_image.save(filename)

        print(f"Image saved as {filename}")
        print(f"Image size: {len(image_data)} bytes")
        if expected_length:
            print(f"Expected size was: {expected_length} bytes")
            if len(image_data) != expected_length:
                print("Warning: Captured size differs from expected size!")
        
    except Exception as e:
        print(f"Error: {str(e)}")
    
    finally:
        # Close the serial connection
        ser.close()
        print("Serial connection closed")



if __name__ == "__main__":
    # You may need to adjust these parameters to match your setup
    PORT = '/dev/tty.wchusbserial110'  # Change this to match your serial port
    BAUDRATE = 250000  # Make sure this matches your Arduino's baud rate
    
    capture_jpeg_from_serial(PORT, BAUDRATE)





