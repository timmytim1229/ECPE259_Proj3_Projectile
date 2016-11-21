import java.awt.BorderLayout;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Vector;

//import java.util.Random;
import javax.swing.JFrame;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.data.time.Millisecond;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;

import gnu.io.CommPortIdentifier;
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent;
import gnu.io.SerialPortEventListener;

//import java.io.FileNotFoundException;
//import java.io.FileReader;
//import java.io.IOException;
//import java.util.Iterator;

//TODO: Make sure plots stay open after data reading is complete.
//TODO: Save data to object to analyze later? Or is text file good enough?

public class SerialTest implements SerialPortEventListener {
	SerialPort serialPort;
    
	// The ports normally used
	private static final String PORT_NAMES[] = { 
			"/dev/tty.usbserial-DA017QQD", 
			"/dev/tty.usbmodem0E213EC1", 
			"COM1", "COM2", "COM3", "COM4"};
	/**
	* A BufferedReader which will be fed by a InputStreamReader 
	* converting the bytes into characters 
	* making the displayed results codepage independent
	*/
	private BufferedReader input;
	private OutputStream output;
	private static final int TIME_OUT = 2000;		// Milliseconds to block while waiting for port open
	private static final int DATA_RATE = 9600;		// Default bits per second for serial port

	
	// The name of the file to open.
    static String fileName = "demosat_data_.txt";
    
    private BufferedReader input_chars;
        
    // Sensor identifier constants
//    private static final char ACCEL_ID = 'A';
//    private static final char BAROMETER_ID = 'B';
//    private static final char GYRO_ID = 'G';
//    private static final char MAGNETOMETER_ID = 'M';
//    private static final char TEMPERATURE_ID = 'T';

    // Sensor vectors for input stream
    // init size 3, add 1 value ahead of currently filled value
    public static Vector<Double> accel_x_vector = new Vector<Double>(3,1);
    public static Vector<Double> accel_y_vector = new Vector<Double>(3,1);
    public static Vector<Double> accel_z_vector = new Vector<Double>(3,1);
    public static Vector<Double> bar_vector = new Vector<Double>(3,1);
    public static Vector<Double> gyro_x_vector = new Vector<Double>(3,1);
    public static Vector<Double> gyro_y_vector = new Vector<Double>(3,1);
    public static Vector<Double> gyro_z_vector = new Vector<Double>(3,1);
    public static Vector<Double> mag_x_vector = new Vector<Double>(3,1);
    public static Vector<Double> mag_y_vector = new Vector<Double>(3,1);
    public static Vector<Double> mag_z_vector = new Vector<Double>(3,1);
    public static Vector<Double> temperature_vector = new Vector<Double>(3,1);
    
    // Sensor time series (ts) initialization for real time plots
    //Millisecond.class goes into ts parameter
    static TimeSeries accel_x_ts = new TimeSeries("accel_x_data");
    static TimeSeries accel_y_ts = new TimeSeries("accel_y_data");
    static TimeSeries accel_z_ts = new TimeSeries("accel_z_data");
    static TimeSeries bar_ts = new TimeSeries("accel_x_data");
    static TimeSeries gyro_x_ts = new TimeSeries("gyro_x_data");
    static TimeSeries gyro_y_ts = new TimeSeries("gyro_y_data");
    static TimeSeries gyro_z_ts = new TimeSeries("gyro_z_data");
    static TimeSeries mag_x_ts = new TimeSeries("barometer_data");
    static TimeSeries mag_y_ts = new TimeSeries("barometer_data");
    static TimeSeries mag_z_ts = new TimeSeries("barometer_data");
    static TimeSeries temperature_ts = new TimeSeries("temperature_data");
    
    /**
     * initialize() function
     * This function finds an open serial port, opens the port with its parameters,
     * opens the input and output streams for the port, and adds port listeners.
     */
	public void initialize() {
		//System.out.println("Initalize");
        	// the next line is for Raspberry Pi and 
            // gets us into the while loop and was suggested here was suggested http://www.raspberrypi.org/phpBB3/viewtopic.php?f=81&t=32186
            //System.setProperty("gnu.io.rxtx.SerialPorts", "/dev/ttyACM0");

		CommPortIdentifier portId = null;
		Enumeration portEnum = CommPortIdentifier.getPortIdentifiers();

		//First, Find an instance of serial port as set in PORT_NAMES.
		while (portEnum.hasMoreElements()) {
			CommPortIdentifier currPortId = (CommPortIdentifier) portEnum.nextElement();
			for (String portName : PORT_NAMES) {
				if (currPortId.getName().equals(portName)) {
					portId = currPortId;
					break;
				}
			}
		}
		if (portId == null) {
			System.out.println("Could not find serial port.");
			return;
		}

		try {
			// open serial port, and use class name for the appName.
			serialPort = (SerialPort) portId.open(this.getClass().getName(), TIME_OUT);

			// set port parameters
			serialPort.setSerialPortParams(DATA_RATE,
					SerialPort.DATABITS_8,
					SerialPort.STOPBITS_1,
					SerialPort.PARITY_NONE);

			// open the streams
			input = new BufferedReader(new InputStreamReader(serialPort.getInputStream()));
			input_chars = new BufferedReader(new InputStreamReader(serialPort.getInputStream()));
			output = serialPort.getOutputStream();

			// add event listeners
			serialPort.addEventListener(this);
			serialPort.notifyOnDataAvailable(true);
		} catch (Exception e) {
			System.err.println(e.toString());
		}
        
	}

	
	/**
	 * This should be called when you stop using the port.
	 * This will prevent port locking on platforms like Linux.
	 */
	public synchronized void close() {
		if (serialPort != null) {
			serialPort.removeEventListener();
			serialPort.close();
		}
	}
	
