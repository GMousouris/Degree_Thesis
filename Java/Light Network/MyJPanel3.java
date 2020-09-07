import javax.swing.JPanel;
import java.awt.*;
import java.awt.geom.AffineTransform;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Path2D;
import java.awt.geom.Point2D;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.lang.Math;
import java.awt.BasicStroke;
import java.lang.Object.*;
import java.util.Random;





public class MyJPanel3 extends JPanel{

    Point[]  points_a; Point[] points_a1;
    Point[]  points_b; Point[] points_b1;
    boolean state = false;




    public MyJPanel3() {
        points_a = new Point[4];
        points_b = new Point[4];

        points_a1 = new Point[4];
        points_b1 = new Point[4];
        reset();
        state = true;

    }

    /*Drawing each line*/
    protected void paintComponent(Graphics gg){
        super.paintComponent(gg);
        Graphics2D g2 = (Graphics2D)gg;

        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_ON);
        Toolkit.getDefaultToolkit().sync();

        if(state){
            g2.setColor(Color.white);
            for(int i=0; i<points_a.length; i++){

                if(i<points_a.length-1){
                    g2.drawLine((int)points_a[i].getX(),(int)points_a[i].getY(),(int)points_a[i+1].getX(),(int)points_a[i+1].getY());
                    g2.drawLine((int)points_b[i].getX(),(int)points_b[i].getY(),(int)points_b[i+1].getX(),(int)points_b[i+1].getY());

                    ////
                    g2.drawLine((int)points_a1[i].getX(),(int)points_a1[i].getY(),(int)points_a1[i+1].getX(),(int)points_a1[i+1].getY());
                    g2.drawLine((int)points_b1[i].getX(),(int)points_b1[i].getY(),(int)points_b1[i+1].getX(),(int)points_b1[i+1].getY());
                }
                else{
                    g2.drawLine((int)points_a[0].getX(),(int)points_a[0].getY(),(int)points_a[3].getX(),(int)points_a[3].getY());
                    g2.drawLine((int)points_b[0].getX(),(int)points_b[0].getY(),(int)points_b[3].getX(),(int)points_b[3].getY());

                    ///
                    g2.drawLine((int)points_a1[0].getX(),(int)points_a1[0].getY(),(int)points_a1[3].getX(),(int)points_a1[3].getY());
                    g2.drawLine((int)points_b1[0].getX(),(int)points_b1[0].getY(),(int)points_b1[3].getX(),(int)points_b1[3].getY());
                }


                g2.drawLine((int)points_a[i].getX(),(int)points_a[i].getY(),(int)points_b[i].getX(),(int)points_b[i].getY());

                g2.drawLine((int)points_a[0].getX(),(int)points_a[0].getY(),(int)points_b[0].getX(),(int)points_b[0].getY());
                g2.drawLine((int)points_a[1].getX(),(int)points_a[1].getY(),(int)points_b[1].getX(),(int)points_b[1].getY());

                ////
                g2.drawLine((int)points_a1[i].getX(),(int)points_a1[i].getY(),(int)points_b1[i].getX(),(int)points_b1[i].getY());

                g2.drawLine((int)points_a1[0].getX(),(int)points_a1[0].getY(),(int)points_b1[0].getX(),(int)points_b1[0].getY());
                g2.drawLine((int)points_a1[1].getX(),(int)points_a1[1].getY(),(int)points_b1[1].getX(),(int)points_b1[1].getY());

                //g2.setColor(Color.white);

                /*
                g2.drawLine((int)points_a.get(0).getX(),(int)points_a.get(0).getY(),(int)points_a.get(2).getX(),(int)points_a.get(2).getY());
                g2.drawLine((int)points_b.get(0).getX(),(int)points_b.get(0).getY(),(int)points_b.get(2).getX(),(int)points_b.get(2).getY());

                g2.drawLine((int)points_a.get(1).getX(),(int)points_a.get(1).getY(),(int)points_a.get(3).getX(),(int)points_a.get(3).getY());
                g2.drawLine((int)points_b.get(1).getX(),(int)points_b.get(1).getY(),(int)points_b.get(3).getX(),(int)points_b.get(3).getY());
                */

            }
        }


    }

    public void reset(){
        //points_a.clear();
        //points_b.clear();
        state = false;
        points_a[0] = (new Point(20,50)); points_a[1] = (new Point(20+60,50));
        points_a[2] = (new Point(20+60,50+60)); points_a[3] = (new Point(20,50+60));

        points_b[0] = new Point(20+20,50+20); points_b[1] = new Point(20+60-20,50+20);
        points_b[2] = new Point(20+60-20,50+60-20); points_b[3] = new Point(20+20,50+60-20);


        points_a1[0] = (new Point(20,50+120)); points_a1[1] = (new Point(20+60,50+120));
        points_a1[2] = (new Point(20+60,50+60+120)); points_a1[3] = (new Point(20,50+60+120));

        points_b1[0] = (new Point(20+25,50+120+25)); points_b1[1] = (new Point(20+60+25,50+120+25));
        points_b1[2] = (new Point(25+20+60,50+120+25+60)); points_b1[3] = (new Point(25+20,50+120+25+60));
    }


}