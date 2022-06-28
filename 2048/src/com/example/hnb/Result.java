package com.example.hnb;

import android.app.Activity;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class Result extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_result);
		
		int maxValue = getIntent().getIntExtra("Max Point", 0);
		int maxPoint = getIntent().getIntExtra("Point", 0);
		
		TextView tv1, tv2;
		Button bt;
		
		tv1 = (TextView)findViewById(R.id.textView2);
		tv2 = (TextView)findViewById(R.id.textView3);
		
		tv1.setText("MAX VALUE : " + Integer.toString(maxValue));
		tv2.setText("MAX POINT : " + Integer.toString(maxPoint));
		
		bt = (Button)findViewById(R.id.button1);
		OnClickListener bt1 = new OnClickListener(){
			public void onClick(View v){
				moveTaskToBack(true);
				finish();
				System.exit(0);
			}
		};
		bt.setOnClickListener(bt1);
	}
}
