import com.tinkerforge.IPConnection;
import com.tinkerforge.BrickletTemperature;

public class ExampleThreshold {
	private static final String HOST = "localhost";
	private static final int PORT = 4223;

	// Change XYZ to the UID of your Temperature Bricklet
	private static final String UID = "XYZ";

	// Note: To make the example code cleaner we do not handle exceptions. Exceptions
	//       you might normally want to catch are described in the documentation
	public static void main(String args[]) throws Exception {
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletTemperature t = new BrickletTemperature(UID, ipcon); // Create device object

		ipcon.connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Get threshold callbacks with a debounce time of 10 seconds (10000ms)
		t.setDebouncePeriod(10000);

		// Add temperature reached listener
		t.addTemperatureReachedListener(new BrickletTemperature.TemperatureReachedListener() {
			public void temperatureReached(short temperature) {
				System.out.println("Temperature: " + temperature/100.0 + " °C");
				System.out.println("It is too hot, we need air conditioning!");
			}
		});

		// Configure threshold for temperature "greater than 30 °C"
		t.setTemperatureCallbackThreshold('>', (short)(30*100), (short)0);

		System.out.println("Press key to exit"); System.in.read();
		ipcon.disconnect();
	}
}
