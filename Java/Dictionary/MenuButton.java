import javax.swing.JButton;
import java.awt.*;
import javax.swing.*;

public class MenuButton extends JButton{
		
		private boolean focus = false;
		private int id;
		

		public MenuButton(){
			super();
		}

		
		public MenuButton(String name,int id){
			super(name);
			this.setBorder(null);
			this.setBackground(Color.lightGray);
			this.setForeground(Color.darkGray);
			this.setFocusPainted(false);
			this.setPreferredSize(new Dimension(120,22));
			this.setFont(new Font("Arial", Font.PLAIN, 16));
			this.id = id;
			
		}


		public MenuButton(String name){
			super(name);
			this.setBorder(null);
			this.setBackground(Color.darkGray);
			this.setForeground(Color.lightGray);
			this.setFocusPainted(false);
			this.setPreferredSize(new Dimension(120,22));
			this.setFont(new Font("Arial", Font.PLAIN, 16));
		}
		
        public int get_id(){
        	return this.id;
        }
		
		public boolean focused(){
			return focus;
		}
		
		public void setFocus(boolean state){
			focus = state;
		}
		
		
	}
