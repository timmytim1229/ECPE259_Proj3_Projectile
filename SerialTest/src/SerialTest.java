import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintWriter;

import gnu.io.CommPortIdentifier; 
import gnu.io.SerialPort;
import gnu.io.SerialPortEvent; 
import gnu.io.SerialPortEventListener; 
import java.util.Enumeration;

import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

import java.util.*;
import java.sql.*;

import java.awt.*;
import javax.swing.JPanel;
import javax.swing.SwingUtilities;

//import java.util.Random;
import javax.swing.JFrame;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.chart.renderer.xy.XYItemRenderer;
import org.jfree.chart.renderer.xy.XYLineAndShapeRenderer;
import org.jfree.data.time.Millisecond;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.TimeSeriesCollection;

//import java.io.FileNotFoundException;
//import java.io.FileReader;
//import java.io.IOException;
//import java.util.Iterator;

//TODO: Make sure plots stay open after data reading is complete.
//TODO: Save data to object to analyze later? Or is text file good enough?

public class SerialTest implements SerialPortEventListener {
	SerialPort serialPort;
    
	/** The port we're normally going to use. */
	private static final String PORT_NAMES[] = { 
			"/dev/tty.usbserial-DA017QQD", "COM1", "COM2", "COM3"};
	/**
	* A BufferedReader which will be fed by a InputStreamReader 
	* converting the bytes into characters 
	* making the displayed results codepage independent
	*/
	private BufferedReader input;
	/** The output stream to the port */
	private OutputStream output;
	/** Milliseconds to block while waiting for port open */
	private static final int TIME_OUT = 2000;
	/** Default bits per second for COM port. */
	private static final int DATA_RATE = 9600;
	
	// The name of the file to open.
    static String fileName = "demosat_data_.txt";
    
    private InputStream input_chars;
        
    // Sensor identifier constants
    private static final char ACCEL_ID = 'A';
    private static final char BAROMETER_ID = 'B';
    private static final char GYRO_ID = 'G';
    private static final char MAGNETOMETER_ID = 'M';

    // Sensor vectors for input stream
    public static Vector accel_x_vector = new Vector(3,1);
    public static Vector accel_y_vector = new Vector(3,1);
    public static Vector accel_z_vector = new Vector(3,1);
    public static Vector bar_vector = new Vector(3,1);
    public static Vector gyro_x_vector = new Vector(3,1);	// init size 3, add 1 value ahead of currently filled value
    public static Vector gyro_y_vector = new Vector(3,1);
    public static Vector gyro_z_vector = new Vector(3,1);
    public static Vector mag_x_vector = new Vector(3,1);
    public static Vector mag_y_vector = new Vector(3,1);
    public static Vector mag_z_vector = new Vector(3,1);
    
    // Sensor time series (ts) initialization for real time plots
    static TimeSeries accel_x_ts = new TimeSeries("accel_x_data", Millisecond.class);
    static TimeSeries accel_y_ts = new TimeSeries("accel_y_data", Millisecond.class);
    static TimeSeries accel_z_ts = new TimeSeries("accel_z_data", Millisecond.class);
    static TimeSeries bar_ts = new TimeSeries("accel_x_data", Millisecond.class);
    static TimeSeries gyro_x_ts = new TimeSeries("gyro_x_data", Millisecond.class);
    static TimeSeries gyro_y_ts = new TimeSeries("gyro_y_data", Millisecond.class);
    static TimeSeries gyro_z_ts = new TimeSeries("gyro_z_data", Millisecond.class);
    static TimeSeries mag_x_ts = new TimeSeries("barometer_data", Millisecond.class);
    static TimeSeries mag_y_ts = new TimeSeries("barometer_data", Millisecond.class);
    static TimeSeries mag_z_ts = new TimeSeries("barometer_data", Millisecond.class);
    
    // Parser Object
    //JSONParser parser = new JSONParser();
    
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
				char [] input_array = new char[20]; // TODO: needs length of packet
				char check_char;
				int len = 0;
				int i = 0;
				int value;
				
				check_char = (char)input_chars.read();
				
