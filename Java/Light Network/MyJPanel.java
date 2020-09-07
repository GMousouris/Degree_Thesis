import javax.swing.JPanel;
import java.awt.*;
import java.awt.geom.AffineTransform;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Path2D;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.lang.Math;
import java.awt.BasicStroke;
import java.lang.Object.*;



/*
Base class : <MyJPanel>
    /*
    Holding data {
         -ArrayList<Line> lines : holds the line data whih paints everytime it get's updated
    }
*/
public class MyJPanel extends JPanel{

    ArrayList<Line> lines;
    int state = 0;
    boolean dragging = false;
    boolean dashed = true;
    boolean circles = true;
    Light light;
    int[] rect;



    public MyJPanel() {
        lines = new ArrayList<Line>();
        rect = new int[4];

    }

    /*Drawing each line*/
    protected void paintComponent(Graphics g){
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D)g;

        Toolkit.getDefaultToolkit().sync();

        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_ON);


        if(dragging){

            g.setColor(new Color(10,100,240,30));
            g.fillRect(rect[0],rect[1],rect[2],rect[3]);
            g.setColor(new Color(17,17,22));
            g.drawRect(rect[0],rect[1],rect[2],rect[3]);
        }
        g.setColor(Color.black);


        for(int i=0; i<lines.size(); i++){
            if(!lines.get(i).isConnected()){continue;}

            if(dashed){
                g2.setStroke(new BasicStroke(1.0f, BasicStroke.CAP_BUTT,
                        BasicStroke.JOIN_MITER, 10.0f, new float[]{6.0f}, lines.get(i).dash_p)); //5.0f
            }
            else{
                g2.setStroke(new BasicStroke(1.0f));
            }

            g.setColor(lines.get(i).color);
            if(lines.get(i).get_p()!=2) {
                g.drawLine(lines.get(i).cords[0], lines.get(i).cords[1], lines.get(i).cords[2], lines.get(i).cords[3]);
                g2.setStroke(new BasicStroke(1.0f));
            }
            else{
                g2.setStroke(new BasicStroke(1.0f, BasicStroke.CAP_BUTT,
                        BasicStroke.JOIN_MITER, 10.0f, new float[]{9.0f}, lines.get(i).dash_p)); //9.0f
                g.drawLine(lines.get(i).cords[0], lines.get(i).cords[1], lines.get(i).cords[2], lines.get(i).cords[3]);
                g2.setStroke(new BasicStroke(1.0f));
            }
            if(lines.get(i).get_p()==1 && circles) {
                //g2.setColor(Color.gray);

                g2.setStroke(new BasicStroke(1.0f, BasicStroke.CAP_BUTT,
                        BasicStroke.JOIN_MITER, 10.0f, new float[]{1.50f}, lines.get(i).dash_p)); //5.0f
                Ellipse2D.Double oval = new Ellipse2D.Double(lines.get(i).cords[0] - 40 - 18 - 45, lines.get(i).cords[1] - 50 - 40, lines.get(i).ell_xy, lines.get(i).ell_xy);

                //Path2D.Double path = new Path2D.Double();
                //path.append(oval,false);
                //AffineTransform trans = new AffineTransform();

                //trans.rotate(lines.get(i).rotation,lines.get(i).cords[0]-40-18-45+200/2,lines.get(i).cords[1]-50-40+200/2);
                //path.transform(trans);

                if (lines.get(i).get_State() && lines.get(i).isPowered()) {
                    g2.setColor(new Color(g2.getColor().getRed() / 2, g2.getColor().getGreen() / 2, g2.getColor().getBlue() / 2, 60));
                    g2.fillOval(lines.get(i).cords[0] - 40 - 18 - 45, lines.get(i).cords[1] - 50 - 40, lines.get(i).ell_xy, lines.get(i).ell_xy);
                }

                g2.setColor(lines.get(i).color);
                g2.draw(oval);
                //g2.draw(path);
            }

                g2.setStroke(new BasicStroke(1.0f));


        }

        //state = 0;
    }



    /*Method that adds a Line to the internal lines ArrayList*/
    public void add_Line(Line line){
        lines.add(line);
    }

    /*Method that clears internal ArrayList lines*/
    public void clear_lines(){
        lines.clear();
    }

    Line get_Line(int index){
        if(index<lines.size()){
            return lines.get(index);
        }
        return null;
    }

    int Size(){
        return this.lines.size();
    }

    public void set_State(int a){
        state = a;
    }

    public void set_Light(Light light){
        this.light = light;
    }

    public void set_Rect(int x1,int y1,int x2,int y2){
        rect[0] = x1; rect[1] = y1; rect[2] = x2; rect[3] = y2;
    }

    public void swap_dashed(){
        dashed = !dashed;
    }

    public void swap_circle_effect(){
        circles = !circles;
    }


}