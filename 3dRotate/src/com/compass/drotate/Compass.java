package com.compass.drotate;



import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;
import android.view.WindowManager;


public class Compass implements SensorEventListener {
	
	public static class CompassData {
		static float yaw;
		static float pitch;
		static float roll;
	};
	static float[] inR = new float[16];
	float[] I = new float[16];
	float[] gravity = new float[3];
	float[] geomag = new float[3];
	float[] orientVals = new float[3];
	
	
	private SensorManager mSensorManager;

	  
	static float yaw, pitch, roll;
	private float prev_yaw, prev_pitch, prev_roll;
	int offset = 0;
	
	RotateActivity.Direction direction  = RotateActivity.Direction.right;

	public static CompassData compassData;
	
	public Compass(Context context) {
		mSensorManager = (SensorManager)context.getSystemService(Context.SENSOR_SERVICE);
	}


	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// TODO Auto-generated method stub
		
	}
	float mod(float num) {
		if(num < 0) 
			return num*-1;
		else return num;
	}


	static final float ALPHA = 0.15f;
	protected float[] lowPass(float[] input, float[] output) {
		if (output == null)
			return input;

		for (int i = 0; i < input.length; i++) {
			output[i] = output[i] + ALPHA * (input[i] - output[i]);
		}
		return output;
	}
	@Override
	public void onSensorChanged(SensorEvent sensorEvent) {

		 // If the sensor data is unreliable return
	    if (sensorEvent.accuracy == SensorManager.SENSOR_STATUS_UNRELIABLE)
	        return;

	    // Gets the value of the sensor that has been changed
	    switch (sensorEvent.sensor.getType()) {  
	        case Sensor.TYPE_ACCELEROMETER:
	            gravity = lowPass( sensorEvent.values.clone(), gravity);
	            break;
	        case Sensor.TYPE_MAGNETIC_FIELD:
	            geomag  = lowPass( sensorEvent.values.clone(), geomag);
	            break;
	        // does not work for all phones (must have a gyroscope)
	        case Sensor.TYPE_ROTATION_VECTOR:
		        inR=new float[16];
		        SensorManager.getRotationMatrixFromVector(inR, sensorEvent.values);
	    }

	    // If gravity and geomag have values then find rotation matrix
	    if (gravity != null && geomag != null) {

	        // checks that the rotation matrix is found
	        boolean success = SensorManager.getRotationMatrix(inR, I,
	                                                          gravity, geomag);
	        
	        
	        
	        //boolean remapped = SensorManager.remapCoordinateSystem(inR, axisX, axisY, inR);
	        
	    }
	        
		// TODO Auto-generated method stub
		// get the angle around the z-axis rotated	    	
		float current_yaw, current_pitch, current_roll;
		current_yaw = Math.round(sensorEvent.values[0]);
		current_pitch = Math.round(sensorEvent.values[1]);
		current_roll = Math.round(sensorEvent.values[2]);
		
						
		yaw = current_yaw;
		pitch = current_pitch;
		roll = current_roll;
		
		
		RotateActivity.tvYaw.setText("Yaw: " + Float.toString(yaw) + " degrees");
		RotateActivity.tvPitch.setText("Pitch: " + Float.toString(pitch) + " degrees");
		RotateActivity.tvRoll.setText("Roll: " + Float.toString(roll) + " degrees");
		

		if (prev_yaw < yaw) {
			direction = RotateActivity.Direction.left;
		}
		if (prev_yaw > yaw) { 
			direction = RotateActivity.Direction.right;
		} 
		if(prev_yaw == yaw) {
			direction = RotateActivity.Direction.notTurning;
		}
		
		  if(prev_yaw > yaw) { direction = RotateActivity.Direction.left; }
		 if(prev_yaw < yaw ) { direction = RotateActivity.Direction.right; }
		  if(prev_pitch > pitch) { direction = RotateActivity.Direction.up; }
		  if(prev_pitch < pitch) { direction = RotateActivity.Direction.down; }
		  if((prev_yaw == yaw) && (prev_pitch == yaw)) { direction =
		  RotateActivity.Direction.notTurning; }
		 
		 
		if(offset == 0) {
			prev_yaw = yaw; 
			prev_pitch = pitch;
			prev_roll = roll;
			offset = 5;
		}
		offset--;
		
	}
	 protected void register() { 		    
     	 mSensorManager.registerListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION),
				 SensorManager.SENSOR_DELAY_GAME);
		mSensorManager.registerListener(this,
				mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD),
				SensorManager.SENSOR_DELAY_GAME);
		mSensorManager.registerListener(this,
				mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
				SensorManager.SENSOR_DELAY_GAME);
     	 
     }
     protected void unregister() {       
         mSensorManager.unregisterListener(this);
     }
     com.compass.drotate.RotateActivity.Direction calculateDirection() {    	 
		return direction; 		
 	}
}