				if(check_char == 'S')	// start packet read					
				{
					len = input_chars.read(); 		// next byte in packet is length of packet
					
					// See what the next char is
					check_char = (char)input_chars.read();
					do
					{
						System.out.println("Input Char = " + check_char);
						// Acceleration data
						if(check_char == ACCEL_ID)
						{						
							value = Integer.parseInt(input.readLine());			// convert input string to int
							System.out.println("Accel Value_x = " + value);
							accel_x_vector.addElement(new Double(value));		// Save to vector
							accel_x_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series					
							
							value = Integer.parseInt(input.readLine());			// convert input string to int
							System.out.println("Accel Value_y = " + value);
							accel_y_vector.addElement(new Double(value));		// Save to vector
							accel_y_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
								
							value = Integer.parseInt(input.readLine());			// convert input string to int
							System.out.println("Accel Value_z = " + value);
							accel_z_vector.addElement(new Double(value));		// Save to vector
							accel_z_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
						}
						else if (check_char == BAROMETER_ID)
						{
							value = Integer.parseInt(input.readLine());			// convert input string to int
							System.out.println("Barometer Value = " + value);
							bar_vector.addElement(new Double(value));			// Save to vector
							bar_ts.addOrUpdate(new Millisecond(), value);		// Save to time	series
						}
						else if (check_char == GYRO_ID)
						{
							value = Integer.parseInt(input.readLine());			// convert input string to int
							System.out.println("Gyro Value_x = " + value);
							gyro_x_vector.addElement(new Double(value));		// Save to vector
							gyro_x_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							
							value = Integer.parseInt(input.readLine());			// convert input string to int
							System.out.println("Gyro Value_y = " + value);
							gyro_y_vector.addElement(new Double(value));		// Save to vector
							gyro_y_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							
							value = Integer.parseInt(input.readLine());			// convert input string to int
							System.out.println("Gyro Value_z = " + value);
							gyro_z_vector.addElement(new Double(value));		// Save to vector
							gyro_z_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
						}
						else if (check_char == MAGNETOMETER_ID)
						{
							value = Integer.parseInt(input.readLine());			// convert input string to int
							System.out.println("Magnetometer Value_x = " + value);
							mag_x_vector.addElement(new Double(value));			// Save to vector
							mag_x_ts.addOrUpdate(new Millisecond(), value);		// Save to time	series
							
							value = Integer.parseInt(input.readLine());			// convert input string to int
							System.out.println("Magnetometer Value_y = " + value);
							mag_y_vector.addElement(new Double(value));			// Save to vector
							mag_y_ts.addOrUpdate(new Millisecond(), value);		// Save to time	series
							
							value = Integer.parseInt(input.readLine());			// convert input string to int
							System.out.println("Magnetometer Value_y = " + value);
							mag_z_vector.addElement(new Double(value));			// Save to vector
							mag_z_ts.addOrUpdate(new Millisecond(), value);		// Save to time	series			
						}
						
						check_char = (char)input_chars.read();
					} while(check_char != 'X');	
				}
				
				
				/*
				// first thoughts on input
				// See what char is next
				
				//check_char = (char)input_chars.read();
				//System.out.println("Input Char = " + check_char);
				
				// Acceleration data
				if(check_char == 'A')
				{	
					value = Integer.parseInt(input.readLine());			// convert input string to int
					System.out.println("Value_x = " + value);
					accel_x_vector.addElement(new Double(value));		// Save to vector
					accel_x_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series					
					
					value = Integer.parseInt(input.readLine());			// convert input string to int
					System.out.println("Value_y = " + value);
					accel_y_vector.addElement(new Double(value));		// Save to vector
					accel_y_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
						
					value = Integer.parseInt(input.readLine());			// convert input string to int
					System.out.println("Value_z = " + value);
					accel_z_vector.addElement(new Double(value));		// Save to vector
					accel_z_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
										
					
					
					
					// Another way to capture data as chars only
					// Capture acceleration_x data chars
					while((char)input_chars.read() != '\n')
					{
						input_array[i] = (char)input_chars.read();
						i++;
					}
					// Save to an int //TODO: Convert to g's later
					int value = Integer.parseInt(new String(input_array));						
					// Save to vector
					accel_x_vector.addElement(new Double(value));						
					// Save to timeseries //TODO: Make plotting cleaner
					accel_x_ts.addOrUpdate(new Millisecond(), value);
					i = 0;
					
					// Capture acceleration_y data chars
					while((char)input_chars.read() != '\n')
					{
						input_array[i] = (char)input_chars.read();
						i++;
					}
					// Save to an int //TODO: Convert to g's later
					value = Integer.parseInt(new String(input_array));						
					// Save to vector
					accel_y_vector.addElement(new Double(value));						
					// Save to timeseries //TODO: Make plotting cleaner
					accel_y_ts.addOrUpdate(new Millisecond(), value);
					i = 0;
					
					// Capture acceleration_z data chars
					while((char)input_chars.read() != '\n')
					{
						input_array[i] = (char)input_chars.read();
						i++;
					}
					// Save to an int //TODO: Convert to g's later
					value = Integer.parseInt(new String(input_array));						
					// Save to vector
					accel_z_vector.addElement(new Double(value));						
					// Save to timeseries //TODO: Make plotting cleaner
					accel_z_ts.addOrUpdate(new Millisecond(), value);
					i = 0;
					
				}		
				 */
				
				
													
				
				
				
				
				
				
