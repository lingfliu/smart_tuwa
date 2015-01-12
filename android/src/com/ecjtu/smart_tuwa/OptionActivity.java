package com.ecjtu.smart_tuwa;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

public class OptionActivity extends Activity {

	private String targetIp;
	private int socketPort;
	private int socketPortLocal;
	private String cmdOn;
	private String cmdOff;
	private String cmdHb;
	
	private Button btnSet;
	private Button btnCancel;
	private Button btnDefault;
	
	private EditText editTargetIp;
	private EditText editSocketPort;
	private EditText editSocketPortLocal;	
	private EditText editCmdOn;
	private EditText editCmdOff;
	private EditText editCmdHb;

	private MainApplication mainApp;	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_option);
		
		mainApp = (MainApplication) this.getApplication();
		targetIp = mainApp.getTargetIp();
		socketPort = mainApp.getSocketPort();
		socketPortLocal = mainApp.getSocketPortLocal();
		cmdOn = mainApp.getCmdOn();
		cmdOff = mainApp.getCmdOff();
		cmdHb = mainApp.getCmdHb();
		
		editTargetIp = (EditText) this.findViewById(R.id.editTargetIp);
		editTargetIp.setText(targetIp);
		
		editSocketPort = (EditText) this.findViewById(R.id.editSocketPort);
		editSocketPort.setText(Integer.toString(socketPort));
		
		editSocketPortLocal = (EditText) this.findViewById(R.id.editSocketPortLocal);
		editSocketPortLocal.setText(Integer.toString(socketPortLocal));		
		
		editCmdOn = (EditText) this.findViewById(R.id.editCmdOn);
		editCmdOn.setText(cmdOn);		
		editCmdOff = (EditText) this.findViewById(R.id.editCmdOff);
		editCmdOff.setText(cmdOff);
		editCmdHb = (EditText) this.findViewById(R.id.editCmdHeartbeat);
		editCmdHb.setText(cmdHb);
		btnSet = (Button) this.findViewById(R.id.btnSet);
		btnSet.setOnClickListener(new Button.OnClickListener(){

			@Override
			public void onClick(View v) {				
				if(editTargetIp.getText().toString().isEmpty()){
					Toast.makeText(getApplicationContext(), "Target IP not set",  Toast.LENGTH_SHORT).show();
					return;
				}
				if(editSocketPort.getText().toString().isEmpty()){
					Toast.makeText(getApplicationContext(), "Port not set",  Toast.LENGTH_SHORT).show();
					return;
				}
				if(editSocketPortLocal.getText().toString().isEmpty()){
					Toast.makeText(getApplicationContext(), "Local port not set",  Toast.LENGTH_SHORT).show();
					return;
				}
				if(editCmdOn.getText().toString().isEmpty()){
					Toast.makeText(getApplicationContext(), "Cmd ON not set",  Toast.LENGTH_SHORT).show();
					return;
				}
				if(editCmdOff.getText().toString().isEmpty()){
					Toast.makeText(getApplicationContext(), "Cmd OFF not set",  Toast.LENGTH_SHORT).show();
					return;
				}
				if(editCmdHb.getText().toString().isEmpty()){
					Toast.makeText(getApplicationContext(), "Cmd HB not set",  Toast.LENGTH_SHORT).show();
					return;
				}
				
				targetIp = editTargetIp.getText().toString();
				socketPort = Integer.parseInt(editSocketPort.getText().toString());
				socketPortLocal = Integer.parseInt(editSocketPortLocal.getText().toString());
				cmdOn = editCmdOn.getText().toString();
				cmdOff = editCmdOff.getText().toString();
				cmdHb = editCmdHb.getText().toString();
				
				
				mainApp.setTargetIp(targetIp);
				mainApp.setSocketPort(socketPort);
				mainApp.setSocketPortLocal(socketPortLocal);
				mainApp.setCmdOn(cmdOn);
				mainApp.setCmdOff(cmdOff);
				mainApp.setCmdHb(cmdHb);
				
				SharedPreferences preference = getSharedPreferences("config",Context.MODE_PRIVATE);
				Editor editor = preference.edit();						
				editor.putString("target_ip", targetIp);
				editor.putInt("socket_port", socketPort);
				editor.putInt("socket_port_local", socketPortLocal);
				editor.putString("cmd_on", cmdOn);
				editor.putString("cmd_off", cmdOff);
				editor.putString("cmd_hb", cmdHb);
				
				editor.commit();
				
				setResult(1,getIntent());		
				finish();
			}
			
		});
		
		btnDefault = (Button) this.findViewById(R.id.btnDefault);
		btnDefault.setOnClickListener(new Button.OnClickListener(){

			@Override
			public void onClick(View v) {				
				editTargetIp.setText(mainApp.getDefaultTargetIp());
				editSocketPort.setText(Integer.toString(mainApp.getDefaultSocketPort()));
				editSocketPortLocal.setText(Integer.toString(mainApp.getDefaultSocketPortLocal()));
				editCmdOn.setText(mainApp.getDefaultCmdOn());
				editCmdOff.setText(mainApp.getDefaultCmdOff());
				editCmdHb.setText(mainApp.getDefaultCmdHb());
				
				targetIp = mainApp.getDefaultTargetIp();
				socketPort = mainApp.getDefaultSocketPort();
				socketPortLocal = mainApp.getDefaultSocketPortLocal();
				cmdOn = mainApp.getDefaultCmdOn();
				cmdOff = mainApp.getDefaultCmdOff();
				cmdHb = mainApp.getDefaultCmdHb();												
			}			
		});
		
		btnCancel = (Button) this.findViewById(R.id.btnCancel);
		btnCancel.setOnClickListener(new Button.OnClickListener(){

			@Override
			public void onClick(View v) {								
				setResult(2,getIntent());
				finish();
			}			
		});
		
		setResult(2,getIntent());
	}
		
}
