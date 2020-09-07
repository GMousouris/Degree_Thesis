
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




/*
Base class : <MyJPanel>
    /*
    Holding data {
         -ArrayList<Line> lines : holds the line data whih paints everytime it get's updated
    }
*/
public class MyJPanel2 extends JPanel{

    int[] points_x;
    int[] points_y;

    ArrayList<Point2D> max_points;
    ArrayList<Point2D> min_points;

    int mid = 0;
    int length = 0;
    int x1 = 0; int y1 =0;
    int x2 =0;  int y2 = 0;
    int y = 0; int x = 0;
    int max = -1; int min = 1000;
    boolean state = false;
    boolean clear_state = true;



    public MyJPanel2(int width) {
        points_x = new int[2000];
        points_y = new int[2000];

        max_points = new ArrayList<Point2D>();
        min_points = new ArrayList<Point2D>();

    }

    /*Drawing each line*/
    protected void paintComponent(Graphics gg){
        super.paintComponent(gg);
        Graphics2D g2 = (Graphics2D)gg;

        Random r = new Random();

        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_ON);

        g2.setColor(Color.green);

        Toolkit.getDefaultToolkit().sync();


        if(!clear_state){

            /*Rect Background Filling*/
            g2.setColor(new Color(145,145,125,75)); //125,125,125,70
            g2.fillRect(1,1,x,getHeight()-2);
            /* */

            /*Rect Background Right*/
            g2.setColor(new Color(32,32,32));
            g2.fillRect(getWidth()-59-1-3-200+45,1,64-2+200-45,getHeight()-2);
            g2.setColor(new Color(0,255,120,100));
            g2.drawLine(getWidth()-59-1-3-200+45,1,getWidth()-59-1-3-200+45,getHeight()-2);
            /* */

            /*Mid line*/
            g2.setColor(new Color(90,70,70)); //darkgray
            g2.setStroke(new BasicStroke(1.0f, BasicStroke.CAP_BUTT,
                    BasicStroke.JOIN_MITER, 10.0f, new float[]{6.0f}, 1.0f)); //5.0f
            g2.drawLine(0,mid,getWidth()-59-1-3-200+45,mid);
            /* */

            /*Function*/
            g2.setStroke(new BasicStroke(1.4f));
            g2.setColor(new Color(0,255,120,140));
            g2.drawPolyline(points_x,points_y,length);

            /*Red hoping line*/
            g2.setStroke(new BasicStroke(1.0f));
            g2.setColor(new Color(250,10,5,99));
            g2.drawLine(0,5,x,y); //0,mid,x,y

            /*Max - Mins*/
            if(state){
                //max//
                for(int i=0; i<max_points.size(); i++){

                    g2.setColor(new Color(220,220,220,100));
                    g2.setStroke(new BasicStroke(1.0f));
                    g2.drawLine((int)max_points.get(i).getX(),(int)max_points.get(i).getY(),(int)max_points.get(i).getX(),mid);

                    g2.setColor(Color.white);
                    g2.setStroke(new BasicStroke(2.0f));
                    g2.drawLine((int)max_points.get(i).getX(),(int)max_points.get(i).getY(),(int)max_points.get(i).getX(),(int)max_points.get(i).getY());
                }
                //min//
                for(int i=0; i<min_points.size(); i++){

                    g2.setColor(new Color(125,40,10));
                    g2.setStroke(new BasicStroke(1.0f));
                    g2.drawLine((int)min_points.get(i).getX(),(int)min_points.get(i).getY(),(int)min_points.get(i).getX(),mid);

                    g2.setColor(new Color(125,40,10));
                    g2.setStroke(new BasicStroke(3.0f));
                    g2.drawLine((int)min_points.get(i).getX(),(int)min_points.get(i).getY(),(int)min_points.get(i).getX(),(int)min_points.get(i).getY());
                }
            }
            /* */
            g2.setStroke(new BasicStroke(1.0f));

            /*Labels*/
            g2.setColor(Color.gray);
            if(!state){
                mid = -8;
                max = -10;
                g2.drawString("  Min : "+0,getWidth()-59,35+20);
            }
            else{
                g2.drawString("  Min : "+(min*20-10),getWidth()-59,35+20);
            }
            g2.drawString("  Max : "+(max*10+100),getWidth()-59,35-20);
            g2.drawString("- Mid : "+(mid*10 + 10*10 - 20),getWidth()-59,35);

            /*Symbols*/
            g2.setStroke(new BasicStroke(2.0f));

            g2.setColor(Color.lightGray);
            g2.fillRect(getWidth()-59-1,35-20-5-1,4,4);
            g2.setColor(new Color(125,40,10));
            g2.fillRect(getWidth()-59-1,35+20-5-1,4,4);
            /* */


        }
        else if(clear_state){
            g2.dispose();
            gg.dispose();
        }

    }

    public void reset(){
        min = 1000; max = -1;
        mid = 0;
        length = 0;
        max_points.clear(); min_points.clear();
    }

    public void clear(){
        clear_state = true;

    }

    public void add(int x,int y){
        points_x[length] = x;
        points_y[length] = getHeight() - y;
        if(y>this.max){
            this.max = y;
            max_points.add(new Point2D.Double(x,getHeight()-y));
        }
        if(y<this.min){
            this.min = y;
            min_points.add(new Point2D.Double(x,getHeight()-y));
        }
        length++;
        mid+=y;

        mid =  mid/length + 31;
        this.y = getHeight()-y;
        this.x = x;

    }

    public void build_function(){

    }

    public void set_func_length(int s){
        length = s;
    }

    public int get_func_length(){
        return this.length;
    }






}