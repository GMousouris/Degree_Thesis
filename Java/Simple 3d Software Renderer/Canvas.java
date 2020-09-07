import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.geom.Line2D;
import java.awt.geom.Point2D;
import java.security.Key;
import java.util.ArrayList;
import java.util.Random;


public class Canvas extends JPanel implements KeyListener,ComponentListener,MouseListener,MouseMotionListener {

    private JFrame frame;

    private int x0 = 800; private int starting_x = 0;
    private int y0 = 800; private int starting_y = 0;

    private boolean pressed = false;
    private boolean resizing = false;
    private boolean frame_pressed = false;

    private gShape shape;
    private ArrayList<Integer> indices;
    private ArrayList<Vec3> vertices;
    private ArrayList<Boolean> flags;

    private final double PI = 3.14159265359;
    private final double dtheta = 3.14159265359;
    double dt = 1.0/60.0;

    private double theta_x = 0.0;
    private double theta_y = 0.0;
    private double theta_z = 0.0;
    private double offset_z = 2.5;

    private Screen_Transformer screen_trans;
    private Mat3 rot_matrix;

    private int state = 0;
    private String rendering_mode = "Line_rendering";

    int faces_counter = 0;
    boolean backface = false;
    boolean outline = false;
    boolean polygon_outline = false;

    ArrayList<Point3D> points;
    Color[] colors;

    public Canvas(JFrame frm){
        /*JPanel*/
        super();

        Toolkit.getDefaultToolkit().setDynamicLayout(false);

        this.setPreferredSize(new Dimension(frm.getWidth(),frm.getHeight()));
        this.setBackground(Color.black);
        this.addKeyListener(this);
        this.addMouseListener(this);
        this.addMouseMotionListener(this);
        this.setFocusable(true);
        /* */
        setGraphics(frm);
        /* */

        /*JFrame*/
        frame = frm;
        frame.addComponentListener(this);
        frame.addMouseListener(this);
        frame.add(this);
        frame.pack();
        frame.setDefaultCloseOperation(frame.EXIT_ON_CLOSE);
        frame.setVisible(true);
        /* */
    }
    /*Setting the Graphic objects*/
    public void setGraphics(JFrame frm){

        indices = new ArrayList<Integer>();
        vertices = new ArrayList<Vec3>();
        flags = new ArrayList<Boolean>();

        points = new ArrayList<Point3D>();

        colors = new Color[]{Color.white,Color.red,Color.yellow,Color.blue,Color.green,Color.gray};

        shape = new Cube(1.0);
        screen_trans = new Screen_Transformer(frm);
    }
    /*------------------------------------------------------- */



    /*---------------------------------------DRAW_TRIANGLE------------------------------------------------*/
    /*Drawing Triangles Method*/
    public void DrawTriangle(Vec3 v0,Vec3 v1,Vec3 v2,Color c) {


        Vec3 pv0 = new Vec3(v0.x,v0.y,v0.z);
        Vec3 pv1 = new Vec3(v1.x,v1.y,v1.z);
        Vec3 pv2 = new Vec3(v2.x,v2.y,v2.z);

        //Sorting vertices by 'y'
        if (pv1.y < pv0.y) swap(pv0,pv1);
        if (pv2.y < pv1.y) swap(pv1,pv2);
        if (pv1.y < pv0.y) swap(pv0,pv1);
        //

        if(pv0.y == pv1.y){ // flat top triangle

            //sorting top vertices by 'x'
            if(pv1.x < pv0.x) swap(pv0,pv1);
            drawFlatTopTriangle(pv0,pv1,pv2,c);
        }
        else if(pv1.y == pv2.y){ //flat bottom triangle

            //sorting bottom vertices by 'x'
            if(pv2.x < pv1.x) swap(pv1,pv2);
            drawFlatBottomTriangle(pv0,pv1,pv2,c);
        }
        else{ // Regular triangle

            //find splitting vertex
            double alpha_split = (pv1.y - pv0.y)/(pv2.y - pv0.y);
            //Vec3 vi = pv0.add((pv2.sub(pv0)).mult(alpha_split));

            Vec3 vi = new Vec3(pv2);
            vi.x-=pv0.x; vi.y-=pv0.y; vi.z-=pv0.z;
            vi.x*=alpha_split; vi.y*=alpha_split; vi.z*=alpha_split;
            vi.x+=pv0.x; vi.y+=pv0.y; vi.z+=pv0.z;


            /*
            Vec3 vi = pv2.sub(pv0);
            vi = vi.mult(alpha_split);
            vi = vi.add(pv0);
            */

            if(pv1.x < vi.x){ // major right
                drawFlatBottomTriangle(pv0,pv1,vi,c);
                drawFlatTopTriangle(pv1,vi,pv2,c);
            }
            else{ // major left
                drawFlatBottomTriangle(pv0,vi,pv1,c);
                drawFlatTopTriangle(vi,pv1,pv2,c);
            }
        }
    }

