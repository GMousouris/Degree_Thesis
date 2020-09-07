import javax.swing.*;
import java.awt.*;
import javax.swing.ImageIcon;
import java.io.*;

public class MyCheckBox2 extends JButton{
        
        boolean focus=false;
        ImageIcon img0,img1,img2,img00,img11,img22;
        boolean expanded = false;
        
        public MyCheckBox2(){
            super();
            this.setBackground(null);
            super.setContentAreaFilled(false);
            this.setBorder(null);
            this.setBorderPainted(false);
            //this.setBorder(BorderFactory.createLineBorder(new Color(39,83,93)));
        
            try{img0 = new ImageIcon("Icons\\expand0.png");}catch(Exception e){}
            try{img1 = new ImageIcon("Icons\\expand1.png");}catch(Exception e){}
            try{img2 = new ImageIcon("Icons\\expand2.png");}catch(Exception e){}
            try{img00 = new ImageIcon("Icons\\expand00.png");}catch(Exception e){}
            try{img11 = new ImageIcon("Icons\\expand11.png");}catch(Exception e){}
            try{img22 = new ImageIcon("Icons\\expand22.png");}catch(Exception e){}

        }



        protected void paintComponent(Graphics g) {
            if (getModel().isPressed()) {
                if(expanded){
                    this.setIcon(img22);
                }
                else{
                    this.setIcon(img2);
                }
                
            } else if (getModel().isRollover()) {
                if(!expanded){
                    this.setIcon(img0);
                }
                else{
                    this.setIcon(img00);
                }
                
            } else {
                if(!expanded){
                     this.setIcon(img1);
                }
                else{
                    this.setIcon(img11);
                }
               
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
        public void set_expanded(boolean a){
            this.expanded = a;
        }
        public boolean isExpanded(){
            return this.expanded;
        }

    }