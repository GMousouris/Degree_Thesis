import javax.swing.JButton;
import java.awt.*;
import javax.swing.*;

public class MyButton2 extends JButton{
		
		private boolean focus = false;
		private int id;
		private word wordd;
		

		public MyButton2(){
			super();
		}

		public MyButton2(String name,word wordd){
			super(name);
			//this.setBackground(new Color(188,176,155));
			//this.setForeground(Color.darkGray);
			this.setBorder(null);
			this.setBackground(Color.white);
			this.setForeground(Color.darkGray);
			this.setFocusPainted(false);
		    //this.setBorderPainted(false);
		    this.wordd = wordd;
			this.setPreferredSize(new Dimension(200,22));
			this.setFont(new Font("Arial", Font.PLAIN, 16));
			
			
			//this.setFont(new Font(this.getFont().getName(),Font.BOLD,this.getFont().getSize()));
			//this.setPreferredSize(new Dimension(200,50));
		}
		
        public int get_id(){
        	return this.id;
        }

        public word get_word(){
        	return this.wordd;
        }
		
		public boolean isFocused(){
			return focus;
		}
		
		public void setFocus(boolean state){
			focus = state;
		}
		
		
	}