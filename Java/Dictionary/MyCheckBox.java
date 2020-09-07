import javax.swing.*;
import java.awt.*;
import javax.swing.ImageIcon;
import java.io.*;

public class MyCheckBox extends JButton{
        
        boolean focus=false;
        ImageIcon img0,img1;
        
        public MyCheckBox(){
            super();
            this.setBackground(null);
            this.setBorder(BorderFactory.createLineBorder(new Color(39,83,93)));

            try{img0 = new ImageIcon("Icons\\x0.png");}catch(Exception e){}
            try{img1 = new ImageIcon("Icons\\x1.png");}catch(Exception e){}

        }

        protected void paintComponent(Graphics g) {
            if (getModel().isPressed()) {
                this.setIcon(img1);
            } else if (getModel().isRollover()) {
                this.setIcon(img0);
            } else {
                this.setIcon(null);
            }
            
            super.paintComponent(g);
        }



        public boolean focused(){
            return focus;
        }
        
        public void setFocus(boolean state){
            focus = state;
        }
        public ImageIcon getImg(){
            return this.img0;
        }
        public ImageIcon getImg2(){
            return this.img1;
        }

    }