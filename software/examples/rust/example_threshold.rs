use std::{error::Error, io, thread};
use tinkerforge::{ipconnection::IpConnection, temperature_bricklet::*};

const HOST: &str = "127.0.0.1";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your Temperature Bricklet

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection
    let temperature_bricklet = TemperatureBricklet::new(UID, &ipcon); // Create device object

    ipcon.connect(HOST, PORT).recv()??; // Connect to brickd
                                        // Don't use device before ipcon is connected

    // Get threshold listeners with a debounce time of 10 seconds (10000ms)
    temperature_bricklet.set_debounce_period(10000);

    //Create listener for temperature reached events.
    let temperature_reached_listener = temperature_bricklet.get_temperature_reached_receiver();
    // Spawn thread to handle received events. This thread ends when the temperature_bricklet
    // is dropped, so there is no need for manual cleanup.
    thread::spawn(move || {
        for event in temperature_reached_listener {
            println!("Temperature: {}{}", event as f32 / 100.0, " °C");
            println!("It is too hot, we need air conditioning!");
        }
    });

    // Configure threshold for temperature "greater than 30 °C"
    temperature_bricklet.set_temperature_callback_threshold('>', 30 * 100, 0);

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}
