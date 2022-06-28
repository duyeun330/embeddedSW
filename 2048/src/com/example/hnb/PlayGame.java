package com.example.hnb;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.GridView;
import android.widget.TextView;

public class PlayGame extends Activity{
	
	public native int movXY();
	public native void printFND(int score);
	public native void printDOT(int pos);
	
	private GridView gridv;
	private padapter adapter;
	private Button bt;
	private TextView maxValue, pointValue;
	BackThread mThread;
	
	Handler mHandler=new Handler(){
		public void handleMessage(Message msg){
			int k;
			if(msg.what==0){
				if (msg.arg1 == 1){
	        		ForPuzzle.getPuzzle().mergeUp();
	        		adapter.notifyDataSetChanged();
	        		printDOT(msg.arg1);
				}
				else if (msg.arg1 == 2){
					ForPuzzle.getPuzzle().mergeLeft();
					adapter.notifyDataSetChanged();
	        		printDOT(msg.arg1);
				}
				else if (msg.arg1 == 3){
					ForPuzzle.getPuzzle().mergeRight();
					adapter.notifyDataSetChanged();
	        		printDOT(msg.arg1);
	        	}
				else if (msg.arg1 == 4){
					ForPuzzle.getPuzzle().mergeDown();
					adapter.notifyDataSetChanged();
	        		printDOT(msg.arg1);
				}

        		k = ForPuzzle.getPuzzle().getMax();
        		maxValue.setText("" + Integer.toString(k));
        		k = ForPuzzle.getPuzzle().getPoint();
        		pointValue.setText("" + Integer.toString(k));
        		printFND(k);
        		if (ForPuzzle.getPuzzle().isEnd()){
        			System.out.println("nono");
        			setResult();
        		}
			}
		}

	};
	
	class BackThread extends Thread{
		int mBackValue=0;
		Handler sHandler;
		
		BackThread(Handler handler){
			sHandler=handler;
		}
		public void run(){
			while(!isInterrupted()){
				mBackValue = movXY();
				Message msg=Message.obtain();
				msg.what = 0;
				msg.arg1 = mBackValue;
				sHandler.sendMessage(msg);
				try{Thread.sleep(400);}
				catch(InterruptedException e){
					Thread.currentThread().interrupt();}
			}
		}
	}
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.puzzle);
        
        System.loadLibrary("backmodule");
        setgrid();
        setDatas();
        setD();
        
        
        mThread=new BackThread(mHandler);
		mThread.setDaemon(true);
		mThread.start();
		
        bt = (Button)findViewById(R.id.button2);
        OnClickListener bt1 = new OnClickListener(){
        	public void onClick(View v){
        		setResult();
        	}
        };
        bt.setOnClickListener(bt1);
        
        maxValue = (TextView)findViewById(R.id.maxtext);
        pointValue = (TextView)findViewById(R.id.point);
        

		int k = ForPuzzle.getPuzzle().getMax();
		maxValue.setText("" + Integer.toString(k));
		k = ForPuzzle.getPuzzle().getPoint();
		pointValue.setText("" + Integer.toString(k));
	}
	protected void setResult() {
		printFND(0);
		printDOT(5);
		Intent intent = new Intent(PlayGame.this, Result.class);
		intent.putExtra("Max Point",  ForPuzzle.getPuzzle().getMax());
		intent.putExtra("Point",  ForPuzzle.getPuzzle().getPoint());
		mThread.interrupt();
		startActivity(intent);
	}
	private void setgrid(){
		gridv = (GridView)findViewById(R.id.gridp);
		
	}
	private void setDatas(){
		ForPuzzle.getPuzzle().initt(PlayGame.this);
		adapter= new padapter(PlayGame.this,0, ForPuzzle.getPuzzle().getArr());
	}
	private void setD(){
		gridv.setAdapter(adapter);
	}
}
