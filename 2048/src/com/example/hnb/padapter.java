package com.example.hnb;

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;

public class padapter extends ArrayAdapter<Integer>{
	private Context context;
	private ArrayList<Integer> arr;
	
	public padapter(Context context, int resource, List<Integer> objects) {
		super(context, resource, objects);
		this.context=context;
		this.arr=new ArrayList<>(objects);
		// TODO Auto-generated constructor stub
	}
	@Override
	public void notifyDataSetChanged(){
		arr = ForPuzzle.getPuzzle().getArr();
		super.notifyDataSetChanged();
	}
	@Override
	public View getView(int position, View convertView, ViewGroup parent){
		if (convertView==null){
			LayoutInflater inflater = (LayoutInflater)context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
			convertView=inflater.inflate(R.layout.itm_p, null);
		}
		if (arr.size()>0){
			pitem pItem=(pitem)convertView.findViewById(R.id.titem);
			pItem.setTett(arr.get(position));
		}
		return convertView;
	}
}