    @SuppressWarnings("Duplicates")
    public void drawFlatTopTriangle(Vec3 v0,Vec3 v1,Vec3 v2 , Color c){

        //calculate slops in screen space
        double m0 = (v2.x - v0.x) / (v2.y - v0.y);
        double m1 = (v2.x - v1.x) / (v2.y - v1.y);

        //calculate -start- and -end- scanlines
        int y_start = (int) Math.ceil(v0.y - 0.5);
        int y_end = (int) Math.ceil(v2.y - 0.5);


        for(int y = y_start; y<y_end; y++){

            //calculate start and end points (x_cords)
            // add 0.5 to 'y' value because we are calculating based on pixel centers
            double px0 = m0 * ((double)(y) + 0.5 - v0.y) + v0.x;
            double px1 = m1 * ((double)(y) + 0.5 - v1.y) + v1.x;

            //calculate start and end pixels
            int x_start = (int)Math.ceil(px0 - 0.5);
            int x_end = (int)Math.ceil(px1 - 0.5);

            for(int x = x_start; x<x_end; x++){
                points.add(new Point3D(x,y,c));
            }
        }
    }

    @SuppressWarnings("Duplicates")
    public void drawFlatBottomTriangle(Vec3 v0, Vec3 v1, Vec3 v2, Color c){

        //calculate slops in screen space
        double m0 = (v1.x - v0.x) / (v1.y - v0.y);
        double m1 = (v2.x - v0.x) / (v2.y - v0.y);

        //calculate -start- and -end- scanlines
        int y_start = (int) Math.ceil(v0.y - 0.5);
        int y_end = (int) Math.ceil(v2.y - 0.5);

        for(int y = y_start; y<y_end; y++){

            //calculate start and end points (x_cords)
            // add 0.5 to 'y' value because we are calculating based on pixel centers
            double px0 = m0 * ((double)(y) + 0.5 - v0.y) + v0.x;
            double px1 = m1 * ((double)(y) + 0.5 - v0.y) + v0.x;

            //calculate start and end pixels
            int x_start = (int)Math.ceil(px0 - 0.5);
            int x_end = (int)Math.ceil(px1 - 0.5);

            for(int x = x_start; x<x_end; x++){
                points.add(new Point3D(x,y,c));
                //System.out.println(x+","+y);

            }
        }
    }

    /*---------------------------------------Compose_Frame_Methods------------------------------------------------*/

    /*Hard copying gObject's IndexedLinesList data*/
    public void initialise_data(){

        indices.clear();
        vertices.clear();
        flags.clear();

        if(state == 0){ //Line rendering

            IndexedLinesList linelist = shape.get_Lines();
            /*Hard copying Indices data*/
            for(int i=0; i<linelist.indices.size(); i++){
                indices.add(linelist.indices.get(i));
            }

            /*Hard copying vertices data*/
            for(int i=0; i<linelist.vertices.size(); i++){
                Vec3 temp = (linelist.vertices.get(i));
                vertices.add(new Vec3(temp.x,temp.y,temp.z));
            }
        }
        if(state == 1){ //Polygon rendering


            IndexedTriangleList triangleList = shape.get_Triangles();
            /*Hard copying indices data*/
            for(int i=0; i<triangleList.indices.size(); i++){
                indices.add(triangleList.indices.get(i));
            }

            /*Hard copying vertices data*/
            for(int i=0; i<triangleList.vertices.size(); i++){
                Vec3 temp = (triangleList.vertices.get(i));
                vertices.add(new Vec3(temp.x,temp.y,temp.z));
            }

            /*hard copying flags*/
            for(int i=0; i<triangleList.flags.size(); i++){
                this.flags.add(triangleList.flags.get(i));
            }
        }

    }

