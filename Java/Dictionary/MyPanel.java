import javax.swing.JPanel;
import javax.imageio.ImageIO;
import java.io.*;
import java.awt.image.BufferedImage;
import java.awt.*;
import javax.swing.*;

public class MyPanel extends JPanel{

		BufferedImage img = null;

		public MyPanel(String image){
			try{
				img = ImageIO.read(new File(image));
			}
			catch(IOException e){}
			
		}
		public MyPanel(){

		}

		public void paintComponent(Graphics g) {
           super.paintComponent(g);
           if(img!=null){
           	g.drawImage(img,0,0,this);
           }
           
           
           
        }
        public void setImage(String image){
          	if(image==null){
          		this.img=null;
          	}
          	else{
          	try{
	    			this.img = ImageIO.read(new File(image));
	    		}
	    		catch(IOException e){}
	    		
                }
        } 
	}