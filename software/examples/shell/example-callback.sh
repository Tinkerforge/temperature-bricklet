#!/bin/sh
# Connects to localhost:4223 by default, use --host and --port to change this

uid=XYZ # Change XYZ to the UID of your Temperature Bricklet

# Handle incoming temperature callbacks
tinkerforge dispatch temperature-bricklet $uid temperature &

# Set period for temperature callback to 1s (1000ms)
# Note: The temperature callback is only called every second
#       if the temperature has changed since the last call!
tinkerforge call temperature-bricklet $uid set-temperature-callback-period 1000

echo "Press key to exit"; read dummy

kill -- -$$ # Stop callback dispatch in background