    /*Applying Rotations on ~XYZ axis at the gObject*/
    public void rotate_obj(){

        rot_matrix = new Mat3();
        /*clamping theta between -π ~ π */
        clamp_thetas();
        /*Rotating from x ~ y ~ z axis*/
        rot_matrix = Mat3.rotateX(theta_x);
        rot_matrix = rot_matrix.matrix_mult(Mat3.rotateY(theta_y));
        rot_matrix = rot_matrix.matrix_mult(Mat3.rotateZ(theta_z));
    }

    /*From xyz to ~ Screen XY Transformation*/
    public void screen_transform(){

        /*
        #1
        Apply rotation and
        transform to world_coordinates
         */
        for(int i=0; i<this.vertices.size(); i++){
            this.vertices.set(i,Mat3.vector_mult(this.vertices.get(i),rot_matrix)); // Applying rotation transformation for every vec3 vertex.
            this.vertices.set(i,this.vertices.get(i).add(new Vec3(0.0,0.0,offset_z))); // Doing something.
        }

        /*
        #2
        Back-face culling
        on world_space coordinates
        */
        if(backface && state == 1){
            for(int i=0; i<this.indices.size()/3; i++){
                Vec3 v0  = new Vec3(this.vertices.get(this.indices.get(i*3)));
                Vec3 v1  = new Vec3(this.vertices.get(this.indices.get(i*3 + 1)));
                Vec3 v2  = new Vec3(this.vertices.get(this.indices.get(i*3 + 2)));

                /*taking the face's normal*/
                Vec3 temp = new Vec3(v1);
                temp = temp.sub(v0);

                Vec3 temp2 = new Vec3(v2);
                temp2.sub(v0);

                Vec3 temp3 = new Vec3(temp.cross_prodcut(temp2));
                /*    */
                if(temp3.dot_mult(v0)>0.0){
                    this.flags.set(i,true);
                }
                else{
                    this.flags.set(i,false);
                }

                //shape.get_Triangles().flags.set(i,(v1.sub(v0)).cross_prodcut(v2.sub(v0)).dot_mult(v0)>0.0);
            }
        }

        /*
        #3
        Transforming word_space coordinates
        to screen coordinates
         */
        for(int i=0; i<this.vertices.size(); i++){
            this.vertices.set(i,screen_trans.transform(this.vertices.get(i))); // Transforming every vertex to screen co-ordinates.
        }


    }


    /*--------------------------------------*/
    /*Composing Frame*/
    public void compose_frame(){

        initialise_data();
        rotate_obj();/*Apply Rotation transforms*/
        screen_transform();/*Screen XY transformation*/
    }
    /*---------------------------------------------------------------------------------------------------*/


    /*paint method*/
    protected void paintComponent(Graphics g){
        super.paintComponent(g);

        Graphics2D g2d = (Graphics2D) g;
        g2d.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
                RenderingHints.VALUE_ANTIALIAS_ON);
        g2d.setColor(Color.white);

        faces_counter = 0;

        /*Composing_frame*/
        compose_frame();


