import javax.swing.JPanel;
import java.awt.*;
import java.util.ArrayList;
import java.lang.Math;

/*
Base class : <MyJPanel>
    /*
    Holding data {
         -ArrayList<Line> lines : holds the line data whih paints everytime it get's updated
    }
*/
public class MyJPanel extends JPanel{

    private float dash_p = 5.0f;
    private ArrayList<Line> lines;
    private int state = 0;
    //Light light;

    public MyJPanel(){
        lines = new ArrayList<Line>();
    }

    /*Drawing each line*/
    protected void paintComponent(Graphics g){
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D)g;


        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_ON);
        //g.setColor(Color.black);

        for(int i=0; i<this.lines.size(); i++){

            if(lines.get(i).stroke){
                g2.setStroke(new BasicStroke(1.0f, BasicStroke.CAP_BUTT,
                        BasicStroke.JOIN_MITER, 10.0f, new float[]{6.0f}, lines.get(i).d_phase));
            }
            else{
                g2.setStroke(new BasicStroke(1.0f));
            }

            g2.setColor(lines.get(i).color);
            g2.drawLine(lines.get(i).cords[0],lines.get(i).cords[1],lines.get(i).cords[2],lines.get(i).cords[3]);

            g2.setStroke(new BasicStroke(1.0f));
        }

        //state = 0;
    }



    /*Method that adds a Line to the internal lines ArrayList*/
    public void add_Line(Line line){
        this.lines.add(line);
    }

    public void setLines(ArrayList<Line> lines){
        this.lines = new ArrayList<Line>(lines);
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

    public void movePhase(){
        for(Line l : lines){
            if(l.d<0){
                l.d_phase+= -2.0f;
            }
            else{
                l.d_phase+= 2.0f;
            }

            if(l.d_phase<=2.0f){
                l.d_phase = 100.0f;
            }

        }
    }






}