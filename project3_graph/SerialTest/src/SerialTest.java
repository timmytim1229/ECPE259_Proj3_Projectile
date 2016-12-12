import static java.lang.Thread.currentThread;
import java.awt.BorderLayout;
import java.io.ByteArrayInputStream;
import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.PrintWriter;
import java.util.Enumeration;
import java.util.Vector;
import java.util.Iterator;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Timer;
import java.lang.Runtime;

import java.lang.*;

// Import for pause
import java.util.concurrent.TimeUnit;

import java.text.Normalizer;

//import java.util.Random;
import javax.swing.JFrame;

import org.jfree.chart.ChartFactory;
import org.jfree.chart.ChartPanel;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.ValueAxis;
import org.jfree.chart.plot.XYPlot;
import org.jfree.data.time.Millisecond;
import org.jfree.data.time.TimeSeries;
import org.jfree.data.time.RegularTimePeriod;
import org.jfree.data.time.TimeSeriesCollection;
import org.jfree.chart.axis.NumberAxis;

import gnu.io.CommPort;
import gnu.io.CommPortIdentifier;
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
			"/dev/tty.usbmodem0E2147C1", 
			"COM1", "COM2", "COM3", "COM4", "COM13", "COM15"};
	/**
	* A BufferedReader which will be fed by a InputStreamReader 
	* converting the bytes into characters 
	* making the displayed results codepage independent
	*/
	private BufferedReader input;
	private BufferedInputStream byte_input;
	private OutputStream output;
	private static final int TIME_OUT = 5000;		// Milliseconds to block while waiting for port open
	private static final int DATA_RATE = 57600;		// Default bits per second for serial port

	// for stopping threads
	// private volatile static boolean exit = false;
	
	// The name of the file to open.
    static String FILENAME = "sensor_data.txt";
    
    private InputStream input_chars;
    byte[] byte_buf = new byte[10];   
    byte[] buff;   
	//InputStream is = new ByteArrayInputStream(buff);
         
    // Sensor identifier constants
    private static final char ACCEL_ID = 'A';
    private static final char BAROMETER_ID = 'B';
    private static final char GYRO_ID = 'G';
    private static final char MAGNETOMETER_ID = 'M';
    private static final char START_ID = 'S';
    private static final char TEMPERATURE_ID = 'T';
    private static final char NEWLINE = '\n';


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
    public static Vector<Double> quat_w_vector = new Vector<Double>(3,1);
    public static Vector<Double> quat_x_vector = new Vector<Double>(3,1);
    public static Vector<Double> quat_y_vector = new Vector<Double>(3,1);
    public static Vector<Double> quat_z_vector = new Vector<Double>(3,1);

    
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
	static TimeSeries quat_w_ts = new TimeSeries("quat_w_data");
	static TimeSeries quat_x_ts = new TimeSeries("quat_x_data");
	static TimeSeries quat_y_ts = new TimeSeries("quat_y_data");
	static TimeSeries quat_z_ts = new TimeSeries("quat_z_data");
    
    int c = 0;
    /**
     * initialize() function
     * This function finds an open serial port, opens the port with its parameters,
     * opens the input and output streams for the port, and adds port listeners.
     */
	public void initialize() {

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
			input_chars = serialPort.getInputStream();	 		
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
			System.out.println("Closed.");
		}
	}
	
	public static class UnicodeFormatter  {
		 
		   static public String byteToHex(byte b) {
		      // Returns hex String representation of byte b
		      char hexDigit[] = {
		         '0', '1', '2', '3', '4', '5', '6', '7',
		         '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
		      };
		      char[] array = { hexDigit[(b >> 4) & 0x0f], hexDigit[b & 0x0f] };
		      return new String(array);
		   }
		 
		   public static String charToHex(char c) {
		      // Returns hex String representation of char c
		      byte hi = (byte) (c >>> 8);
		      byte lo = (byte) (c & 0xff);
		      return byteToHex(hi) + byteToHex(lo);
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
				char check_char, packetStartChar, validateEndChar;
				//int len = 0;
				int value_msb, value_lsb;
				short combine_value;
				double value;
				double p_value; 					// precise value
				RegularTimePeriod sample_rate, sample_rate2;
				long ms1 = 0, ms2, ms_sum = 0, ms_total = 0, ms_final = 0;

				packetStartChar = (char)input_chars.read();				
				//System.out.println("1st Input Char = " + packetStartChar);
				while(packetStartChar != 'S')
				{
					//wait for S
					packetStartChar = (char)input_chars.read();
					//System.out.println(packetStartChar);
				}
				
				// start packet read
				if(packetStartChar == 'S') {		
					
					// See what the next char is
					check_char = (char)input_chars.read();
					//System.out.println(check_char);
					do{	
						//input_chars.read();
						//input_chars.read();
						// read acceleration data
						for(int i = 0; i < 3; i++) {
							//construct value from 2 chars
							value_msb = input_chars.read();
							value_lsb = input_chars.read();
							value = ((short)((value_msb) << 8) | (value_lsb & 0xff)) / 4096.0;					
							if(i == 0) {
								System.out.println("Accel Value_x = " + value);
								accel_x_vector.addElement(new Double(value));		// Save to vector
								accel_x_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series	
							} 
							else if(i == 1) {
								System.out.println("Accel Value_y = " + value);
								accel_y_vector.addElement(new Double(value));		// Save to vector
								accel_y_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							} else {
								System.out.println("Accel Value_z = " + value);
								accel_z_vector.addElement(new Double(value));		// Save to vector
								accel_z_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							}
						}
						
						// See what the next char is
						check_char = (char)input_chars.read();
						//System.out.println(check_char);
						
						// read temperature data
						value_msb = input_chars.read();
						value_lsb = input_chars.read();
						value = ((short)((value_msb) << 8) | (value_lsb & 0xff))/ 340.0 + 36.53;	
						//System.out.println("Temperature Value = " + p_value);
						temperature_vector.addElement(new Double(value));			// Save to vector
						temperature_ts.addOrUpdate(new Millisecond(), value);		// Save to time	series
						
						// See what the next char is
						check_char = (char)input_chars.read();
						//System.out.println(check_char);
						
						for(int i = 0; i < 3; i++) {
							//construct value from 2 chars
							value_msb = input_chars.read();
							value_lsb = input_chars.read();
							//value = ((short)((value_msb) << 8) | (value_lsb & 0xff)) * 1000 / 65535;
							value = ((short)((value_msb) << 8) | (value_lsb & 0xff)) * 1000 / 65535;

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
						check_char = (char)input_chars.read();	// read W
						System.out.println(check_char);
						
						
						// read quaternions
						for(int i = 0; i < 4; i++) {
							//construct value from 2 chars
							value_msb = input_chars.read();
							value_lsb = input_chars.read();
							value = (float)((short)((value_msb) << 8) | (value_lsb & 0xff)) / 16384.0;					
							if(i == 0) {
								System.out.println("Quaternion_w = " + value);
								quat_w_vector.addElement(new Double(value));		// Save to vector
								quat_w_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							} else if(i == 1) {
								System.out.println("Quaternion_x = " + value);
								quat_x_vector.addElement(new Double(value));		// Save to vector
								quat_x_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							} else if(i == 2) {
								System.out.println("Quaternion_y = " + value);
								quat_y_vector.addElement(new Double(value));		// Save to vector
								quat_y_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							} else {
								System.out.println("Quaternion_z = " + value);
								quat_z_vector.addElement(new Double(value));		// Save to vector
								quat_z_ts.addOrUpdate(new Millisecond(), value);	// Save to time	series
							}
						}
						

						sample_rate = quat_z_ts.getTimePeriod(accel_z_ts.getItemCount()-1);
						sample_rate2 = quat_z_ts.getTimePeriod(accel_z_ts.getItemCount()-2);
						ms1 = sample_rate.getLastMillisecond();
						ms2 = sample_rate2.getLastMillisecond();
						ms_sum += ms1 - ms2;
						ms_total++;
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
						
						validateEndChar = (char)input_chars.read();
						if(validateEndChar != '\n') {
							System.out.println("ERROR, LAST CHARACTER IS NOT \n!");
							//remove elements that were placed in vectors
							accel_x_vector.remove(accel_x_vector.size()-1);
							accel_x_ts.delete(accel_x_ts.getItemCount()-1, accel_x_ts.getItemCount()-1);
							accel_y_vector.remove(accel_y_vector.size()-1);
							accel_y_ts.delete(accel_y_ts.getItemCount()-1, accel_y_ts.getItemCount()-1);
							accel_z_vector.remove(accel_z_vector.size()-1);
							accel_z_ts.delete(accel_z_ts.getItemCount()-1, accel_z_ts.getItemCount()-1);
//							temperature_vector.remove(temperature_vector.size()-1);
//							temperature_ts.delete(temperature_ts.getItemCount()-1, temperature_ts.getItemCount()-1);
							gyro_x_vector.remove(gyro_x_vector.size()-1);
							gyro_x_ts.delete(gyro_x_ts.getItemCount()-1, gyro_x_ts.getItemCount()-1);
							gyro_y_vector.remove(gyro_y_vector.size()-1);
							gyro_y_ts.delete(gyro_y_ts.getItemCount()-1, gyro_y_ts.getItemCount()-1);
							gyro_z_vector.remove(gyro_z_vector.size()-1);
							gyro_z_ts.delete(gyro_z_ts.getItemCount()-1, gyro_z_ts.getItemCount()-1);
							quat_w_vector.remove(quat_w_vector.size()-1);
							quat_w_ts.delete(quat_w_ts.getItemCount()-1, quat_w_ts.getItemCount()-1);
							quat_x_vector.remove(quat_x_vector.size()-1);
							quat_x_ts.delete(quat_x_ts.getItemCount()-1, quat_x_ts.getItemCount()-1);
							quat_y_vector.remove(quat_y_vector.size()-1);
							quat_y_ts.delete(quat_y_ts.getItemCount()-1, quat_y_ts.getItemCount()-1);
							quat_z_vector.remove(quat_z_vector.size()-1);
							quat_z_ts.delete(quat_z_ts.getItemCount()-1, quat_z_ts.getItemCount()-1);
							//Thread.sleep(2000);
							//validateEndChar = '\n';
							break;
						}
					} while(validateEndChar != '\n');	
				}
				c++;
				System.out.println("c = " + c);

				if (c == 1000){
					close();	
					ms_final = ms_sum / ms_total;
					System.out.println("Sample Rate = " + ms_final);
					System.out.println("finished closing");
					System.out.println("STARTING WRITING TO FILE");
					writeToFile();
					System.out.println("ENDING WRITING TO FILE");
					Runtime.getRuntime().exec("matlab -r antonio");
					
					//Thread.currentThread().interrupt();
					return;							
				}
			} catch (Exception e) {
				//Thread.currentThread().interrupt();
				return;
				//System.err.println(e.toString());
			}
		}		
	}
	
	public static void writeToFile() {
        PrintStream out = null;
 
        Iterator<Double> accel_x_itr = accel_x_vector.iterator();
        Iterator<Double> accel_y_itr = accel_y_vector.iterator();
        Iterator<Double> accel_z_itr = accel_z_vector.iterator();
        //Iterator<Double> temperature_itr = temperature_vector.iterator();
        Iterator<Double> gyro_x_itr = gyro_x_vector.iterator();
        Iterator<Double> gyro_y_itr = gyro_y_vector.iterator();
        Iterator<Double> gyro_z_itr = gyro_z_vector.iterator();
        Iterator<Double> quat_w_itr = quat_w_vector.iterator();
        Iterator<Double> quat_x_itr = quat_x_vector.iterator();
        Iterator<Double> quat_y_itr = quat_y_vector.iterator();
        Iterator<Double> quat_z_itr = quat_z_vector.iterator();
        
        		
        try {
            System.out.println("Start writing to file.");
            out = new PrintStream(new FileOutputStream(FILENAME, true));
            
            DateFormat df = new SimpleDateFormat("MM/dd/yy HH:mm:ss");
            Date dateobj = new Date();
 	            
            out.println("Start Time: " + df.format(dateobj));          
            out.println("sensor" + "," + "value");

            while(accel_x_itr.hasNext()) {
            	out.println("Acceleration_x" + "," + accel_x_itr.next());
            	out.println("Acceleration_y" + "," + accel_y_itr.next());
            	out.println("Acceleration_z" + "," + accel_z_itr.next());
            	//out.println("Temperature" + "," + temperature_itr.next());
            	out.println("Gyro_x" + "," + gyro_x_itr.next());
            	out.println("Gyro_y" + "," + gyro_y_itr.next());
            	out.println("Gyro_z" + "," + gyro_z_itr.next());
            	out.println("Quat_w" + "," + quat_w_itr.next());
            	out.println("Quat_x" + "," + quat_x_itr.next());
            	out.println("Quat_y" + "," + quat_y_itr.next());
            	out.println("Quat_z" + "," + quat_z_itr.next());
            }
        } catch (ArrayIndexOutOfBoundsException e) {
            System.err.println("ArrayIndexOutOfBoundsException Error:" +
                 e.getMessage());
        } catch (IOException e) {
            System.err.println("IOException: " + e.getMessage());
        } finally {
            if (out != null) {
                System.out.println("PrintStream");
                out.close();
            } else {
                System.out.println("Couldn't open connection");
            }
        }
    }
	
	private static JFreeChart createChart(TimeSeriesCollection dataset, String title, String dataType) {
		
		JFreeChart chart = ChartFactory.createTimeSeriesChart(
				title,  							// title
				"Time",           					// x-axis label
				"Value (" + dataType + ")",   		// y-axis label
				dataset,            				// data
				true,              					// create legend?
				true,               				// generate tooltips?
				false               				// generate URLs?
				);

      final XYPlot plot = chart.getXYPlot();
      ValueAxis axis = plot.getDomainAxis();
    
      return chart;
	}
	