        /*Choosing between states {line/triangle rendering}*/
        if(state == 0){ /*Drawing the lines*/

            for(int i=0; i<this.indices.size(); i = i+2){

                Vec3 v1 = this.vertices.get(indices.get(i));
                Vec3 v2 = this.vertices.get(indices.get(i+1));

                Shape line = new Line2D.Double(v1.x,v1.y,v2.x,v2.y);
                g2d.draw(line);

            }

        }//
        else if(state == 1){ /*Drawing Polygons ~ Triangles*/

            for(int i=0; i<this.indices.size()/3; i++){
                points.clear();

                if(backface){
                    if(this.flags.get(i)){
                        continue;
                    }
                }
                faces_counter++;

                /*Drawing only the outline of the faces*/
                if(outline){
                    int indexx[] = new int[]{shape.faces.get(i/2).a,shape.faces.get(i/2).b,shape.faces.get(i/2).c,shape.faces.get(i/2).d};
                    for(int q =0; q<4; q++){
                        int index = indexx[q];
                        int index2 = indexx[0];
                        if(q!=3){
                            index2 = indexx[q+1];
                        }
                        Shape line = new Line2D.Double(vertices.get(index).x,vertices.get(index).y,vertices.get(index2).x,vertices.get(index2).y);
                        g2d.draw(line);
                    }
                }
                /*Drawing the polygons*/
                else{

                    /*Drawing the outline of the polygons*/
                    if(polygon_outline){
                        Shape line1 = new Line2D.Double(vertices.get(indices.get(i*3)).x,vertices.get(indices.get(i*3)).y,vertices.get(indices.get(i*3+1)).x,vertices.get(indices.get(i*3+1)).y);
                        Shape line2 = new Line2D.Double(vertices.get(indices.get(i*3+1)).x,vertices.get(indices.get(i*3+1)).y,vertices.get(indices.get(i*3+2)).x,vertices.get(indices.get(i*3+2)).y);
                        Shape line3 = new Line2D.Double(vertices.get(indices.get(i*3+2)).x,vertices.get(indices.get(i*3+2)).y,vertices.get(indices.get(i*3)).x,vertices.get(indices.get(i*3)).y);
                        g2d.draw(line1); g2d.draw(line2); g2d.draw(line3);
                    }
                    /*Filling the polygons with Color*/
                    else{
                        DrawTriangle((vertices.get(indices.get(i*3))),(vertices.get(indices.get(i*3+1))),(vertices.get(indices.get(i*3+2))),Color.black);
                        for(int j=0; j<points.size(); j++){
                            g2d.setColor(colors[i/2]);
                            g2d.drawLine(points.get(j).x,points.get(j).y,points.get(j).x,points.get(j).y);
                            g2d.setColor(Color.white);
                        }
                    }

                }
                /* */

            }

        }
        /*panel_info*/
        drawInfo(g2d);
        g2d.setColor(Color.white);

    }



    /*---------------------------------------------------------------------*/
    /*Listener*/
    @Override
    public void keyPressed(KeyEvent e) {

        if(e.getKeyCode() == KeyEvent.VK_UP){ //rotation speed - up
            dt+=0.25/60.0;
            repaint();
        }
        else if(e.getKeyCode()== KeyEvent.VK_DOWN){ //rotation speed - down
            dt-=0.25/60.0;
            repaint();
        }
        else if(e.getKeyCode()== KeyEvent.VK_SHIFT){ // zoom - in
            offset_z-=0.1;
            repaint();
        }
        else if(e.getKeyCode()== KeyEvent.VK_CONTROL){ // zoom - out
            offset_z+=0.1;
            repaint();
        }

        else if(e.getKeyCode()==KeyEvent.VK_0 || e.getKeyCode()== KeyEvent.VK_NUMPAD0 ){

        }
        else if(e.getKeyCode()==KeyEvent.VK_1 || e.getKeyCode()== KeyEvent.VK_NUMPAD1 ){ //rendering_mode
            if(state == 0){
                state = 1;
                rendering_mode = "Polygon_rendering";
            }
            else if(state == 1){
                state = 0;
                rendering_mode = "Line_rendering";
            }
            repaint();
        }
        else if(e.getKeyCode()==KeyEvent.VK_2 || e.getKeyCode()== KeyEvent.VK_NUMPAD2 ){ //backface
            backface = !backface;
            repaint();
        }
        else if(e.getKeyCode()==KeyEvent.VK_3 || e.getKeyCode()== KeyEvent.VK_NUMPAD3 ) { //outline
            outline = !outline;
            repaint();
        }
        else if(e.getKeyCode()==KeyEvent.VK_4 || e.getKeyCode()== KeyEvent.VK_NUMPAD4 ) { //outline
            polygon_outline = !polygon_outline;
            repaint();
        }

        else if(e.getKeyCode()== KeyEvent.VK_ENTER){ //reset_states
            reset();
        }
        /* */

        /*Rotation Keys*/
        if(e.getKeyCode()==KeyEvent.VK_Q){
            theta_x+= dtheta * dt;
            repaint();
        }
        if(e.getKeyCode()==KeyEvent.VK_W){
            theta_y+= dtheta * dt;
            repaint();
        }
        if(e.getKeyCode()==KeyEvent.VK_E){
            theta_z+= dtheta * dt;
            repaint();
        }
        if(e.getKeyCode()==KeyEvent.VK_A){
            theta_x-= dtheta * dt;
            repaint();
        }
        if(e.getKeyCode()==KeyEvent.VK_S){
            theta_y-= dtheta * dt;
            repaint();
        }
        if(e.getKeyCode()==KeyEvent.VK_D){
            theta_z-= dtheta * dt;
            repaint();
        }
        /*  */

    }
    @Override
    public void keyTyped(KeyEvent e) {

    }
    @Override
    public void keyReleased(KeyEvent e) {

    }
    /*----------------------------------------------------------------*/

    /*--------panel_info--------*/
    public void drawInfo(Graphics2D g2d){

        g2d.drawString("theta_x : "+theta_x,10,15);
        g2d.drawString("theta_y : "+theta_y,10,30);
        g2d.drawString("theta_z : "+theta_z,10,45);
        g2d.drawString("offset_z : "+offset_z,10,60);
        g2d.drawString("rotation speed : "+dt*dtheta,10,75);

        g2d.setColor(new Color(145,145,145));
        g2d.drawString("               _CONTROLS_",frame.getWidth()-200,15);
        g2d.drawString("Q/A : +/-                              theta_x",frame.getWidth()-200,30);
        g2d.drawString("W/S : +/-                             theta_y",frame.getWidth()-200,45);
        g2d.drawString("E/D : +/-                              theta_z",frame.getWidth()-200,60);
        g2d.drawString("SHIFT/CTRL : +/-             z_offset",frame.getWidth()-200,75);
        g2d.drawString("K_UP/K_DOWN : +/-  rot_speed",frame.getWidth()-200,90);
        g2d.drawString("K_1 : on/off polygon render mode",frame.getWidth()-200,105);
        g2d.drawString("K_2 : on/off         backface culling",frame.getWidth()-200,120);
        g2d.drawString("K_3 : on/off              outline mode",frame.getWidth()-200,135);

        g2d.setColor(Color.white);

        g2d.setColor(Color.green);
        g2d.drawString("rendering mode : "+rendering_mode,10,frame.getHeight()-15-30-15);
        if(state == 1){
            g2d.setColor(Color.yellow);
            g2d.drawString("Polygons : "+shape.get_Triangles().indices.size()/3,10,frame.getHeight()-15-30);
            g2d.setColor(Color.white);

            g2d.setColor(Color.green);
            g2d.drawString("Polygon Outline mode : ",frame.getWidth()-190,frame.getHeight()-30-30-30);
            if(polygon_outline){g2d.setColor(Color.white);}else{g2d.setColor(Color.red);}
            g2d.drawString(String.valueOf(polygon_outline),frame.getWidth()-190+131,frame.getHeight()-30-30-30);


            g2d.setColor(Color.green);
            g2d.drawString("Outline mode : ",frame.getWidth()-190,frame.getHeight()-30-30-15);
            if(outline){
                g2d.setColor(Color.white);
            }else{g2d.setColor(Color.red);}
            g2d.drawString(String.valueOf(outline),frame.getWidth()-190+87,frame.getHeight()-30-30-15);
            g2d.setColor(Color.green);

            g2d.drawString("Backface mode : ",frame.getWidth()-190,frame.getHeight()-30-30);
            g2d.setColor(Color.white);
            if(backface){
                g2d.setColor(Color.white);
            }else{
                g2d.setColor(Color.red);
            }
            g2d.drawString(String.valueOf(backface),frame.getWidth()-190+95,frame.getHeight()-30-30);
            g2d.setColor(Color.white);

            g2d.setColor(Color.yellow);
            int counter = faces_counter;
            if(outline)counter = faces_counter/2;
            g2d.drawString("Polygons drawn : "+counter,frame.getWidth()-190,frame.getHeight()-30-15);
            g2d.setColor(Color.white);
        }

        g2d.setColor(Color.white);

    }


    /*Clamping theta_values from -π to +π*/
    public void clamp_thetas(){

        double thetas[] = new double[]{theta_x,theta_y,theta_z};

        for(int i=0; i<3; i++){
            double modded = thetas[i] % (2.0 * PI);
            if(modded>PI){
                thetas[i] = (modded - (2.0*PI));
            }
            else{
                thetas[i] = modded;
            }
        }
    }
    /* */
    public void swap(Vec3 a,Vec3 b){
        Vec3 temp = new Vec3(a);
        a.Clone(b);
        b.Clone(temp);
    }
    /* */

    public void reset(){

        dt = 1.0/60.0;
        state = 0;
        rendering_mode = "Line_rendering";
        offset_z = 2.5;
        theta_x = 0.0;
        theta_y = 0.0;
        theta_z = 0.0;
        repaint();
    }
    /*----------------------------------------------------------------*/

    @Override
    public void componentResized(ComponentEvent e) {

        if (e.getSource().equals(frame)) {
            int dx = frame.getWidth() - x0;
            int dy = frame.getHeight() - y0;

            if(dx!=0 && dy==0){
                frame.setSize(x0+dx,x0+dx);
            }
            else if(dy!=0 && dx==0){
                frame.setSize(y0+dy,y0+dy);
            }
            else if(dx!=0 && dy!=0){
                if(Math.abs(dx)>Math.abs(dy)){
                    frame.setSize(x0+dx,x0+dx);
                }
                else{
                    frame.setSize(dy+y0,dy+y0);
                }
            }

            x0 = frame.getWidth();
            y0 = frame.getHeight();

            screen_trans.update();
            //repaint();
        }
    }

    @Override
    public void componentMoved(ComponentEvent e) {

    }

    @Override
    public void componentShown(ComponentEvent e) {

    }

    @Override
    public void componentHidden(ComponentEvent e) {

    }


    /*-----------------------------------------------------------------*/

    @Override
    public void mouseClicked(MouseEvent e) {

    }

    @Override
    public void mousePressed(MouseEvent e) {
        if(e.getSource().equals(this)){
            starting_x = e.getX(); starting_y = e.getY();
            pressed = true;
        }
        else{
            pressed = false;
        }
    }

    @Override
    public void mouseReleased(MouseEvent e) {

        pressed = false;
    }

    @Override
    public void mouseEntered(MouseEvent e) {

    }

    @Override
    public void mouseExited(MouseEvent e) {

    }



    @Override
    public void mouseDragged(MouseEvent e) {

        int dx = e.getX() - starting_x;
        int dy = e.getY() - starting_y;
        double dth = dt*dtheta/5;
        if(state==1 && !outline && !polygon_outline){
            dth = dth*5;
        }
        //System.out.println("dx : "+dx+"\ndy : "+dy);
        if(dy!=0){
            if(dy <0){

                theta_x-=dth;
            }
            else if(dy>0){
                theta_x+=dth;
            }

        }
        if(dx!=0){
            if(dx<0){
                theta_y-=dth;
            }
            else{
                theta_y+=dth;
            }
        }
        repaint();

        starting_x = e.getX();
        starting_y = e.getY();
    }
    @Override
    public void mouseMoved(MouseEvent e) {

    }
    /*-----------------------------------------------------------------*/


}
