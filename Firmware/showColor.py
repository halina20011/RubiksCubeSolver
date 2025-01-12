import tkinter as tk
import serial

# Configuration
SERIAL_PORT = "/dev/ttyUSB0"  # Path to the serial device
BAUD_RATE = 115200  # Match the baud rate of your device
WINDOW_WIDTH = 800
WINDOW_HEIGHT = 600

def read_rgb_values(serial_connection):
    """Reads a line from the serial port and parses it into RGB values."""
    try:
        line = serial_connection.readline().decode("utf-8").strip()
        r, g, b = map(int, line.split())
        return (r, g, b)
    except (ValueError, UnicodeDecodeError):
        print(f"Invalid line received: {line}")
        return None

def update_color(canvas, serial_connection):
    """Reads RGB values and updates the canvas color."""
    color = read_rgb_values(serial_connection)
    if color:
        # Convert RGB to hex color
        hex_color = f"#{color[0]:02x}{color[1]:02x}{color[2]:02x}"
        canvas.config(bg=hex_color)
    # Continue waiting for the next line
    canvas.after(1, update_color, canvas, serial_connection)

def main():
    # Open the serial connection
    try:
        serial_connection = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=None)  # Blocking mode
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        return

    # Set up the tkinter window
    root = tk.Tk()
    root.title("Color Display")
    root.geometry(f"{WINDOW_WIDTH}x{WINDOW_HEIGHT}")
    canvas = tk.Canvas(root, width=WINDOW_WIDTH, height=WINDOW_HEIGHT)
    canvas.pack(fill="both", expand=True)

    # Start the update loop
    update_color(canvas, serial_connection)
    root.mainloop()

    # Close the serial connection when done
    serial_connection.close()

if __name__ == "__main__":
    main()
