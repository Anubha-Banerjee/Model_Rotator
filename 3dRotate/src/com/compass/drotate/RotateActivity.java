package com.compass.drotate;


import java.io.BufferedReader;
import java.io.Console;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

import android.hardware.SensorManager;
import android.os.Bundle;
import android.app.Activity; 
import android.content.pm.ActivityInfo;
import android.util.Log;
import android.view.Menu;
import android.view.Surface;
import android.view.WindowManager;
import android.webkit.ConsoleMessage;
import android.widget.TextView;

public class RotateActivity extends Activity {

	Compass compass;
	enum Direction{
		left,
		right,
		up,
		down,
		notTurning
	};
	
	public static TextView tvYaw, tvPitch, tvRoll;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_rotate);
		tvYaw = (TextView) findViewById(R.id.yaw);
		tvPitch = (TextView) findViewById(R.id.pitch);
		tvRoll = (TextView) findViewById(R.id.roll);
		compass = new Compass(this); 
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
		new Thread(new Runnable() { 
	        public void run() { 
	        	Socket sock;  
	    		String serverMsg = null;    
	    		boolean again = true; 
	        	  try {     
	      			sock = new Socket("192.168.1.8", 9797);  
	      			    
	      		    PrintWriter outp = new PrintWriter(sock.getOutputStream(), true);
	      		    BufferedReader inp = new BufferedReader(new InputStreamReader(sock.getInputStream()));
	      		   
	      		    serverMsg = inp.readLine(); 	      	  
	      		  	 
	      		    while(again) {	      		      		    	 
	      		    	
	      		    	//Direction directionName = compass.calculateDirection();
	      		    	//outp.println(directionName);
	      		    	//delay(100);
	      		    	//outp.println("====");
	      		    	//outp.println(Compass.yaw);
	      		    	String RotationMatrix = ""; 
	      		        int rotation = getWindowManager().getDefaultDisplay().getRotation();
	      		        int axisX = 0, axisY = 0;
						switch (rotation) {
						case Surface.ROTATION_0:
							axisX = SensorManager.AXIS_X;
							axisY = SensorManager.AXIS_Y;
							break;

						case Surface.ROTATION_90:
							axisX = SensorManager.AXIS_Y;
							axisY = SensorManager.AXIS_MINUS_X;
							break;

						case Surface.ROTATION_180:
							axisX = SensorManager.AXIS_MINUS_X;
							axisY = SensorManager.AXIS_MINUS_Y;
							break;

						case Surface.ROTATION_270:
							axisX = SensorManager.AXIS_MINUS_Y;
							axisY = SensorManager.AXIS_X;
							break;

						default:
							break;
						}
						Log.e("e" , "rotn : " + rotation);
						SensorManager.remapCoordinateSystem(Compass.inR, axisX, axisY, Compass.inR);
	      		    	for(int i = 0 ; i < 16 ; i++) {
	      		    		RotationMatrix = RotationMatrix + Compass.inR[i] + "\n";	      		    		
	      		    	}
	   
	      		    	outp.println("S"+RotationMatrix+"X");
	      		    	
	      		    	//outp.println("0000");
	      		    	
	      		    	//Log.e("Yaw" , "yaw is: " + Compass.yaw);
	      		    	//Log.e("Pitch" , "pitch is: " + Compass.pitch);
	      		    	//Log.e("Roll" , "roll is: " + Compass.roll);
	      		    	
	      		    	/*outp.println(Compass.yaw);
	      		    	outp.println(Compass.pitch);
	      		    	outp.println(Compass.roll);*/
	      		    	//outp.println('x');
	      		    	delay(100); 
	      		
	      		    
	      		    }
	      		} catch (IOException e) {
	      			// TODO Auto-generated catch block
	      			e.printStackTrace();
	      		}
	        }
	    }).start();	  
	}
	private static void delay(int i) {
		// TODO Auto-generated method stub
		try {
		    Thread.sleep(i);
		} catch(InterruptedException ex) {
		    Thread.currentThread().interrupt();
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.rotate, menu);
		return true;
	}
	
	@Override
	protected void onResume() {
		super.onResume();
		compass.register();
	}

	@Override
	protected void onPause() {
		super.onPause();
		compass.unregister();
	}

}
