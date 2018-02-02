#!/bin/sh
# Connects to localhost:4223 by default, use --host and --port to change this

uid=XYZ # Change XYZ to the UID of your Temperature Bricklet

# Get threshold callbacks with a debounce time of 10 seconds (10000ms)
tinkerforge call temperature-bricklet $uid set-debounce-period 10000

# Handle incoming temperature reached callbacks
tinkerforge dispatch temperature-bricklet $uid temperature-reached\
 --execute "echo Temperature: {temperature}/100 °C. It is too hot, we need air conditioning!" &

# Configure threshold for temperature "greater than 30 °C"
tinkerforge call temperature-bricklet $uid set-temperature-callback-threshold threshold-option-greater 3000 0

echo "Press key to exit"; read dummy

kill -- -$$ # Stop callback dispatch in background
