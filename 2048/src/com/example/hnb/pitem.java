package com.example.hnb;

import android.annotation.TargetApi;
import android.content.Context;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.os.Build;
import android.util.AttributeSet;
import android.widget.TextView;

@TargetApi(Build.VERSION_CODES.JELLY_BEAN)
public class pitem extends TextView{

	public pitem(Context context) {
		super(context);
		// TODO Auto-generated constructor stub
	}
	public pitem(Context context, AttributeSet attrs) {
		super(context, attrs);
		// TODO Auto-generated constructor stub
	}

	public pitem(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		// TODO Auto-generated constructor stub
	}
	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec){
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		int n = getMeasuredWidth();
		setMeasuredDimension(n, n);
	}
	public void setTett(int n){
		if (n < 100){
			setTextSize(40);
			
		}
		else if (n < 1000){
			setTextSize(35);
		}
		else {
			setTextSize(30);
		}
		if (n >= 8){
			setTextColor(Color.WHITE);
		}
		else {
			setTextColor(Color.BLACK);
		}
		GradientDrawable drawable=(GradientDrawable)this.getBackground();
		drawable.setColor(ForPuzzle.getPuzzle().collor(n));
		setBackground(drawable);
		
		if (n == 0){
			setText(" ");
		}
		else {
			setText("" + n);
			
		}
	}
}
