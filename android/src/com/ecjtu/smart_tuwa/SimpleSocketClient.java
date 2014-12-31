package com.ecjtu.smart_tuwa;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class SimpleSocketClient {
	public static final int MSG_CONNECTED = 1;
	public static final int MSG_DISCONNECTED = 2;
	public static final int MSG_WRITE = 3;	
	public static final int MSG_READ = 4;

	private String cmdHb; 

	String targetIp;
	int port;

	Socket socket;
	InputStream input;
	OutputStream output;		

	Handler contextHandler;	

	HeartBeatThread hbThrd;
	WriteThread wThrd;
	ReadThread rThrd;	

	public SimpleSocketClient(String ip, int port, Handler handler){
		targetIp = ip;
		this.port = port;
		contextHandler = handler;			
	}

					

	public void write(String data){
		byte[] bytes = data.getBytes();
		wThrd.write(bytes);
	}

	public void onDisconnected(){
		//Auto connect 
		connect();		
	}
		
	public void setCmdHb(String val){
		cmdHb = val;
	}

	
	public void connect(){
		new Thread(new Runnable(){
			@Override
			public void run(){
				try{						
					if(socket!=null)
						disconnect();
					socket = new Socket();														
					socket.setKeepAlive(true);
					socket.setSoTimeout(200);// 设置超时时间
					socket.connect(new  InetSocketAddress(targetIp, port),200);
					
					try {
						input = socket.getInputStream();
						output = socket.getOutputStream();
						hbThrd = new HeartBeatThread(output);
						wThrd = new WriteThread(output);
						rThrd = new ReadThread(input);
						wThrd.start();
						rThrd.start();
						hbThrd.start();
						contextHandler.sendEmptyMessage(SimpleSocketClient.MSG_CONNECTED);
					}catch (IOException e) {
						e.printStackTrace();				
					}							
				}catch(UnknownHostException e){
					e.printStackTrace();					
					contextHandler.sendEmptyMessage(MSG_DISCONNECTED);
				}catch(IOException e){
					e.printStackTrace();
					contextHandler.sendEmptyMessage(MSG_DISCONNECTED);					
				}																		
			}
		}).start();		
	}

	public void disconnect(){
		new Thread(new Runnable(){
			@Override
			public void run(){
				try{
					if(hbThrd!=null)
						hbThrd.doStop();
					if(wThrd!=null)
						wThrd.doStop();
					if(rThrd!=null)
						rThrd.doStop();				
					socket.close();		
					socket=null;
					contextHandler.sendEmptyMessage(MSG_DISCONNECTED);
				}catch(IOException e){
					e.printStackTrace();
				}	
			}
		}).start();			
	}

	private class HeartBeatThread extends Thread{

		private boolean isRunning;
		private int rate;
		private OutputStream output;		
		public HeartBeatThread(OutputStream output){
			isRunning = true;
			rate = 2000;
			this.output = output;
		}

		@Override
		public void run(){
			while(isRunning){
				try{
					try{
						output.write(cmdHb.getBytes());
					}catch(IOException e){
						e.printStackTrace();
						onDisconnected();
					}					
					Thread.sleep(rate);
				}catch(InterruptedException e){
					e.printStackTrace();
				}
			}
		}

		public void doStop(){
			isRunning = false;
		}
	}

	private class ReadThread extends Thread{
		private boolean isRunning;
		InputStream input;
		byte[] bytes;
		public ReadThread(InputStream input){
			isRunning = true;
			this.input = input;
			bytes = new byte[1024];
		}
		@Override
		public void run(){
			while(isRunning){
				try{
					int len = input.read(bytes);
					if(len>0){
						contextHandler.obtainMessage(SimpleSocketClient.MSG_READ, len, -1, bytes).sendToTarget();
					}
				}catch(IOException e){
					e.printStackTrace();
					//onDisconnected();
				}
			}
		}

		public void doStop(){
			isRunning = false;
		}
	}

	private class WriteThread extends Thread{

		private boolean isRunning;
		private boolean isWrite;
		private byte[] dataBytes;		
		private OutputStream output;
		public WriteThread(OutputStream output){
			isRunning = true;
			isWrite = false;
			this.output = output;
		}

		@Override
		public void run(){
			while(isRunning){
				if(isWrite){
					synchronized(dataBytes){
						isWrite = false;
						try{						
							output.write(dataBytes);
						}catch (IOException e) {
							e.printStackTrace();
							onDisconnected();
						}
					}
				}
			}
		}

		public void doStop(){
			isRunning = false;
		}

		public void write(byte[] bytes){
			dataBytes = bytes;
			isWrite = true;
		}
	}

}