//	public static char constructVal() {
//		int value, value_msb, value_lsb;
//		double p_value;
//		char char_check;
//		
//		value_msb = input_chars.read();
//		check_char = (char)value_msb;
//		value_lsb = input_chars.read();
//		
//		// Construct value from 2 bytes
//		//value = value_msb << 8 | value_lsb;					
//		//p_value = (double)value / 939.650784;	
//		
//		return char_check;
//		}
	
	/**
	 * Main.
	 * 
	 */
	public static void main(String[] args) throws Exception {

		// for serial streaming
		SerialTest main = new SerialTest();
		main.initialize();
//		Thread t = new Thread() {
//			public void run( ) {
//				//the following line will keep this app alive for 100000 seconds,
//				//waiting for events to occur and responding to them (printing incoming messages to console).
//				
//				while(!exit){
//					try {
//						System.out.println("In run");
//						Thread.sleep(100000000);
//						
//					} catch (InterruptedException ie) {
//						//Thread.currentThread().interrupt();
//					}
//				}
//				System.out.println("Exiting Thread!");
//			}
//		};
		
		//serialThread t = new serialThread();
		
		//t.run();
//		System.out.println("Started");
//		
//        String get = "aaa";
//		System.out.println("Started2");
		
		//t.stop();

		
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
        

        //System.out.println("gg1");
        //writeToFile();
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
//        NumberAxis accel_x_axis_y = (NumberAxis) accel_x_plot.getRangeAxis();
//        accel_x_axis_y.setRange(-4.0, 4.0);
        accel_x_axis.setAutoRange(true);
        //accel_x_axis.setFixedAutoRange(60000.0); 
		
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
//        NumberAxis accel_y_axis_y = (NumberAxis) accel_y_plot.getRangeAxis();
//        accel_y_axis_y.setRange(-4.0, 4.0);
        accel_y_axis.setAutoRange(true);
        //accel_y_axis.setFixedAutoRange(60000.0);       

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
//        NumberAxis accel_z_axis_y = (NumberAxis) accel_z_plot.getRangeAxis();
//        accel_z_axis_y.setRange(-4.0, 4.0);
        accel_z_axis.setAutoRange(true);
        //accel_z_axis.setFixedAutoRange(60000.0);
        
        // Temperature
        TimeSeriesCollection temperature_dataset = new TimeSeriesCollection(temperature_ts);
        JFreeChart temperature_chart = ChartFactory.createTimeSeriesChart(
            "Temperature",
            "Time",
            "Value (C)",
            temperature_dataset,
            true,
            true,
            false
        );
        final XYPlot temperature_plot = temperature_chart.getXYPlot();
        ValueAxis temperature_axis = temperature_plot.getDomainAxis();
        temperature_axis.setAutoRange(true);
        //temperature_axis.setFixedAutoRange(60000.0);
        
		// Gyro X
        TimeSeriesCollection gyro_x_dataset = new TimeSeriesCollection(gyro_x_ts);
        JFreeChart gyro_x_chart = ChartFactory.createTimeSeriesChart(
            "X Gyro",
            "Time",
            "Value",
            gyro_x_dataset,
            true,
            true,
            false
        );
        final XYPlot gyro_x_plot = gyro_x_chart.getXYPlot();
        ValueAxis gyro_x_axis = gyro_x_plot.getDomainAxis();

        //gyro_x_axis.setAutoRange(true);
        //gyro_x_axis.setFixedAutoRange(60000.0); 
		
        // Gyro Y
        TimeSeriesCollection gyro_y_dataset = new TimeSeriesCollection(gyro_y_ts);
        JFreeChart gyro_y_chart = ChartFactory.createTimeSeriesChart(
            "Y Gyro",
            "Time",
            "Value",
            gyro_y_dataset,
            true,
            true,
            false
        );
        final XYPlot gyro_y_plot = gyro_y_chart.getXYPlot();
        ValueAxis gyro_y_axis = gyro_y_plot.getDomainAxis();
        gyro_y_axis.setAutoRange(true);
        //gyro_y_axis.setFixedAutoRange(60000.0);       

        // Gyro Z
        TimeSeriesCollection gyro_z_dataset = new TimeSeriesCollection(gyro_z_ts);
        JFreeChart gyro_z_chart = ChartFactory.createTimeSeriesChart(
            "Z Gyro",
            "Time",
            "Value",
            gyro_z_dataset,
            true,
            true,
            false
        );
        final XYPlot gyro_z_plot = gyro_z_chart.getXYPlot();
        ValueAxis gyro_z_axis = gyro_z_plot.getDomainAxis();
        gyro_z_axis.setAutoRange(true);
        //gyro_z_axis.setFixedAutoRange(60000.0);
       
       JFrame accel_frame = new JFrame("Acceleration Plots"); // frame is the window that holds the panels (these panels contain real time plots)
       accel_frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
       JFrame temperature_frame = new JFrame("Temperature_frame Plots"); // frame is the window that holds the panels (these panels contain real time plots)
       temperature_frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
       
       JFrame gyro_frame = new JFrame("Gyro Plots"); // frame is the window that holds the panels (these panels contain real time plots)
       gyro_frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
       
       // Acceleration X
       ChartPanel accel_x_label = new ChartPanel(accel_x_chart);
       accel_frame.getContentPane().add(accel_x_label, BorderLayout.WEST);
       
       // Acceleration Y
       ChartPanel accel_y_label = new ChartPanel(accel_y_chart);
       accel_frame.getContentPane().add(accel_y_label, BorderLayout.SOUTH);
       
       // Acceleration Z
       ChartPanel accel_z_label = new ChartPanel(accel_z_chart);
       accel_frame.getContentPane().add(accel_z_label, BorderLayout.EAST);
       
       accel_frame.pack();
       accel_frame.setVisible(true); 
       
       // Temperature
       ChartPanel temperature_label = new ChartPanel(temperature_chart);
       temperature_frame.getContentPane().add(temperature_label, BorderLayout.CENTER);
       
       temperature_frame.pack();
       temperature_frame.setVisible(true); 
       
       // Gyro X
       ChartPanel gyro_x_label = new ChartPanel(gyro_x_chart);
       gyro_frame.getContentPane().add(gyro_x_label, BorderLayout.WEST);
       
       // Gyro Y
       ChartPanel gyro_y_label = new ChartPanel(gyro_y_chart);
       gyro_frame.getContentPane().add(gyro_y_label, BorderLayout.SOUTH);
       
       // Gyro Z
       ChartPanel gyro_z_label = new ChartPanel(gyro_z_chart);
       gyro_frame.getContentPane().add(gyro_z_label, BorderLayout.EAST);
        
       gyro_frame.pack();
       gyro_frame.setVisible(true);   
	}
}
