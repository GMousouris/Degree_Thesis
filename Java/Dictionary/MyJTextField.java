import javax.swing.JTextField;
import java.awt.*;
import javax.swing.*;

public class MyJTextField extends JTextField{

    	private boolean focus = false;
    	private int number = -1;
    	private String label="";

    	public MyJTextField(){
    		super();
    	}

    	public MyJTextField(String txt){
    		super(txt);
    	}

    	public MyJTextField(String txt,int length){
    		super(txt,length);
    	}

    	public void setFocus(boolean bool){
    		this.focus = bool;
    	}

    	public boolean isFocused(){
    		return this.focus;
    	}

    	public int getNumber(){
    		return this.number;
    	}

    	public void setNumber(int a){
    		this.number = a;
    	}

    	public String getLabel(){
    		return this.label;
    	}

    	public void setLabel(String label){
    		this.label = label;
    	}

    }