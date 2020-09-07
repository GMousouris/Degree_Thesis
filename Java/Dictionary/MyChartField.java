import javax.swing.JTextField;
import java.awt.*;
import javax.swing.*;
import javax.swing.ImageIcon;
import java.io.*;


public class MyChartField extends JTextField{

    	private boolean focus = false;
        private int x;

    	public MyChartField(){
            
    		super();
            this.x  =0;
    	}

    	public MyChartField(String txt){
            
    		super(txt);
            this.x  = 0;
    	}

    	public MyChartField(String txt,int length){
    		super(txt,length);
            this.x = 0;
    	}

    	public void setFocus(boolean bool){
    		this.focus = bool;
    	}

    	public boolean isFocused(){
    		return this.focus;
    	}
        public void setX(int x){
            this.x = x;
        }
        public int getX(){
            return this.x;
        }


    }