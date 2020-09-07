import javax.swing.*;
import java.awt.*;
import javax.swing.ImageIcon;
import java.io.*;

public class Chart_button extends JButton{
        
        boolean focus=false;
        
        
        public Chart_button(String name){
            super(name);
            this.setBackground(null);
            this.setBorder(BorderFactory.createLineBorder(new Color(39,83,93)));
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

        


        public boolean focused(){
            return focus;
        }
        
        public void setFocus(boolean state){
            focus = state;
        }
        

    }