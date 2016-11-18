import java.io.*;

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
import org.jfree.io.IOUtils;

//import java.io.FileNotFoundException;
//import java.io.FileReader;
//import java.io.IOException;
//import java.util.Iterator;

//TODO: Make sure plots stay open after data reading is complete.
//TODO: Save data to object to analyze later? Or is text file good enough?

public class SerialTest_testing_file {
	
	// The name of the file to open.
    static String fileName = "demosat_data_SAMPLE.txt";
    
    // Parser Object
    static JSONParser parser = new JSONParser();

    static TimeSeries ts = new TimeSeries("data", Millisecond.class);
    static TimeSeries ts2 = new TimeSeries("data2", Millisecond.class);
    
    //public static Vector tempVector = new Vector();
    //public static Vector luminosityVector = new Vector();
    public static Vector tempVector = new Vector(3,1);
    
    /**
     * initialize() function
     * This function finds an open serial port, opens the port with its parameters,
     * opens the input and output streams for the port, and adds port listeners.
     */
	
	

	
	/**
	 * This should be called when you stop using the port.
	 * This will prevent port locking on platforms like Linux.
	 */
	

	
	/**
	 * Handle an event on the serial port. Read the data, fix to valid JSON, parse each JSON line,
	 * save each value and unit in a dynamic vector, save each full JSON line with date
	 * to a file, and print each parsed JSON line.
	 */
	

	

	/**
	 * Main.
	 * 
	 */
	public static void main(String[] args) throws Exception {

		// for serial streaming
		SerialTest_testing_file main = new SerialTest_testing_file();
		//main.initialize();
		/*Thread t=new Thread() {
			public void run() {
				//the following line will keep this app alive for 100000 seconds,
				//waiting for events to occur and responding to them (printing incoming messages to console).
				try {Thread.sleep(100000000);} catch (InterruptedException ie) {}
			}
		};*/
		//t.start();
		System.out.println("Started");
		
	 	System.out.println("Size: " + tempVector.capacity() + "\n");

		
		String inputLine = null;
		
		try {
            // FileReader reads text files in the default encoding.
            FileReader fileReader = new FileReader(fileName);

            // Always wrap FileReader in BufferedReader.
            BufferedReader bufferedReader = new BufferedReader(fileReader);

            while((inputLine = bufferedReader.readLine()) != null) {
            	
            	System.out.println("Line = " + inputLine);
           	 
            	
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
	            		 
	            	 	 System.out.println("Vector Element: " + (Double)tempVector.lastElement() + "\n");
	            		// enumerate the elements in the vector.
	            	 	 
	            	 	 System.out.println("Size: " + tempVector.capacity() + "\n");
	            		 
	            	 }        	 	            		 	            	 
           	} 
           	catch (ParseException e) {
           			//e.printStackTrace();
           			System.out.println("Incomplete data. Setting current line to null.\n");
           			inputLine = null;           			
           		}
            	
       		 Enumeration tvEnum = tempVector.elements(); 
       		 while(tvEnum.hasMoreElements())
       		 {
       			 System.out.print("VECTOR DATA:" + tvEnum.nextElement() + " ");
       			 System.out.println(); 
       		 }
            	
            }   

            // Always close files.
            bufferedReader.close();         
        }
        catch(FileNotFoundException ex) {
            System.out.println(
                "Unable to open file '" + 
                fileName + "'");                
        }
        catch(IOException ex) {
            System.out.println(
                "Error reading file '" 
                + fileName + "'");                  
            // Or we could just do this: 
            // ex.printStackTrace();
        }
		/*
		try(FileWriter fw = new FileWriter("outFile.txt", true);
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
						
	    // for real time plotting
        TimeSeriesCollection dataset = new TimeSeriesCollection(ts);
        JFreeChart chart = ChartFactory.createTimeSeriesChart(
            "Temperature",
            "Time",
            "Value (C)",
            dataset,
            true,
            true,
            false
        );
        final XYPlot plot = chart.getXYPlot();
        ValueAxis axis = plot.getDomainAxis();
        axis.setAutoRange(true);
        axis.setFixedAutoRange(60000.0); 
        
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
       
       JFrame frame = new JFrame("GraphTest"); // frame is the window that holds the panels (these panels contain real time plots)
       frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
      
       ChartPanel label = new ChartPanel(chart);
       frame.getContentPane().add(label, BorderLayout.WEST);

       ChartPanel label2 = new ChartPanel(chart2);
       frame.getContentPane().add(label2, BorderLayout.EAST);
        
       frame.pack();
       frame.setVisible(true);          		
	}
}