	/**
	 * Handle an event on the serial port. Read the data, fix to valid JSON, parse each JSON line,
	 * save each value and unit in a dynamic vector, save each full JSON line with date
	 * to a file, and print each parsed JSON line.
	 */
	public synchronized void serialEvent(SerialPortEvent oEvent) {
		if (oEvent.getEventType() == SerialPortEvent.DATA_AVAILABLE) {		
			try {				
				//String input_Line = "";
				//char [] input_array = new char[20]; // TODO: needs length of packet
				char check_char;
				//int len = 0;
				int value, value_msb, value_lsb;
				double p_value; 					// precise value

				check_char = (char)input_chars.read();				
				System.out.println("1st Input Char = " + check_char);
				while(check_char != 'S')
				{
					//wait for S
					check_char = (char)input_chars.read();
				}
				
				// start packet read
				if(check_char == 'S') {		
					
					// See what the next char is
					check_char = (char)input_chars.read();
					do{	
						
						// read acceleration data
						for(int i = 0; i < 3; i++) {
							//construct value from 2 chars
							value_msb = input_chars.read();
							value_lsb = input_chars.read();
							value = value_msb << 8 | value_lsb;					
							p_value = (double)value / 8192.0;
							
							if(i == 0) {
								System.out.println("Accel Value_x = " + p_value);
								accel_x_vector.addElement(new Double(p_value));		// Save to vector
								accel_x_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series	
							} else if(i == 1) {
								System.out.println("Accel Value_y = " + p_value);
								accel_y_vector.addElement(new Double(p_value));		// Save to vector
								accel_y_ts.addOrUpdate(new Millisecond(), p_value);	// Save to time	series
							} else {
								System.out.println("Accel Value_z = " + p_value);
								accel_z_vector.addElement(new Double(p_value));		// Save to vector
								accel_z_ts.addOrUpdate(new Millisecond(), p_value);	// Save to time	series
							}
						}
						
						// See what the next char is
						check_char = (char)input_chars.read();
						System.out.println(check_char);
						
						// read temperature data
						value_msb = input_chars.read();
						value_lsb = input_chars.read();
						value = value_msb << 8 | value_lsb;		
						p_value = (double)value / 340.0 + 36.53;
						
						System.out.println("Temperature Value = " + value);
						temperature_vector.addElement(new Double(value));			// Save to vector
						temperature_ts.addOrUpdate(new Millisecond(), value);		// Save to time	series
						
						// See what the next char is
						check_char = (char)input_chars.read();
						System.out.println(check_char);
						
						for(int i = 0; i < 3; i++) {
							//construct value from 2 chars
							value_msb = input_chars.read();
							value_lsb = input_chars.read();
							value = value_msb << 8 | value_lsb;					
							p_value = (double)value / 939.650784;
							
							if(i == 0) {
								System.out.println("Gyro Value_x = " + value);
								gyro_x_vector.addElement(new Double(value));		// Save to vector
								gyro_x_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							} else if(i == 1) {
								System.out.println("Gyro Value_y = " + value);
								gyro_y_vector.addElement(new Double(value));		// Save to vector
								gyro_y_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							} else {
								System.out.println("Gyro Value_z = " + value);
								gyro_z_vector.addElement(new Double(value));		// Save to vector
								gyro_z_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							}
						}
							
//						else if (check_char == MAGNETOMETER_ID)
//						{
//							value = Integer.parseInt(input.readLine());			// convert input string to int
//							System.out.println("Magnetometer Value_x = " + value);
//							mag_x_vector.addElement(new Double(value));			// Save to vector
//							mag_x_ts.addOrUpdate(new Millisecond(), value);		// Save to time	series
//							
//							value = Integer.parseInt(input.readLine());			// convert input string to int
//							System.out.println("Magnetometer Value_y = " + value);
//							mag_y_vector.addElement(new Double(value));			// Save to vector
//							mag_y_ts.addOrUpdate(new Millisecond(), value);		// Save to time	series
//							
//							value = Integer.parseInt(input.readLine());			// convert input string to int
//							System.out.println("Magnetometer Value_y = " + value);
//							mag_z_vector.addElement(new Double(value));			// Save to vector
//							mag_z_ts.addOrUpdate(new Millisecond(), value);		// Save to time	series			
//						}
						
						check_char = (char)input_chars.read();
						if(check_char != '\n')
							System.out.println("ERROR, LAST CHARACTER IS NOT \n!");
					} while(check_char != '\n');	
				}
				
				
				
				/*
            	 try(FileWriter fw = new FileWriter(fileName, true);
              		    BufferedWriter bw = new BufferedWriter(fw);
              		    PrintWriter out = new PrintWriter(bw))
              		{
            		 	// Write JSON line to file with timestamp.
     	           		if (inputLine != null)
     	           		{
     	           			long retryDate = System.currentTimeMillis();
		     	            Timestamp original = new Timestamp(retryDate);
		     	            Calendar cal = Calendar.getInstance();
		     	            cal.setTimeInMillis(original.getTime());
		     	            
		     	            String formatStr = "%1$-80s %2$s"; // 80 char left padding
		     	            out.println(String.format(formatStr, inputLine, original));
     	           		}
              		} catch (IOException e) {
              		    //TODO
              		}                            
           	 	*/
			} catch (Exception e) {
				//System.err.println(e.toString());
			}
		}		
	}

