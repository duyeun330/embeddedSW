package com.example.hnb;

import android.app.Activity;
import android.content.Intent;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageButton;

public class MainActivity extends Activity {
	
	MediaPlayer mp;
	ImageButton ibtn;
	Button play, exit;
	boolean mplay = true;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        ibtn = (ImageButton)findViewById(R.id.imageButton1);
        mp = MediaPlayer.create(this, R.raw.hello);
        mp.start();
        mp.setLooping(true);
        OnClickListener mplisten = new OnClickListener(){
        	public void onClick(View v){
        		if (mplay){
        			mplay = false;
        			mp.pause();
        		}
        		else {
        			mplay = true;
        			mp.start();
        			mp.setLooping(true);
        		}
        	}
        };
        ibtn.setOnClickListener(mplisten);
        
        play = (Button)findViewById(R.id.button1);
        exit = (Button)findViewById(R.id.button2);
        
        OnClickListener playlisten = new OnClickListener(){
        	public void onClick(View v){
        		Intent intent=new Intent(MainActivity.this, PlayGame.class);
        		startActivity(intent);
        	}
        };
        play.setOnClickListener(playlisten);
        
        OnClickListener exitlisten = new OnClickListener(){
        	public void onClick(View v){
        		mp.release();
        		finish();
        	}
        };
        exit.setOnClickListener(exitlisten);
	}

	

}