				/*
				// Fix JSON formatting from Demosat
				
				inputLine = inputLine.replace("~", "");
           	 	inputLine = inputLine.replace("|", "");
           	 	if (inputLine.isEmpty()) {
           		    inputLine = null; 
           		}
           	 	
                //Parse each JSON line and plot the corresponding value
            	 try {
	            	 Object obj = parser.parse(inputLine);
	
	            	 JSONObject jsonObject = (JSONObject) obj;
	
	            	 String name = (String) jsonObject.get("sensorName");
	            	 //System.out.println("SensorName = " + name);	            	 	            	 	            
	            				
	            	 String unit = (String) jsonObject.get("unit");
	            	 //System.out.println("Unit = " + unit);
	            	 
	            	 Double value = (Double) jsonObject.get("value");
	            	 //System.out.println("Value = " + value);
	            				
	            	 long cs = (long) jsonObject.get("cs");
	            	 //System.out.println("CS = " + cs);

	            	 if (name.equals("temp"))
	            	 {
	            		 System.out.println(inputLine);
	            		 System.out.println("SensorName = " + name);
	            		 System.out.println("Unit = " + unit);
	            		 System.out.println("Value = " + value);
	            		 System.out.println("CS = " + cs);
	            		 
	            		 // Add value to vector
	            		 tempVector.addElement(new Double(value));
	            		 
	            		 // Add value to dataset
	            		 ts.addOrUpdate(new Millisecond(), value);
	            		 
	            	 	 //System.out.println("Vector Element: " + (Double)tempVector.lastElement() + "\n");
	            		// enumerate the elements in the vector.
	            		 
	            		 Enumeration tvEnum = tempVector.elements(); 
	            		 while(tvEnum.hasMoreElements())
	            		 {
	            			 System.out.print(tvEnum.nextElement() + " ");
	            			 System.out.println(); 
	            		 }
	            		 
	            	 }
	            	 else if (name.equals("luminosity"))
	            	 {
	            		 System.out.println(inputLine);
	            		 System.out.println("SensorName = " + name);
	            		 System.out.println("Unit = " + unit);
	            		 System.out.println("Value = " + value);
	            		 System.out.println("CS = " + cs);
	            		 
	            		 // Add value to vector
	            		 //luminosityVector.addElement(new Double(value));
	            		 
	            		 // Add value to dataset
	                     //ts2.addOrUpdate(new Millisecond(), (Double)luminosityVector.lastElement());
	            		 ts2.addOrUpdate(new Millisecond(), value);
	            		 
	            		 //System.out.println("Vector Element: " + (Double)luminosityVector.lastElement() + "\n");
	            		 
	            		 Enumeration lvEnum = luminosityVector.elements(); 
	            		 while(lvEnum.hasMoreElements())
	            		 {
	            			 System.out.print(lvEnum.nextElement() + " ");
	            			 System.out.println(); 
	            		 }
	            		 
	            	 }	                	      	 	            		 	            	 
            	} 
            	catch (ParseException e) {
            			//e.printStackTrace();
            			System.out.println("Incomplete data. Setting current line to null.\n");
            			inputLine = null;           			
            		}
            	 */
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
				try {Thread.sleep(100000000);} catch (InterruptedException ie) {}
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
        
        /*
        TimeSeriesCollection dataset2 = new TimeSeriesCollection(ts2);
        JFreeChart chart2 = ChartFactory.createTimeSeriesChart(
            "Luminosity",
            "Time",
            "Value (Lux)",
            dataset2,
            true,
            true,
            false
        );
       final XYPlot plot2 = chart2.getXYPlot();
       ValueAxis axis2 = plot2.getDomainAxis();
       axis2.setAutoRange(true);
       axis2.setFixedAutoRange(60000.0);
       XYItemRenderer renderer = plot2.getRenderer();
       renderer.setSeriesPaint(0, Color.blue);
       */
       
       
       
       
       JFrame frame = new JFrame("Sensor Plots"); // frame is the window that holds the panels (these panels contain real time plots)
       frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
       
       
       // Acceleration X
       ChartPanel accel_x_label = new ChartPanel(accel_x_chart);
       frame.getContentPane().add(accel_x_label, BorderLayout.WEST);
       
       // Acceleration Y
       ChartPanel accel_y_label = new ChartPanel(accel_x_chart);
       frame.getContentPane().add(accel_y_label, BorderLayout.CENTER);
       
       // Acceleration Z
       ChartPanel accel_z_label = new ChartPanel(accel_x_chart);
       frame.getContentPane().add(accel_z_label, BorderLayout.EAST);

       
       /*
       ChartPanel label2 = new ChartPanel(chart2);
       frame.getContentPane().add(label2, BorderLayout.EAST);
       */
        
       frame.pack();
       frame.setVisible(true);          		
	}
}