	/**
	 * Main.
	 * 
	 */
	public static void main(String[] args) throws Exception {

		// for serial streaming
		SerialTest main = new SerialTest();
		main.initialize();
		Thread t=new Thread() {
			public void run() {
				//the following line will keep this app alive for 100000 seconds,
				//waiting for events to occur and responding to them (printing incoming messages to console).
				try {
					System.out.println("In run");
					Thread.sleep(100000000);
					
				} catch (InterruptedException ie) {}
			}
		};
		t.start();
		System.out.println("Started");
		
		/*
		// Timestamp to file
		try(FileWriter fw = new FileWriter(fileName, true);
      		    BufferedWriter bw = new BufferedWriter(fw);
      		    PrintWriter out = new PrintWriter(bw))
      			{
		           	long retryDate = System.currentTimeMillis();
	     	        Timestamp original = new Timestamp(retryDate);
	     	        Calendar cal = Calendar.getInstance();
	     	        cal.setTimeInMillis(original.getTime());
	     	            
		            out.println("\nStart Time: " + original);
      		} catch (IOException e) {
      		    //TODO
      		} 
      		*/                           
						
	    // For real time plotting
		// Acceleration X
        TimeSeriesCollection accel_x_dataset = new TimeSeriesCollection(accel_x_ts);
        JFreeChart accel_x_chart = ChartFactory.createTimeSeriesChart(
            "X Acceleration",
            "Time",
            "Value (g)",
            accel_x_dataset,
            true,
            true,
            false
        );
        final XYPlot accel_x_plot = accel_x_chart.getXYPlot();
        ValueAxis accel_x_axis = accel_x_plot.getDomainAxis();
        accel_x_axis.setAutoRange(true);
        accel_x_axis.setFixedAutoRange(60000.0); 
		
        // Acceleration Y
        TimeSeriesCollection accel_y_dataset = new TimeSeriesCollection(accel_y_ts);
        JFreeChart accel_y_chart = ChartFactory.createTimeSeriesChart(
            "Y Acceleration",
            "Time",
            "Value (g)",
            accel_y_dataset,
            true,
            true,
            false
        );
        final XYPlot accel_y_plot = accel_y_chart.getXYPlot();
        ValueAxis accel_y_axis = accel_y_plot.getDomainAxis();
        accel_y_axis.setAutoRange(true);
        accel_y_axis.setFixedAutoRange(60000.0);       
        
        // Acceleration Z
        TimeSeriesCollection accel_z_dataset = new TimeSeriesCollection(accel_z_ts);
        JFreeChart accel_z_chart = ChartFactory.createTimeSeriesChart(
            "Z Acceleration",
            "Time",
            "Value (g)",
            accel_z_dataset,
            true,
            true,
            false
        );
        final XYPlot accel_z_plot = accel_z_chart.getXYPlot();
        ValueAxis accel_z_axis = accel_z_plot.getDomainAxis();
        accel_z_axis.setAutoRange(true);
        accel_z_axis.setFixedAutoRange(60000.0);
        
       
       
       
       
       JFrame frame = new JFrame("Sensor Plots"); // frame is the window that holds the panels (these panels contain real time plots)
       frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
       
       
       // Acceleration X
       ChartPanel accel_x_label = new ChartPanel(accel_x_chart);
       frame.getContentPane().add(accel_x_label, BorderLayout.WEST);
       
       // Acceleration Y
       ChartPanel accel_y_label = new ChartPanel(accel_y_chart);
       frame.getContentPane().add(accel_y_label, BorderLayout.CENTER);
       
       // Acceleration Z
       ChartPanel accel_z_label = new ChartPanel(accel_z_chart);
       frame.getContentPane().add(accel_z_label, BorderLayout.EAST);

        
       frame.pack();
       frame.setVisible(true);          		
	}
}
