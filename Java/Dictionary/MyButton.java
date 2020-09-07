import javax.swing.JButton;
import java.awt.*;
import javax.swing.*;

public class MyButton extends JButton{
		
		private boolean focus = false;
		private int id;
		

		public MyButton(){
			super();
		}

		
		public MyButton(String name,int id){
			super(name);
			this.setBorder(BorderFactory.createLineBorder(new Color(22,21,21)));
			this.setBackground(new Color(108,223,193));
			this.setForeground(Color.white);
			this.setFocusPainted(false);
			this.setPreferredSize(new Dimension(120,22));
			this.setFont(new Font("Arial", Font.PLAIN, 16));
			this.id = id;
		}


		public MyButton(String name){
			super(name);
			super.setContentAreaFilled(false);
			this.setBorder(BorderFactory.createRaisedBevelBorder());
			this.setBackground(new Color(73,183,154));
			this.setForeground(new Color(226,226,226));
			this.setFocusPainted(false);
			this.setPreferredSize(new Dimension(120,22));
			this.setFont(new Font("Arial", Font.PLAIN, 16));
		}

		protected void paintComponent(Graphics g) {
            if (getModel().isPressed()) {
                g.setColor(new Color(65,159,134));
                setForeground(Color.white);
            }
            else if(getModel().isRollover()){
            	g.setColor(new Color(82,228,189));
            	setForeground(Color.white);
            }
            else{
            	g.setColor(new Color(73,183,154));
            	setForeground(new Color(226,226,226));
            }
            g.fillRect(0, 0, getWidth(), getHeight());
            super.paintComponent(g);
        }
        public void setContentAreaFilled(boolean b) {
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