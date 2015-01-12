package com.ecjtu.smart_tuwa;

import java.io.UnsupportedEncodingException;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

public class ControlActivity extends Activity {

	private final int REQUEST_CODE_SET = 1;
	
	private final int RESULT_CODE_SET = 1;
	private final int RESULT_CODE_CANCEL = 2;
	
	private ImageButton btnSwitch;
	private ImageButton btnConn;	
	
	private boolean isConnected;
	private boolean isSwitchOn;
	
	TextView textConn;
	
	SimpleSocketClient socketClient;
	private MainApplication mainApp;	
	
	String cmdOn;
	String cmdOff;
	
	Handler handler = new Handler(){		
		public void handleMessage(Message msg){
			switch(msg.what){
			case SimpleSocketClient.MSG_CONNECTED:
				isConnected = true;
				Toast.makeText(getApplicationContext(), "Connected",  Toast.LENGTH_SHORT).show();
				btnConn.setImageDrawable(getResources().getDrawable(R.drawable.connect));
				break;
			case SimpleSocketClient.MSG_DISCONNECTED:
				isConnected = false;
				Toast.makeText(getApplicationContext(), "Disconnected",  Toast.LENGTH_SHORT).show();
				btnConn.setImageDrawable(getResources().getDrawable(R.drawable.disconnected));
				break;
			case SimpleSocketClient.MSG_READ:
				process(msg.arg1, (byte[])msg.obj);
				break;
			case AppMessage.MSG_CONTROL_SEND:
				socketClient.write((String) msg.obj);
				break;
			case AppMessage.MSG_OPOTION_SET:
				socketClient.setCmdHb(mainApp.getCmdHb());
				cmdOn = mainApp.getCmdOn();
				cmdOff = mainApp.getCmdOff();
			}
		}
	};
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_control);		
		
		isConnected = false;
		isSwitchOn = false;
		
		btnConn = (ImageButton) this.findViewById(R.id.btnConn);
		btnConn.setImageDrawable(getResources().getDrawable(R.drawable.disconnected));
		btnSwitch = (ImageButton) this.findViewById(R.id.btnSwitch);
		btnSwitch.setImageDrawable(getResources().getDrawable(R.drawable.switch_off));
		textConn = (TextView) this.findViewById(R.id.textConn);				
		mainApp = (MainApplication) getApplication();
		cmdOn = mainApp.getCmdOn();
		cmdOff = mainApp.getCmdOff();
		
		btnSwitch.setOnClickListener(new ImageButton.OnClickListener(){

			@Override
			public void onClick(View v) {
				if(isConnected){
					if(!isSwitchOn){
						handler.obtainMessage(AppMessage.MSG_CONTROL_SEND,-1,-1,cmdOn).sendToTarget();
						isSwitchOn = true;
						btnSwitch.setImageDrawable(getResources().getDrawable(R.drawable.switch_on));
					}
					else{
						handler.obtainMessage(AppMessage.MSG_CONTROL_SEND,-1,-1,cmdOff).sendToTarget();
						isSwitchOn = false;
						btnSwitch.setImageDrawable(getResources().getDrawable(R.drawable.switch_off));
						}					
				}else{
					Toast.makeText(getApplicationContext(), "Not connected", Toast.LENGTH_SHORT).show();
				}
				
			}
			
		});
		
		btnConn.setOnClickListener(new ImageButton.OnClickListener(){

			@Override
			public void onClick(View v) {
				if(isConnected){
					socketClient.disconnect();
				}else{
					socketClient.connect();
				}
				
			}
			
		});
		
		socketClient = new SimpleSocketClient(mainApp.getTargetIp(),mainApp.getSocketPort(),handler);
		socketClient.setCmdHb(mainApp.getCmdHb());
	}
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu){
		menu.add(0,1,1,"Option");
		return super.onCreateOptionsMenu(menu);
	}
	@Override
	public boolean onOptionsItemSelected(MenuItem item){
		if(item.getItemId() == 1){
			Intent intent = new Intent();
			intent.setClass(ControlActivity.this, OptionActivity.class);
			startActivityForResult(intent, REQUEST_CODE_SET);
		}
		return false;		
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data){
		if(REQUEST_CODE_SET  == requestCode){
			switch(resultCode){
			case RESULT_CODE_SET:
				//Reset the connection
				Toast.makeText(getApplicationContext(), "Paras changed",  Toast.LENGTH_SHORT).show();
				handler.sendEmptyMessage(AppMessage.MSG_OPOTION_SET);
				break;
			case RESULT_CODE_CANCEL:
				Toast.makeText(getApplicationContext(), "Paras unchanged",  Toast.LENGTH_SHORT).show();
				//Do nothing
				break;
			}
		}
	}
	
	public void process(int len, byte[] data){
		byte[] val = new byte[len];
		for(int m = 0; m<len; m++){
			val[m] = data[m];
		}		
		try {
			textConn.setText(new String(val,"UTF8"));
		} catch (UnsupportedEncodingException e) {		
			e.printStackTrace();
		}		
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event){
		if(keyCode == KeyEvent.KEYCODE_BACK && event.getAction() == KeyEvent.ACTION_DOWN){
			socketClient.disconnect();
			finish();
			System.exit(0);
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}
}
