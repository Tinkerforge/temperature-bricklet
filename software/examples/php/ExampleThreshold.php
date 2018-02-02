<?php

require_once('Tinkerforge/IPConnection.php');
require_once('Tinkerforge/BrickletTemperature.php');

use Tinkerforge\IPConnection;
use Tinkerforge\BrickletTemperature;

const HOST = 'localhost';
const PORT = 4223;
const UID = 'XYZ'; // Change XYZ to the UID of your Temperature Bricklet

// Callback function for temperature reached callback
function cb_temperatureReached($temperature)
{
    echo "Temperature: " . $temperature/100.0 . " °C\n";
    echo "It is too hot, we need air conditioning!\n";
}

$ipcon = new IPConnection(); // Create IP connection
$t = new BrickletTemperature(UID, $ipcon); // Create device object

$ipcon->connect(HOST, PORT); // Connect to brickd
// Don't use device before ipcon is connected

// Get threshold callbacks with a debounce time of 10 seconds (10000ms)
$t->setDebouncePeriod(10000);

// Register temperature reached callback to function cb_temperatureReached
$t->registerCallback(BrickletTemperature::CALLBACK_TEMPERATURE_REACHED,
                     'cb_temperatureReached');

// Configure threshold for temperature "greater than 30 °C"
$t->setTemperatureCallbackThreshold('>', 30*100, 0);

echo "Press ctrl+c to exit\n";
$ipcon->dispatchCallbacks(-1); // Dispatch callbacks forever

?>
