package com.example.hnb;

import java.util.ArrayList;
import java.util.Random;

import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.util.Log;

public class ForPuzzle {
	private static ForPuzzle forpuzzle;
	private ArrayList<Integer> arr = new ArrayList<>();
	private int[][] p = new int[4][4];
	private int[] getcols;
	private Random r = new Random();
	
	static {
		forpuzzle= new ForPuzzle();
	}
	
	public static ForPuzzle getPuzzle(){
		return forpuzzle;
	}
	public void initt(Context context){
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				p[i][j]=0;
				arr.add(0);
			}
		}
		TypedArray ta = context.getResources().obtainTypedArray(R.array.getcolorblock);
		getcols = new int[ta.length()];
		for (int i = 0; i < ta.length(); i++){
			getcols[i] = ta.getColor(i, 0);
		}
		ta.recycle();
		setRandom();
		clearP();
	}
	public ArrayList<Integer> getArr(){
		return arr;
	}
	public int collor(int n){
		if (n ==0) {
			return Color.WHITE;
		}
		else {
			int a = (int) (Math.log(n)/Math.log(2));
			return getcols[a-1];
		}
	}
	public void setRandom(){
		int n = 0;
		int tmp;
		for (int i = 0; i < 16; i++){
			if (arr.get(i)==0){
				n++;
			}
		}
		if (n > 1){
			tmp = r.nextInt(2) + 1;
		}
		else {
				if (n==1){
					tmp = 1;
				}
				else tmp = 0;
			}
		while (tmp != 0) {
			int ro = r.nextInt(4);
			int co = r.nextInt(4);
			if (p[ro][co] == 0){
				p[ro][co] = 2;
				tmp--;
			}
		}
	}
	public void clearP(){
		arr.clear();
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				arr.add(p[i][j]);
			}
		}
	}
	public void mergeLeft(){
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				int tmp1 = p[i][j];
				if (tmp1 != 0){
					for (int k = j + 1; k < 4; k++) {
						int tmp2 = p[i][k];
						if (tmp2 != 0){
							if (tmp1 == tmp2){
								p[i][j] = tmp1 * 2;
								p[i][k] = 0;
							}
							break ;
						}
					}
				}
			}
		}
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				int tmp1 = p[i][j];
				if (tmp1 == 0){
					for (int k = j + 1; k < 4; k++) {
						int tmp2 = p[i][k];
						if (tmp2 != 0){
							p[i][j] = p[i][k];
							p[i][k] = 0;
							break ;
						}
					}
				}
			}
		}
		setRandom();
		clearP();
	}
	public void mergeRight(){
		for (int i = 3; i >= 0; i--){
			for (int j = 3; j >= 0; j--){
				int tmp1 = p[i][j];
				if (tmp1 != 0){
					for (int k = j - 1; k >= 0; k--) {
						int tmp2 = p[i][k];
						if (tmp2 != 0){
							if (tmp1 == tmp2){
								p[i][j] = tmp1 * 2;
								p[i][k] = 0;
							}
							break ;
						}
					}
				}
			}
		}
		for (int i = 3; i >= 0; i--){
			for (int j = 3; j >= 0; j--){
				int tmp1 = p[i][j];
				if (tmp1 == 0){
					for (int k = j - 1; k >= 0; k--) {
						int tmp2 = p[i][k];
						if (tmp2 != 0){
							p[i][j] = p[i][k];
							p[i][k] = 0;
							break ;
						}
					}
				}
			}
		}
		setRandom();
		clearP();
	}
	public void mergeUp(){
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				int tmp1 = p[j][i];
				if (tmp1 != 0){
					for (int k = j + 1; k < 4; k++) {
						int tmp2 = p[k][i];
						if (tmp2 != 0){
							if (tmp1 == tmp2){
								p[j][i] = tmp1 * 2;
								p[k][i] = 0;
							}
							break ;
						}
					}
				}
			}
		}
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				int tmp1 = p[j][i];
				if (tmp1 == 0){
					for (int k = j + 1; k < 4; k++) {
						int tmp2 = p[k][i];
						if (tmp2 != 0){
							p[j][i] = p[k][i];
							p[k][i] = 0;
							break ;
						}
					}
				}
			}
		}
		setRandom();
		clearP();
	}
	public void mergeDown(){
		for (int i = 3; i >= 0; i--){
			for (int j = 3; j >= 0; j--){
				int tmp1 = p[j][i];
				if (tmp1 != 0){
					for (int k = j - 1; k >= 0; k--) {
						int tmp2 = p[k][i];
						if (tmp2 != 0){
							if (tmp1 == tmp2){
								p[j][i] = tmp1 * 2;
								p[k][i] = 0;
							}
							break ;
						}
					}
				}
			}
		}
		for (int i = 3; i >= 0; i--){
			for (int j = 3; j >= 0; j--){
				int tmp1 = p[j][i];
				if (tmp1 == 0){
					for (int k = j - 1; k >= 0; k--) {
						int tmp2 = p[k][i];
						if (tmp2 != 0){
							p[j][i] = p[k][i];
							p[k][i] = 0;
							break ;
						}
					}
				}
			}
		}
		setRandom();
		clearP();
	}
	public int getMax(){
		int n = 0;
		for (int i = 0; i < 4; i++) {
			for (int j = 0 ; j< 4; j++) {
				if (n < p[i][j])
					n = p[i][j];
			}
		}
		return n;
	}
	public int getPoint(){
		int n = 0;
		for (int i = 0; i < 4; i++) {
			for (int j = 0 ; j< 4; j++) {
					n += p[i][j];
			}
		}
		return n;
	}
	public boolean isEnd(){
		boolean ret = false;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				if (p[i][j] == 0)
					return ret;
			}
		}
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 3; j++) {
				if (p[i][j] == p[i][j + 1])
					return ret;
			}
		}
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 3; j++) {
				if (p[j][i] == p[j + 1][i])
					return ret;
			}
		}
		Log.d("tag", "hello");
		ret = true;
		return ret;
	}
}
