use std::{error::Error, io, thread};
use tinkerforge::{ip_connection::IpConnection, temperature_bricklet::*};

const HOST: &str = "localhost";
const PORT: u16 = 4223;
const UID: &str = "XYZ"; // Change XYZ to the UID of your Temperature Bricklet.

fn main() -> Result<(), Box<dyn Error>> {
    let ipcon = IpConnection::new(); // Create IP connection.
    let t = TemperatureBricklet::new(UID, &ipcon); // Create device object.

    ipcon.connect((HOST, PORT)).recv()??; // Connect to brickd.
                                          // Don't use device before ipcon is connected.

    // Get threshold receivers with a debounce time of 10 seconds (10000ms).
    t.set_debounce_period(10000);

    let temperature_reached_receiver = t.get_temperature_reached_callback_receiver();

    // Spawn thread to handle received callback messages.
    // This thread ends when the `t` object
    // is dropped, so there is no need for manual cleanup.
    thread::spawn(move || {
        for temperature_reached in temperature_reached_receiver {
            println!("Temperature: {} °C", temperature_reached as f32 / 100.0);
            println!("It is too hot, we need air conditioning!");
        }
    });

    // Configure threshold for temperature "greater than 30 °C".
    t.set_temperature_callback_threshold('>', 30 * 100, 0);

    println!("Press enter to exit.");
    let mut _input = String::new();
    io::stdin().read_line(&mut _input)?;
    ipcon.disconnect();
    Ok(())
}
