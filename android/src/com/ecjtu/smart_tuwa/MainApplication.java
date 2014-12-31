package com.ecjtu.smart_tuwa;

import android.app.Application;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

public class MainApplication extends Application{
	private static final String DEFAULT_TARGET_IP = "192.168.1.129";
	private static final int DEFAULT_SOCKET_PORT = 2006;
	private static final int DEFAULT_SOCKET_PORT_LOCAL = 2006;
	private static final String DEFAULT_CMD_ON = "A";
	private static final String DEFAULT_CMD_OFF = "B";
	private static final String DEFAULT_CMD_HB = "C";
	
	private String targetIp;
	private int socketPort;
	private int socketPortLocal;
	private String cmdOn;
	private String cmdOff;
	private String cmdHb;
	
	@Override 
	public void onCreate() {
		super.onCreate();
		SharedPreferences preference = getSharedPreferences("config",Context.MODE_PRIVATE);
		Editor editor = preference.edit();		
		targetIp = preference.getString("target_ip", DEFAULT_TARGET_IP);
		socketPort = preference.getInt("socket_port", DEFAULT_SOCKET_PORT);
		socketPortLocal = preference.getInt("socket_port_local", DEFAULT_SOCKET_PORT);
		cmdOn = preference.getString("cmd_on", DEFAULT_CMD_ON);
		cmdOff = preference.getString("cmd_off", DEFAULT_CMD_OFF);
		cmdHb = preference.getString("cmd_hb", DEFAULT_CMD_HB);
		
		editor.putString("target_ip", targetIp);
		editor.putInt("socket_port", socketPort);
		editor.putInt("socket_port_local", socketPortLocal);
		editor.putString("cmd_on", cmdOn);
		editor.putString("cmd_off", cmdOff);
		editor.putString("cmd_hb", cmdHb);
		editor.commit();
	}
	
	public String getDefaultTargetIp(){
		return DEFAULT_TARGET_IP;
	}
	
	public int getDefaultSocketPort(){
		return DEFAULT_SOCKET_PORT;
	}
	
	public int getDefaultSocketPortLocal(){
		return DEFAULT_SOCKET_PORT_LOCAL;
	}
	
	public String getDefaultCmdOn(){
		return DEFAULT_CMD_ON;
	}
	
	public String getDefaultCmdOff(){
		return DEFAULT_CMD_OFF;
	}
	
	public String getDefaultCmdHb(){
		return DEFAULT_CMD_HB;
	}
	
	public String getTargetIp(){
		return targetIp;
	}
	
	public int getSocketPort(){
		return socketPort;		
	}
	
	public int getSocketPortLocal(){
		return socketPortLocal;
	}
	
	public String getCmdOn(){
		return cmdOn;
	}
	
	public String getCmdOff(){
		return cmdOff;
	}
	
	public String getCmdHb(){
		return cmdHb;
	}
	
	public void setTargetIp(String val){
		targetIp = val;
	}
	
	public void setSocketPort(int val){
		socketPort = val;
	}
	
	public void setSocketPortLocal(int val){
		socketPortLocal = val;
	}
	
	public void setCmdOn(String val){
		cmdOn = val;
	}
	
	public void setCmdOff(String val){
		cmdOff = val;
	}
	
	public void setCmdHb(String val){
		cmdHb = val;
	}
	
	public void setDefaults(){
		targetIp = this.getDefaultTargetIp();
		socketPort = this.getDefaultSocketPort();
		socketPortLocal = this.getDefaultSocketPortLocal();
		cmdOn = this.getDefaultCmdOn();
		cmdOff = this.getDefaultCmdOff();
		cmdHb = this.getDefaultCmdHb();
	}	
}
