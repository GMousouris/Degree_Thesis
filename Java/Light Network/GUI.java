import javax.swing.JFrame;
import java.awt.event.ActionListener;
import java.awt.event.*;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JTextField;
import javax.swing.JTextArea;
import javax.swing.JScrollPane;
import javax.swing.JDialog;

import java.util.ArrayList;
import java.awt.Insets;
import java.awt.Dimension;
import java.awt.Color;
import javax.swing.BorderFactory;
import javax.swing.SwingUtilities;
import javax.swing.JPopupMenu;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JMenuBar;
import javax.swing.JMenu;
import javax.swing.UIManager.*;
import javax.swing.*;
import java.io.*;
import javax.swing.event.MouseInputListener;
import javax.swing.Timer;
import java.util.Random;






public class GUI extends JFrame implements ActionListener, WindowListener,MouseInputListener,ComponentListener {

    /*Variables*/
    JButton main_switch, power ,extend_panel;//Main Buttons
    JMenuBar main_menu; //Main MenuBar
    JMenu menu, menu2,menu3; //MenuBar's menus
    Light_Info power_info;//JPanel holding main_switch's info
    JMenuItem remove_light2, cut_power, cut_power2, edit, edit2, save_state, new_network, add_light,add_Light2, remove_light,reset_positions,reset_states,dashed_lines,circle_lines,paint,switch_states;//JMenu items appearing when 'right_clicking' at a <Light> component

    MyJPanel light_panel;//light_panel holding the <Light> components
    MyJPanel2 bottom_panel;
    MyJPanel3 ext_panel;
    JScrollPane light_panel_scroll;//JScrollPane holding the light_panel

    ArrayList<Light> light_system;//ArrayList holding the <Light> data
    ArrayList<Line> lines;//ArrayList holding the <Line> data

    boolean switch_state = false; //data holding main_switch's state
    light_choose_Frame choose_window;//frame appearing when 'adding new light'
    int pointer = -1;
    int counter = 0;
    /*- - - - - - - - - - - - - - - - - -*/
    Insets frame_insets;


    boolean right_click = false;
    boolean pressed = false;
    boolean dragged = false;
    Light pressed_light = null;
    int x = 0; int x1 = -1; int y1 = -1;
    int y = 0;

    Timer timer;
    MyThread thread;
    MyThread2 t;
    MyThread3 th3;
    int index = 0;
    private boolean panel_pressed = false;
    private Light selected_pressed;

    private boolean selection = false;
    private boolean right_click_selection = false;

    private int scroll_y = 0;
    private int bot_y = 0;
    private boolean state_paint = true;
    private boolean extend_state = false;


    /*Constructor()*/
    public GUI() {

        super("Application");
        setSize(440 + 160 + 125+50, 400 + 25+50);
		/*light_system ArrayList will hold the <Light> components*/
        light_system = new ArrayList<Light>();

		/*lines ArrayList will holed the <Line> data for each <Light> component*/
        lines = new ArrayList<Line>();



		/*Setting GUI*/
        setGui();
		/* */

		/*Setting the Frame*/
        this.addComponentListener(this);
        this.setMinimumSize(new Dimension(323,135));
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLocationRelativeTo(null);
        //pack();
        setVisible(true);

    }
	/*- - - - - - - - - - - - - - - - - -*/

    /*Setting GUI*/
    public void setGui() {


        scroll_y = -70; //100
        bot_y = +10; //0

        this.addWindowListener(this);
        this.addMouseListener(this);
        this.setLayout(null);
        this.getContentPane().setBackground(new Color(32, 32, 32));

        frame_insets = this.getInsets();

		/*Main menu bar*/
        main_menu = new JMenuBar();
        main_menu.setBackground(Color.gray);
        main_menu.setBorder(null);

        menu = new JMenu("  File  ");
        menu.setOpaque(true);
        menu.setForeground(Color.white);
        menu.setBackground(Color.darkGray);

        save_state = new JMenuItem("Save current state");
        save_state.addActionListener(this);
        new_network = new JMenuItem("Create new network");
        new_network.addActionListener(this);

        menu.add(new_network);
        menu.addSeparator();
        menu.add(save_state);

        menu2 = new JMenu("  Light  ");
        menu2.setOpaque(true);
        menu2.setForeground(Color.white);
        menu2.setBackground(Color.darkGray);

        add_light = new JMenuItem("Add Light");
        add_light.addActionListener(this);

        remove_light = new JMenuItem("Remove Light");
        remove_light.addActionListener(this);

        edit2 = new JMenuItem("Edit Light");
        edit2.addActionListener(this);

        cut_power2 = new JMenuItem("Connect/Disconnect");
        cut_power2.addActionListener(this);

        reset_positions = new JMenuItem("Reset ALL Light positions to default");
        reset_positions.addActionListener(this);

        reset_states = new JMenuItem("Reset ALL Light states to default");
        reset_states.addActionListener(this);

        switch_states = new JMenuItem("Switch ALL Light states");
        switch_states.addActionListener(this);

        menu2.add(add_light);
        menu2.addSeparator();
        menu2.add(edit2);
        menu2.addSeparator();
        menu2.add(cut_power2);
        menu2.addSeparator();
        menu2.add(remove_light);
        menu2.addSeparator();
        menu2.add(switch_states);
        menu2.addSeparator();
        menu2.add(reset_positions);
        menu2.addSeparator();
        menu2.add(reset_states);


        menu3 = new JMenu("  Options  ");
        menu3.setOpaque(true);
        menu3.setForeground(Color.white);
        menu3.setBackground(Color.darkGray);

        dashed_lines = new JMenuItem("Enable/Disable Dashed Lines");
        dashed_lines.addActionListener(this);

        circle_lines = new JMenuItem("Enable/Disable Circle Light effect");
        circle_lines.addActionListener(this);

        paint = new JMenuItem("Enable/Disable Graph");
        paint.addActionListener(this);

        menu3.add(dashed_lines);
        menu3.addSeparator();
        menu3.add(circle_lines);
        menu3.addSeparator();
        menu3.add(paint);

        main_menu.add(menu);
        main_menu.add(menu2);
        main_menu.add(menu3);


        this.setJMenuBar(main_menu);
		/* */

		/*Main Switch*/
        main_switch = new JButton("Main Switch");
        main_switch.addActionListener(this);
        //main_switch.setBorder(BorderFactory.createBevelBorder(0));
        main_switch.setBackground(Color.darkGray);
        main_switch.setForeground(Color.lightGray);
        main_switch.setFocusPainted(false);
        main_switch.setBounds(frame_insets.left + 10, frame_insets.top + 69 - 40, 110, 30);
        main_switch.addMouseListener(this);
        this.add(main_switch);
		/* */

		/*Button showing main_switch's state*/
        power = new JButton();
        power.setBackground(Color.black);
        power.setFocusPainted(false);
        power.setBounds((this.getWidth()-35) / 2 - 25, frame_insets.top + 60 - 40, 40, 40);
        power.setBorder(BorderFactory.createLineBorder(Color.red, 2));
        this.add(power);

        extend_panel = new JButton();
        extend_panel.addActionListener(this);
        extend_panel.addMouseListener(this);
        extend_panel.setFocusPainted(false);
        extend_panel.setBounds(frame_insets.left  + this.getWidth()-70+10+2 , frame_insets.top + 10 , 35,35);
        this.add(extend_panel);


		/*Panel showing for main_switch's state*/
        power_info = new Light_Info("Main Switch", " -");
        power_info.setBounds((this.getWidth()-35) / 2 - 25 + 42, frame_insets.top + 51 - 40, 150, 50);
        this.add(power_info);

		/* */
        ext_panel = new MyJPanel3();
        ext_panel.setBackground(new Color(5,8,9));
        ext_panel.setBounds(frame_insets.left + getWidth()-150-150+50+6+2,frame_insets.top+40+40-20+2,220+8-10-2,this.getHeight()-150-60);
        this.add(ext_panel,0);
        ext_panel.setVisible(false);

		/*light_panel , main panel which will contain the light components etc*/
        light_panel = new MyJPanel();
        light_panel.setLayout(null);
        light_panel.addMouseMotionListener(this);
        light_panel.addMouseListener(this);



        light_panel.setSize(400 + 160 + 130, 200 + 100);
        light_panel.setBackground(new Color(25,22,22));

		/*light_panel_scroll , which will contain the light_panel*/
        light_panel_scroll = new JScrollPane(light_panel, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        light_panel_scroll.getVerticalScrollBar().setUnitIncrement(16);
        light_panel_scroll.setBorder(BorderFactory.createLineBorder(Color.black));

        light_panel_scroll.setBounds(frame_insets.left + 10, frame_insets.top + 100 - 40+1, this.getWidth()-35, this.getHeight()-125+ scroll_y);
        light_panel_scroll.setPreferredSize(new Dimension(this.getWidth()-35, this.getHeight()-125+ scroll_y));
        this.add(light_panel_scroll);


		/* */

		bottom_panel = new MyJPanel2(this.getWidth()-35);
		//bottom_panel.setBackground(new Color(14,14,14,60));
		bottom_panel.setBackground(new Color(10,5,5));
        bottom_panel.setBounds(frame_insets.left + 10, frame_insets.top + 100 - 40+1 + light_panel_scroll.getHeight()+4, this.getWidth()-35, this.getHeight()- light_panel_scroll.getHeight() - 125- bot_y);
        bottom_panel.setPreferredSize(new Dimension(this.getWidth()-35, this.getHeight()- light_panel_scroll.getHeight() - 125 - bot_y));
        bottom_panel.setLayout(null);
        this.add(bottom_panel);
        bottom_panel.build_function();


		/*Remove_light2 JMenuItem appearing when a right-click occurs at a light component*/
        remove_light2 = new JMenuItem("Remove");
        remove_light2.addActionListener(this);
        remove_light2.setOpaque(true);
        remove_light2.setBackground(Color.lightGray);
		/* */

		/*cut_power JMenuItem appearing when a right-click occurs at a light component*/
        cut_power = new JMenuItem("Connect/Disconnect");
        cut_power.addActionListener(this);
        cut_power.setOpaque(true);
        cut_power.setBackground(Color.lightGray);
		/* */

		/*Edit JMenuItem appearing when a right-click occurs at a light component*/
        edit = new JMenuItem("Edit");
        edit.addActionListener(this);
        edit.setOpaque(true);
        edit.setBackground(Color.lightGray);
		/* */
        add_Light2 = new JMenuItem("Add Light");
        add_Light2.addActionListener(this);
        add_Light2.setOpaque(true);
        add_Light2.setBackground(Color.lightGray);
		/* */

		//this.getContentPane().setComponentZOrder(ext_panel,0);


		this.revalidate();
		this.repaint();


        thread = new MyThread();
        thread.start();

    }
    /*- - - - - - - - - - - - THREADS- - - - - ------------------------------------------------------------------*/
    class MyThread2 extends Thread{

        Light light;
        Light_Info info;
        int counter = 15;
        int z = 0;
        int p = -1;
        int q =0;

        public MyThread2(Light light){
            this.light = light;
            this.info = light.get_info_panel();
        }


        public void run(){

            while(!this.light.isPressed() && !this.light.is_Selected()){

                light_panel.repaint();

                try{
                    this.sleep(40);
                }
                catch(Exception e){}

                if(this.light.get_State()){
                    this.light.setBackground(this.light.get_Color());
                }
                else{
                    this.light.setBackground(new Color(counter,counter,counter));
                }

                this.light.get_info_panel().setBackground(new Color(64+counter,64+counter,64+counter));
                this.light.get_info_panel().light_name.setBackground(new Color(64+counter,64+counter,64+counter));
                this.light.get_info_panel().type_name.setBackground(new Color(64+counter,64+counter,64+counter));

                if(z<=9){
                    z++;
                    if(p==-1){
                        counter+=15;
                    }
                    else{
                        counter-=15;
                    }

                }
                else{
                    q++;
                    if(p ==-1){p =1;} else{p=-1;}
                    z = 0;
                }
                if(q==2){break;}
            }
            if(this.light.get_State()){
                this.light.setBackground(this.light.get_Color());
            }
            else{
                this.light.setBackground(Color.black);
            }

            this.light.get_info_panel().setBackground(Color.darkGray);
            this.light.get_info_panel().light_name.setBackground(Color.darkGray);
            this.light.get_info_panel().type_name.setBackground(Color.darkGray);

        }

    }



    ////////////////////////////////////
    class MyThread extends Thread{

        int counter = 255;
        int p = -1;


        public MyThread(){

        }

        public void run(){
            while(true){
                light_panel.repaint();
                try{
                    /*if(counter == 255 || counter == 0){
                        this.sleep(160);
                    }*/
                    //else{
                        this.sleep(50);
                    //}

                }
                catch(Exception e){}

                for(int i=0; i<lines.size(); i++){
                    lines.get(i).dash_p+=1.5f;
                    if(lines.get(i).dash_p>200.0f){
                        lines.get(i).dash_p=0.0f;
                    }
                    if(lines.get(i).get_State() && !switch_state){
                        lines.get(i).color = new Color(70-counter/4 ,255-counter/2 -counter/4 ,180-counter/2); //70-counter/4 ,255-counter/2 ,150-counter/2

                    }
                    else if(!lines.get(i).get_State() ||  (lines.get(i).get_State() && switch_state)){
                        lines.get(i).color = new Color(counter/4 + counter/2,0,0);
                    }
                    if(!switch_state){
                        power.setBorder(BorderFactory.createLineBorder(new Color(counter/4 + counter/2,0,0),2));
                    }
                    else{
                        power.setBorder(BorderFactory.createLineBorder(new Color(70-counter/4 ,255-counter/2 ,180-counter/2),2));
                    }
                }
                if(counter>0 && counter<255){
                    if(p==-1){
                        counter-=15;
                    }
                    else{
                        counter+=15;
                    }
                }
                else if(counter==0){
                    p = 1; counter+=15;
                }
                else if(counter==255){
                    p = -1; counter-=15;
                }


            }
        }
    }
    //////////////////////////////
    class MyThread3 extends Thread{

        int counter = 255;
        int p = -1;
        Light light;
        int z = 0;



        public MyThread3(Light light){
            this.light = light;
        }

        public void run(){
            while(light.get_State() && !switch_state && (!light.is_Selected())){ //right_click_selection do something
                light_panel.repaint();
                try{
                    this.sleep(20);

                }
                catch(Exception e){}

                light.setBorder(BorderFactory.createLineBorder(new Color(70-counter/4 ,255-counter/2 ,150-counter/2),2));
                light.get_info_panel().setBorder(BorderFactory.createLineBorder(new Color(70-counter/4 ,255-counter/2 ,150-counter/2),2));

                if(counter>0 && counter<255){
                    if(p==-1){
                        counter-=15;
                    }
                    else{
                        counter+=15;
                    }
                }
                else if(counter==0){
                    p = 1; counter+=15;
                    z++;
                }
                else if(counter==255){
                    z++;
                    p = -1; counter-=15;
                }
                if(z==3){break;}

            }
            light.get_info_panel().setBorder(null);
            if(!light.get_State() || switch_state){
                light.setBorder(BorderFactory.createLineBorder(Color.red,2));
            }
            else{
                light.setBorder(BorderFactory.createLineBorder(Color.green,2));
            }
            if(light.isHover()){
                light.setHover(true);
            }
            if(light.isPressed()){
                light.setPressed(true);
            }
        }
    }



    public class MyThread5 extends Thread{
        int indexx;
        Light_Info2 panel;
        Light light;
        int varP =0;
        int varH = 0;
        String varS =" ";
        int check = 0;
        int q = 0;
        int z = 0;
        int x =0;
        int y = 31;


        public MyThread5(int i){
            this.indexx = i;
            this.light = light_system.get(i);
            this.panel = light_system.get(i).get_info2_panel();
        }

        public void run(){

            revalidate(); repaint();
            if(state_paint){
                bottom_panel.clear_state = false;

            }
            bottom_panel.length = 0;
            bottom_panel.add(x,y);

            Random r = new Random();
            panel.setVisible(true);

            //bottom_panel.x1 = x; bottom_panel.y1 = y;

            revalidate(); repaint();

            while(light.isHover() && !dragged && !selection){

                if(x+200+60-5>getWidth()){

                    bottom_panel.reset();
                    x = 0; bottom_panel.add(x,y);
                }

                revalidate(); repaint();

                try{
                    this.sleep(50);
                }catch(Exception e){}
                if(varH-check>=10){
                    varS+=". ";
                    check = varH;
                    if(varS.equals(" . . . . . ")){
                        varS=" ";
                    }

                }
                q++; x = x+2; //bottom_panel.x1 = x; bottom_panel.x2 = x+1;

                if(light.isConnected() && light.get_State() && !switch_state && q>1){
                     bottom_panel.state = true;
                     q = 0;
                     varP = r.nextInt(600-200)+200; varH++;
                     y = varP/10 - 10;

                     //bottom_panel.y1 = bottom_panel.y2;
                     //bottom_panel.add(x,bottom_panel.y2);
                     //bottom_panel.length++;
                     this.panel.setText(" __INFO__\n Power : "+varP+"W \n Hours : "+varH+"\n" +varS);
                     //bottom_panel.y2 = y;
                     //bottom_panel.add(x+1,y);

                }

                bottom_panel.add(x,y);


            }
            bottom_panel.state = false;

            revalidate(); repaint();
            bottom_panel.clear();
            bottom_panel.reset();
            revalidate(); repaint();


            panel.setVisible(false);
            panel.repaint();

        }
    }


    public class MyThread6 extends Thread{


        int x1,x2,x3,x4,y1,y2,y3,y4;
        int q = 0;
        int starting_x,ending_x;
        int starting_y,ending_y;

        public MyThread6(){

        }

        public void run(){

            ext_panel.reset();
            ext_panel.state = true;
            starting_x = (int)ext_panel.points_a[0].getX();  starting_y = (int)ext_panel.points_a[0].getY();
            ending_x = (int)ext_panel.points_a[2].getX();    ending_y = (int)ext_panel.points_a[2].getY();

            while(extend_state){
                while(true){
                    if(!extend_state){break;}
                    q++;
                    revalidate();
                    repaint();
                    try{
                        this.sleep(40);
                    }
                    catch (Exception e){}

                    //de3ia peristrofh
                    if(ext_panel.points_a[0].x <ext_panel.points_a[1].x){

                        //aristerh plevra
                        ext_panel.points_a[0].x++; ext_panel.points_b[0].x++;
                        ext_panel.points_a[3].x++; ext_panel.points_b[3].x++;

                        ext_panel.points_a1[0].x++; ext_panel.points_b1[0].x++;
                        ext_panel.points_a1[3].x++; ext_panel.points_b1[3].x++;
                        if(q>1){
                            ext_panel.points_a[0].y--; ext_panel.points_b[0].y--;
                            ext_panel.points_a[3].y++; ext_panel.points_b[3].y++;

                            ext_panel.points_a1[0].y--; ext_panel.points_b1[0].y--;
                            ext_panel.points_a1[3].y++; ext_panel.points_b1[3].y++;

                        }

                        //de3ia plevra
                        ext_panel.points_a[1].x--;  ext_panel.points_b[1].x--;
                        ext_panel.points_a[2].x--;  ext_panel.points_b[2].x--;

                        ext_panel.points_a1[1].x--;  ext_panel.points_b1[1].x--;
                        ext_panel.points_a1[2].x--;  ext_panel.points_b1[2].x--;
                        if(q>1){
                             ext_panel.points_a[1].y++; ext_panel.points_b[1].y++;
                             ext_panel.points_a[2].y--; ext_panel.points_b[2].y--;

                            ext_panel.points_a1[1].y++; ext_panel.points_b1[1].y++;
                            ext_panel.points_a1[2].y--; ext_panel.points_b1[2].y--;

                        }
                        if(q>1){q=0;}

                    }
                    //aristerh peristrofh
                    else {

                        ext_panel.points_a[0].x++; ext_panel.points_b[0].x++;
                        ext_panel.points_a[3].x++; ext_panel.points_b[3].x++;

                        ext_panel.points_a1[0].x++; ext_panel.points_b1[0].x++;
                        ext_panel.points_a1[3].x++; ext_panel.points_b1[3].x++;
                        if(q>1){
                            ext_panel.points_a[0].y++; ext_panel.points_b[0].y++;
                            ext_panel.points_a[3].y--; ext_panel.points_b[3].y--;

                            ext_panel.points_a1[0].y++; ext_panel.points_b1[0].y++;
                            ext_panel.points_a1[3].y--; ext_panel.points_b1[3].y--;
                        }

                        //de3ia plevra
                        ext_panel.points_a[1].x--; ext_panel.points_b[1].x--;
                        ext_panel.points_a[2].x--; ext_panel.points_b[2].x--;

                        ext_panel.points_a1[1].x--; ext_panel.points_b1[1].x--;
                        ext_panel.points_a1[2].x--; ext_panel.points_b1[2].x--;
                        if(q>1){
                            ext_panel.points_a[1].y--; ext_panel.points_b[1].y--;
                            ext_panel.points_a[2].y++; ext_panel.points_b[2].y++;

                            ext_panel.points_a1[1].y--; ext_panel.points_b1[1].y--;
                            ext_panel.points_a1[2].y++; ext_panel.points_b1[2].y++;

                        }
                        if(q>1){q=0;}

                    }
                    if(ext_panel.points_a[0].x == ending_x){
                        ext_panel.reset();
                        ext_panel.state = true;
                        q = 0;
                    }

                }//inner while

            }//outer while
        }
    }



    /*-------------------------------------------------------------------------------------------------------------------*/


    public void reset_selected(Light light){
        boolean st = false;
        if(light == null){st = true;}

        for(int i=0; i<light_system.size(); i++){

            if(st){
                light =light_system.get(i);
            }
            light.get_info_panel().reset();
            if(!switch_state && light.isConnected() && light.get_State()){
                light.setBorder(BorderFactory.createLineBorder(Color.green,2));
            }
            else {
                light.setBorder(BorderFactory.createLineBorder(Color.red, 2));
            }
            light.Set_Selected(false);
            light_panel.repaint();
            if(!st){break;}
        }
    }






    public void move_light(Light light , int x,int y,int ind){


        light.get_info_panel().setLocation(x,y);
        light.setLocation(x,y-41);
        light.get_info2_panel().setLocation(x-100+20-3,y-41-1);
        //light.get_info_panel().data_area.setBounds(light.get_info_panel().data_panel.getInsets().left ,light.get_info_panel().data_panel.getInsets().top,100,200);
        //data_panel.setBounds(light_system.get(i).getX()-100+20-3,light_system.get(i).getY()-1,75+5,93);

        light.setX(light.getX());
        light.setY(light.getY());

        Color c = light.get_Color();


        light.get_info_panel().setBackground(new Color(64,64,64,40));
        light.get_info_panel().light_name.setBackground(new Color(64,64,64,40));
        light.get_info_panel().type_name.setBackground(new Color(64,64,64,40));
        light.get_info_panel().type.setForeground(new Color(125,125,125,70));
        light.get_info_panel().name.setForeground(new Color(125,125,125,70));
        light.get_info_panel().light_name.setForeground(Color.lightGray);
        light.get_info_panel().type_name.setForeground(Color.lightGray);


        if(light.isConnected()){

            light_panel.get_Line(ind).set_p(2);

            if(light.get_State() && !switch_state){
                if(!c.equals(Color.white)){
                    light.setBackground(new Color(c.getRed()/2,c.getGreen()/2,c.getBlue()/2));
                }
                else {
                    light.setBackground(new Color(125,125,125,50));
                }

            }
            else{
                light.setBackground(new Color(0,0,0,40));
            }
            light_panel.get_Line(ind).set_Cords(light.getX() + 30, light.getY() + 20, this.getWidth() / 2 - 30, 0);

            light_panel.revalidate();
            light_panel.repaint();
        }
        else{
            light.setBackground(new Color(0,0,0,40));
        }
         light.get_info_panel().revalidate();
         light.get_info_panel().repaint();
         light.get_info2_panel().revalidate(); light.get_info2_panel().repaint();
         light.revalidate(); light.repaint();
         //light.get_info_panel().data_panel.revalidate();
         //light.get_info_panel().data_panel.repaint();

    }



    /*Updating light_panel's components*/
    public void update_light_panel() {

        light_panel.removeAll();
        light_panel.revalidate();
        light_panel.repaint();

        Insets insets = light_panel.getInsets();
        int distance_x = 166;
        int distance_y = 20;
        int counter = 0;
        int layers = 1;
        int count = 0;

        int times_x = light_panel.getWidth()/(150+20);

        light_panel.clear_lines();
		/*Setting Lights*/
        for (int i = 0; i<light_system.size(); i++) {
            if(light_system.get(i).getXX()==-1 || light_system.get(i).getYY()==-1){
                light_system.get(i).setBounds(insets.left + (counter*distance_x) + 10, insets.top + distance_y, 45, 40);
                light_system.get(i).get_info_panel().setBounds(light_system.get(i).getX(),light_system.get(i).getY()+41, 150, 50);
                light_system.get(i).get_info2_panel().setBounds(light_system.get(i).getX()-100+20-3,light_system.get(i).getY()-1,75+5,93);
            }
            else{
                light_system.get(i).setBounds(light_system.get(i).getX(),light_system.get(i).getY(), 45, 40);
                light_system.get(i).get_info_panel().setBounds(light_system.get(i).getX(),light_system.get(i).getY()+41, 150, 50);
                light_system.get(i).get_info2_panel().setBounds(light_system.get(i).getX()-100+20-3,light_system.get(i).getY()-1,75+5,93);
            }

            //light_system.get(i).get_info_panel().setOpaque(false);
            light_panel.add(light_system.get(i));
            light_panel.add(light_system.get(i).get_info_panel());
            light_panel.add(light_system.get(i).get_info2_panel());
            light_panel.setComponentZOrder(light_system.get(i).get_info2_panel(),0);

			/*Setting Line for each Light*/
            if (!switch_state) {
                lines.get(i).update();
            }
            lines.get(i).set_Cords(light_system.get(i).getX() + 30, light_system.get(i).getY() + 20, this.getWidth() / 2 - 30, 0);
            if (light_system.get(i).isConnected()) {
                lines.get(i).switch_con(true);
            }
            else{
                lines.get(i).switch_con(false);
            }
            light_panel.add_Line(lines.get(i));

			/* */
            counter++;
            if (counter == times_x) {
                layers++;
                counter = 0;
                distance_y += 120;
            }
            if(light_system.get(i).get_State()){
                count++;
            }

        }
		/*Setting light_panel's size according to it's lights*/
        light_panel.setPreferredSize(new Dimension(this.getWidth()-35, layers * 100 + 250));


		/*Updating light_panel's components*/
        light_panel.revalidate();
        light_panel.repaint();

    }
	/*- - - - - - - - - - - - - - - - - -*/


	/*- - - - - - - - - - - - - - - - - FUNCTIONS - - - - - - - - - - - - - - - - - -*/
	public void switch_light(int x){
	    boolean st = false;

	    /*case : switch_state is : [off] ~ (lights are getting powered)*/
	    for(int i=0; i<light_system.size(); i++){
	        if(x!=-1){
	            st = true;
	            i = x;
            }
            else{

            }
            lines.get(i).set_Power(!switch_state);
            if (!switch_state) {
                light_system.get(i).swap(true); // switching Light state[true/false] and it's background,border..
                lines.get(i).swap(); // switching the relative line's state[true/false] also

                if (light_system.get(i).isConnected()) {
                    lines.get(i).update();// updating line's Color

                    if(light_system.get(i).get_State()){
                        th3 = new MyThread3(light_system.get(i));
                        th3.start();
                        //update_lighthning(1);
                    }
                    else{
                        //update_lighthning(-1);
                    }
                }
                else{
                    //update_lighthning(-1);
                }

                //light_panel.repaint(); // updating light_panel
                //light_panel.repaint(light_system.get(i));
            }//
					 /*case : switch_state is : [on] ~ (lights may switch states but aren't getting powered) */
            else {
                light_system.get(i).swap(false); //switching only the state[true/false] of Light, not updating it's background/border etc..
                lines.get(i).swap(); // switching only the relative line's state[true/false] , not it's color.
            }
            if(st){
                break;
            }
            light_system.get(i).repaint();

        }
        //System.out.println(lines.get(0).isPowered());
    }

    public void update_lighthning(int x){
    }
    /*- - - - - - - - - - - - - - - - - -*/

    /*Add Light Dialog*/
    public void show_add_light_dialog() {
        this.setEnabled(false);
        choose_window = new light_choose_Frame();
        choose_window.addWindowListener(this);
    }
	/*- - - - - - - - - - - - - - - - - -*/

    /*Add Light*/
    public void add_Light(String type) {


        if (type.equals("Regular light")) {
            normal_Light temp = new normal_Light(choose_window.getLight_label());
            temp.addActionListener(this);
            temp.setFocusPainted(false);
            light_system.add(temp);

        }
        else {
            rgb_Light temp2 = new rgb_Light(choose_window.getLight_label(), choose_window.getLight_color());
            temp2.addActionListener(this);
            temp2.setFocusPainted(false);
            light_system.add(temp2);
        }
        light_system.get(light_system.size() - 1).addMouseMotionListener(this);
        light_system.get(light_system.size() - 1).get_info_panel().addMouseMotionListener(this);
        light_system.get(light_system.size() - 1).get_info_panel().addMouseListener(this);

        lines.add(new Line());
        if(switch_state){
            lines.get(lines.size()-1).set_Power(false);
        }

        update_light_panel();

        Thread t0 = new MyThread2(light_system.get(light_system.size()-1));
        t0.start();


    }
	/*- - - - - - - - - - - - - - - - - -*/

    /*Remove Light*/
    public void remove_Light(int item) {
        if(item !=-1){
            int result = JOptionPane.showConfirmDialog(null, "Light[" + light_system.get(item).get_Label() + "] will get Removed!\nAre you sure you want to continue?",
                    "!", JOptionPane.OK_CANCEL_OPTION);
            if (result == JOptionPane.OK_OPTION) {
                light_system.remove(item);
                lines.remove(item);
                update_light_panel();
            }
        }
        else{

            int result = JOptionPane.showConfirmDialog(null, " Lights will get Removed!\nAre you sure you want to continue?",
                    "!", JOptionPane.OK_CANCEL_OPTION);
            if (result == JOptionPane.OK_OPTION) {

                for(int i=light_system.size()-1; i>=0; i--){
                    if(light_system.get(i).is_Selected()){
                       light_system.remove(i);
                       lines.remove(i);
                    }
                }
                update_light_panel();
            }

        }

    }
	/*- - - - - - - - - - - - - - - - - -*/


    /*Saving Current State Dialog and File writing*/
    public void save_current_state() {

        JFileChooser chooserr = new JFileChooser();
        chooserr.setCurrentDirectory(new java.io.File("."));
        chooserr.setDialogTitle("Choose directory");
        chooserr.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);

        chooserr.setAcceptAllFileFilterUsed(false);

        if (chooserr.showOpenDialog(this) == JFileChooser.APPROVE_OPTION) {
            try {
                FileWriter fw = new FileWriter(chooserr.getSelectedFile() + "\\saved_current_state.txt");
                BufferedWriter bw = new BufferedWriter(fw);

                bw.write("-----Current State of Network------");
                bw.newLine();
                bw.write("-main_swith /state : [" + switch_state + "]-\n");
                bw.newLine();
                for (int i = 0; i<light_system.size(); i++) {
                    if (light_system.get(i).getClass().getName().equals("normal_Light")) {
                        bw.write("-Light<" + light_system.get(i).get_Label() + "> /State : [" + light_system.get(i).get_State() + "] /Power : [" + (!switch_state && light_system.get(i).isConnected()) + "] /Type : [Regular]");
                    }
                    else {
                        Color c = light_system.get(i).get_Color();
                        int r = c.getRed();
                        int g = c.getGreen();
                        int b = c.getBlue();
                        bw.write("-Light<" + light_system.get(i).get_Label() + "> /State : [" + light_system.get(i).get_State() + "] /Power : [" + (!switch_state && light_system.get(i).isConnected())+ "] /Type : [rgb] {" + r + "," + g + "," + b + "}");
                    }

                    bw.newLine();
                }

                bw.close();
                JOptionPane.showMessageDialog(null, "File saved Succesfully!");
            }
            catch (Exception e) {
                JOptionPane.showMessageDialog(null, "Oops ! Something went wrong!\nPlease Try again!");
            }

        }
        else {

        }

    }
	/*- - - - - - - - - - - - - - - - - -*/


    public void show_edit_dialog(int index) {
        this.setEnabled(false);
        choose_window = new light_choose_Frame(light_system.get(index));
        choose_window.addWindowListener(this);
        pointer = index;
    }

    public void edit_Light() {

        Color c = choose_window.getLight_color();
        String label = choose_window.getLight_label();
        if (!light_system.get(pointer).getClass().getName().equals("normal_Light")) {
            light_system.get(pointer).set_Color(c);
            if (light_system.get(pointer).isConnected() && !switch_state) {
                light_system.get(pointer).update();
            }
        }
        light_system.get(pointer).set_Label(label);
        update_light_panel();
    }


    /*Right Click Menu*/
    class PopUpMenu extends JPopupMenu {

        public PopUpMenu() {
            if(!selection){
                add(edit);
                add(cut_power);
                add(remove_light2);
            }
            else{
                add(cut_power);
                add(remove_light2);
            }

        }

        public PopUpMenu(int a){
            add(add_Light2);
        }
    }
    //

    /*Showing Righ Click Menu when a right_click occurs*/
    public void popMenu(MouseEvent e) {
        PopUpMenu menu = new PopUpMenu();
        menu.setBorder(BorderFactory.createLineBorder(Color.darkGray));
        menu.show(e.getComponent(), e.getX(), e.getY());
    }
    public void popMenu2(MouseEvent e){
        PopUpMenu menu = new PopUpMenu(1);
        menu.setBorder(BorderFactory.createLineBorder(Color.darkGray));
        menu.show(e.getComponent(), e.getX(), e.getY());
    }
	/*- - - - - - - - - - - - - - - - - -*/



	/*- - - - - - - LISTENERS - - - - - - -*/


    /*Action Listener*/
    @SuppressWarnings("Duplicates")
    public void actionPerformed(ActionEvent e) {


		/*- - - - Event : main_switch - - - -*/
		/*Checking if main_switch is : [on/off] - handling power to light*/
        if (e.getSource().equals(main_switch)) {
            for(int i=0; i<light_panel.Size(); i++){
                light_panel.get_Line(i).set_Power(switch_state);
            }
			/*case : main_switch is : [off]*/
            if (!switch_state) {
                switch_state = !switch_state;
                power.setBackground(Color.white);//Updating power[button] to white background indicating that main_switch is turning to : [on]
                power.setBorder(BorderFactory.createLineBorder(Color.green, 2));//Updating it's border..

																				/*turning power off for every <Light> component*/
                for (int i = 0; i<light_system.size(); i++) {
                    light_system.get(i).turn_off();
                    lines.get(i).reset();

                }

                 //light_panel.repaint(); //Updating light_panel's components

            }
			 /*case : main_switch is : [on]*/
            else {
                switch_state = !switch_state;
                power.setBackground(Color.black); //Updating power[button] to black background indicating that main_switch is turning to : [off]
                power.setBorder(BorderFactory.createLineBorder(Color.red, 2)); //Updating it's border..

																			   /*Updating the state of each <Light> component once the power is going through the Lights again*/
                for (int i = 0; i<light_system.size(); i++) {

                    light_system.get(i).update();

                    if (light_system.get(i).isConnected()) {
                        lines.get(i).update();
                        if(light_system.get(i).get_State()){
                            MyThread3 t3 = new MyThread3(light_system.get(i));
                            t3.start();
                        }

                    }
                }
                //light_panel.repaint();//Updating light_panel
            }

            //switch_state = !switch_state;//switching main_switch state to : [on]->[off] / [off]->[on]
            power_info.set_State(switch_state);//updating main_switch's info.
        }
		/*- - - - - - - - - - - - - - - - - -*/


		/*- - - - Event : add_light - - - -*/
        else if (e.getSource().equals(add_light) || e.getSource().equals(add_Light2)) {
            if(t!=null){if(t.isAlive()){
                try{
                    t.join();
                }catch(Exception ae){}}

            }
            show_add_light_dialog(); //showing dialog in order to add new Light
        }
		/* */

		/*- - - - Event : remove_light - - - -*/
        else if (e.getSource().equals(remove_light2) || e.getSource().equals(remove_light)) {
			/*checking wich <Light> component is currently Focused/Pressed */
            if(selection){
                remove_Light(-1);
            }
            else{
                for (int i = 0; i<light_system.size(); i++) {

                    if (light_system.get(i).isPressed()) {
                        remove_Light(i);
                    }
                }

            }

        }
        else if (e.getSource().equals(new_network)) {
            int result = JOptionPane.showConfirmDialog(null,"This Light-network will get Removed!\nAre you sure you want to continue?",
                    "!", JOptionPane.OK_CANCEL_OPTION);
            if (result == JOptionPane.OK_OPTION) {
                lines.clear();
                light_system.clear();
                update_light_panel();
            }

        }
        else if (e.getSource().equals(save_state)) {
            save_current_state();
        }
        else if (e.getSource().equals(edit) || e.getSource().equals(edit2)) {
            for (int i = 0; i<light_system.size(); i++) {
                if (light_system.get(i).isPressed()) {
                    show_edit_dialog(i);
                    break;
                }
            }
        }

        else if (e.getSource().equals(cut_power) || e.getSource().equals(cut_power2)) {

            Light light;

            for (int i = 0; i<light_system.size(); i++) {
                if(selection){
                    if(light_system.get(i).is_Selected()){
                        light = light_system.get(i);
                    }
                    else{
                        continue;
                    }
                }
                else{
                    light = light_system.get(i);
                }
                if(selection){

                    light.swap_connection();
                    if (light.isConnected()) {
                        if (!switch_state && light.get_State()) {
                            //reset_selected(light_system.get(i)); //error
                            light.setBackground(light.get_Color());
                            //light.Set_Selected(false);
                            //MyThread3 t3 = new MyThread3(light);
                            //t3.start();
                        }

                    }
                    else {
                        //light.turn_off();
                        light.setBackground(Color.black);
                    }

                    update_light_panel();

                }
                else if (light.isPressed()) {
                    light.swap_connection();
                    if (light.isConnected()) {
                        if (!switch_state) {
                            light.update();
                            MyThread3 t4 = new MyThread3(light);
                            t4.start();
                        }

                    }
                    else {
                        light.turn_off();
                    }

                    update_light_panel();
                }
            }
            //reset_selected(null); //error

        }

		/* */

		/*- - - - Event : Light Component - - - -*/
        else if (e.getSource() instanceof Light || e.getSource().equals(switch_states)) {
            if(selection){
                reset_selected(null); //error
            }
			/*checking wich <Light> component is pressed*/
			if(e.getSource().equals(switch_states)){
			    switch_light(-1);
            }
            else{
                for (int i = 0; i<light_system.size(); i++) {
                    if (e.getSource().equals(light_system.get(i))) {
                        switch_light(i);
                    }
                }
            }

        }
        else if(e.getSource().equals(reset_positions)){
            if(selection){
                reset_selected(null); //error
            }
            for(int i=0; i<light_system.size(); i++){
                light_system.get(i).setX(-1); light_system.get(i).setY(-1);
            }
            update_light_panel();
        }
        else if(e.getSource().equals(reset_states)){
           // switch_state = false;
            lines.clear();
            for(int i=0; i<light_system.size(); i++){
                light_system.get(i).reset();
                light_system.get(i).update();
                Line line = new Line();
                line.set_Power(true);
                lines.add(line);
            }

            update_light_panel();
        }
        else if(e.getSource().equals(dashed_lines)){
            light_panel.swap_dashed();
        }
        else if(e.getSource().equals(circle_lines)){
            light_panel.swap_circle_effect();
        }

        else if(e.getSource().equals(paint)){

            state_paint = !state_paint;
            //t.join();
        }
        //
        else if(e.getSource().equals(extend_panel)){

            if(!extend_state){
                MyThread6 tt = new MyThread6(); tt.start();
                light_panel_scroll.setSize(light_panel_scroll.getWidth()-220,light_panel_scroll.getHeight());
                light_panel_scroll.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
                ext_panel.setVisible(true);
            }
            else{
                light_panel_scroll.setSize(light_panel_scroll.getWidth()+220,light_panel_scroll.getHeight());
                light_panel_scroll.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
                ext_panel.setVisible(false);
            }
            extend_state = !extend_state;
            update_light_panel();
        }



    }
	/*- - - - - - - - - - - - - - - - - -*/




    /*MouseInput Listener*/
    public void mousePressed(MouseEvent e) {

        if(selection){
            boolean s = false;
            for(int i=0; i<light_system.size(); i++){
                if(light_system.get(i).get_info_panel().equals(e.getSource()) && light_system.get(i).is_Selected()){
                    s = true;
                    break;
                }
                else{
                    continue;
                }
            }
            if(!s){

                selection = false;
                reset_selected(null); //error
            }
        }



        if(e.getSource().equals(light_panel) && !SwingUtilities.isRightMouseButton(e)){
            x1 = e.getX(); y1 = e.getY();
            panel_pressed = true;
        }


        else if(SwingUtilities.isRightMouseButton(e) && e.getSource().equals(light_panel)){
            popMenu2(e);
            panel_pressed = false;
            x1 =-1;
            y1 =-1;
        }

		/*- - - - Event : Right Click at a <Light> component's info panel - - - -*/
        else if (SwingUtilities.isRightMouseButton(e) && e.getSource() instanceof Light_Info) {
            right_click = true;

            for (int i = 0; i<light_system.size(); i++) { //Checking wich one is pressed .
                if (light_system.get(i).get_info_panel().equals(e.getSource())) {
                    if(!light_system.get(i).is_Selected()){

                        selection = false;
                        reset_selected(null); //error
                    }

                    if(selection){
                        right_click_selection = true;
                    }
                    popMenu(e); //showing Confirmation window [ok/cancel] before permantly removing the light.
                    break;
                }
            }
        }

		/* */

		/*- - - - Event : [General] Clicking at a <Light> component's info panel - - - -*/
        if (e.getSource() instanceof Light_Info) {

            pressed = true; x = e.getX(); y = e.getY();
            //checking wich one ..
            for (int i = 0; i<light_system.size(); i++) {
                if (e.getSource().equals(light_system.get(i).get_info_panel()) && !light_system.get(i).is_Selected()) {
                    light_system.get(i).setPressed(true); //setting the <Light> component to Pressed[true] state.
                    pressed_light = light_system.get(i);
                    //lines.get(i).set_p(0);
                    index = i;
                }
                else if(e.getSource().equals(light_system.get(i).get_info_panel()) && light_system.get(i).is_Selected()){
                    //light_system.get(i).setPressed(false);
                    if(!right_click){
                        selected_pressed = light_system.get(i);
                        index = i;

                    }
                    break;
                }
                else if(!light_system.get(i).is_Selected()) {

                    light_system.get(i).setPressed(false);
                    if (switch_state) {
                        light_system.get(i).setBorder(BorderFactory.createLineBorder(Color.red, 2));
                    }
                }

            }



        }
		/* */

		/*- - - - Event : [General] Clicking at any other Component instead of a <Light> component's info panel - - - -*/
        else {

            for (int i = 0; i<light_system.size(); i++) {
                light_system.get(i).setPressed(false); // reseting it's Light's GUI to it's previous state .
                if (switch_state) {
                    light_system.get(i).setBorder(BorderFactory.createLineBorder(Color.red, 2));
                }
            }

        }
		/* */

    }
    //


    public void mouseReleased(MouseEvent e) {


        light_panel.dragging = false;
        panel_pressed = false;

        if(pressed && dragged){
            if(index!=-1 && pressed_light.isConnected()){
                light_panel.get_Line(index).set_p(0);
            }
            if(!pressed_light.equals(selected_pressed)){
                pressed_light.get_info_panel().setBackground(new Color(64,64,64));
                pressed_light.get_info_panel().light_name.setBackground(new Color(64,64,64));
                pressed_light.get_info_panel().type_name.setBackground(new Color(64,64,64));
                pressed_light.get_info_panel().light_name.setForeground(Color.white);
                pressed_light.get_info_panel().type_name.setForeground(Color.white);

                pressed_light.get_info_panel().type.setForeground(Color.lightGray);
                pressed_light.get_info_panel().name.setForeground(Color.lightGray);

                if(!switch_state && pressed_light.get_State() && pressed_light.isConnected()){
                    pressed_light.setBackground(pressed_light.get_Color());
                }
                else{
                    pressed_light.setBackground(Color.black);
                }
            }
            else if(selection){
                for(int i=0; i<light_system.size(); i++){
                    light_panel.get_Line(i).set_p(0);
                    if(light_system.get(i).is_Selected()){
                        if(!switch_state && light_system.get(i).get_State() && light_system.get(i).isConnected()){
                            light_system.get(i).setBackground(light_system.get(i).get_Color());
                        }
                        else{
                            light_system.get(i).setBackground(Color.black);
                        }

                    }
                }
            }


        }
        pressed_light = null;
        selected_pressed = null;

        dragged = false;
        pressed = false;
        right_click = false;
        pressed_light = null;
        index = -1;


    }


    public void mouseEntered(MouseEvent e) {

		/*- - - - Event : MouseInput entering a <Light> component's info panel - - - -*/
        if (e.getSource() instanceof Light_Info) {

            for (int i = 0; i<light_system.size(); i++) {
                if (e.getSource().equals(light_system.get(i).get_info_panel()) && !light_system.get(i).is_Selected() ) {
                    light_system.get(i).setHover(true); // setting the <Light> component to Hover[true] state.

                    MyThread5 t5 = new MyThread5(i); t5.start();


                    if(!dragged && !panel_pressed ) {
                        lines.get(i).set_p(1);

                        //MyThread4 ts = new MyThread4(i); ts.start();
                        //light_system.get(i).get_info_panel().show_data();
                    }
                }
                //reseting each <Light> component's which is not currently [mouseInput entered] to it's previous GUI state
                else if(!light_system.get(i).is_Selected()){
                    light_system.get(i).setHover(false);

                    //light_system.get(i).get_info_panel().off_data();
                    lines.get(i).set_p(0);
                    if (switch_state) {
                        light_system.get(i).setBorder(BorderFactory.createLineBorder(Color.red, 2));
                    }
                }

            }
        }
		/*- - - - Event : MouseInput entering at <remove_light2>/<cut_power> JMenuItem - - - -*/
        else if (e.getSource().equals(remove_light)) {
            remove_light.setBackground(new Color(226, 74, 74));
            remove_light.setForeground(Color.black);
        }
        else if (e.getSource().equals(add_light)) {
            add_light.setBackground(new Color(139, 255, 167));
            add_light.setForeground(Color.black);
        }
        else if (e.getSource().equals(main_switch)) {
            main_switch.setBackground(Color.lightGray);
            main_switch.setForeground(Color.darkGray);
        }
		/* */

    }//


    public void mouseExited(MouseEvent e) {

		/*- - - - Event : MouseInput exiting from a <Light> component's info panel - - - -*/
        if (e.getSource() instanceof Light_Info) {

            for (int i = 0; i<light_system.size(); i++) {
                if (e.getSource().equals(light_system.get(i).get_info_panel()) && !light_system.get(i).is_Selected()) {


                    //setting the <Light> component to it's previous GUI state.
                    light_system.get(i).setHover(false);
                    lines.get(i).set_p(0);
                    if (switch_state && !light_system.get(i).isPressed()) {
                        light_system.get(i).setBorder(BorderFactory.createLineBorder(Color.red, 2));
                    }
                }

            }
        }
		/*- - - - Event : MouseInput exiting from <remove_light2>/<cut_power> JMenuItem - - - -*/
        else if (e.getSource().equals(remove_light)) {
            remove_light.setBackground(Color.lightGray);
            remove_light.setForeground(Color.black);
        }
        else if (e.getSource().equals(add_light)) {
            add_light.setBackground(Color.lightGray);
            add_light.setForeground(Color.black);
        }
        else if (e.getSource().equals(main_switch)) {
            main_switch.setBackground(Color.darkGray);
            main_switch.setForeground(Color.white);
        }
		/* */

    }//

    public void mouseClicked(MouseEvent e) {

    }

    /*- - - - - - - - - - - - - - - - - -*/
    public void mouseMoved(MouseEvent e) {

    }

    public void mouseDragged(MouseEvent e) {



        if(e.getSource().equals(light_panel) && x1!=-1 && y1!=-1 ){
            light_panel.dragging = true;
            int minx,maxx,miny,maxy;
            if(e.getX()>x1){
                maxx = e.getX(); minx = x1;
                if(e.getY()>y1){
                    maxy=e.getY();
                    miny = y1;
                    light_panel.set_Rect(x1,y1,e.getX()-x1,e.getY()-y1);
                }
                else{
                    maxy = y1;
                    miny = e.getY();
                    light_panel.set_Rect(x1,e.getY(),e.getX()-x1,y1-e.getY());
                }
            }
            else{
                minx = e.getX(); maxx = x1;
                if(e.getY()>y1){
                    maxy =e.getY(); miny=y1;
                    light_panel.set_Rect(e.getX(),y1,x1-e.getX(),e.getY()-y1);
                }
                else{
                    maxy = y1; miny = e.getY();
                    light_panel.set_Rect(e.getX(),e.getY(),x1-e.getX(),y1-e.getY());
                }
            }

            for(int i=0; i<light_system.size(); i++){
                int x = light_system.get(i).getX();
                int y = light_system.get(i).getY();
                if((x<=maxx && (x>minx || x>minx-40)) && (y<=maxy && (y>miny || y+40>miny))) {


                    light_system.get(i).get_info_panel().setHover();
                    light_system.get(i).setBorder(BorderFactory.createLineBorder(new Color(10,100,240,80),2));
                    light_system.get(i).Set_Selected(true);


                    selection = true;
                    //selected
                }
                else if(light_system.get(i).is_Selected()){
                    reset_selected(light_system.get(i));
                }
                //light_panel.revalidate();
                light_panel.repaint();
            }//

            //light_panel.repaint();
        }

        else if(e.getSource() instanceof Light_Info && pressed &&!right_click) {

            int dx = e.getComponent().getLocation().x - x;
            int dy = e.getComponent().getLocation().y - y;

            dragged = true;
            if (!selection) {
                e.translatePoint(e.getComponent().getLocation().x - x, e.getComponent().getLocation().y - y);
                move_light(pressed_light,e.getX(),e.getY(),index);

            }
            else {

                if(selection) {

                    pressed_light = selected_pressed;
                    //pressed_light.get_info_panel().off_data();
                }

                int x0 = pressed_light.getX();
                int y0 = pressed_light.getY();

                move_light(pressed_light, + pressed_light.getX()+e.getX(), e.getY() + pressed_light.getY(), index);

                int x1 = pressed_light.getX() - x0;
                int y1 = pressed_light.getY() - y0;

                if (selection) {
                    for (int i = 0; i < light_system.size(); i++) {

                        if (light_system.get(i).equals(pressed_light)) {
                            continue;
                        } else {
                            if (light_system.get(i).is_Selected()) {
                                move_light(light_system.get(i), light_system.get(i).get_info_panel().getX() + x1, light_system.get(i).get_info_panel().getY()+y1, i);
                            }

                        }
                    }
                }

            }

        }


    }
    /*- - - - - - - - - - - - - - - - - -*/


    /*Window Listener*/
    public void windowClosed(WindowEvent e) {

    }
    public void windowActivated(WindowEvent e) {

    }
    public void windowClosing(WindowEvent e) {

		/*- - - - Event : Window closing is not the Main Window - - - -*/
        if (!e.getSource().equals(this)) {
            if (!this.isEnabled()) {
                this.setEnabled(true); // Restoring Focus on the main window .
            }
        }
		/* */


		/*- - - - Event : Window closing is the 'add_light_window' - - - -*/
        if (e.getSource().equals(choose_window) && choose_window.get_State() == 0) {
            String type = choose_window.getLight_type(); // getting the type of <Light> {regular,rgb} user selected to add.
            add_Light(type); // adding the <Light> component .
        }
        if (e.getSource().equals(choose_window) && choose_window.get_State() == 1) {
            edit_Light();
        }
		/* */
    }

    public void windowDeactivated(WindowEvent e) {

    }
    public void windowDeiconified(WindowEvent e) {

    }
    public void windowIconified(WindowEvent e) {

    }
    public void windowOpened(WindowEvent e) {
    }
	/*- - - - - - - - - - - - - - - - - -*/

    public void componentResized(ComponentEvent e){


        if(e.getSource().equals(this)){
            counter++;
            if(counter>2){

                light_panel.setPreferredSize(new Dimension(this.getWidth()-35,this.getHeight()-125+ scroll_y));
                if(!extend_state){
                    light_panel_scroll.setBounds(frame_insets.left + 10, frame_insets.top + 100 - 40+1, this.getWidth()-35,this.getHeight()-125 + scroll_y);

                }
                else{
                    light_panel_scroll.setBounds(frame_insets.left + 10, frame_insets.top + 100 - 40+1, this.getWidth()-35-220,this.getHeight()-125 + scroll_y);

                }
                bottom_panel.setBounds(frame_insets.left + 10, frame_insets.top + 100 - 40+1 + light_panel_scroll.getHeight()+4, this.getWidth()-35, this.getHeight()- light_panel_scroll.getHeight() - 125- bot_y);
                bottom_panel.setPreferredSize(new Dimension(this.getWidth()-35, this.getHeight()- light_panel_scroll.getHeight() - 125- bot_y));


                power.setBounds((this.getWidth()-35) / 2 - 20-2, frame_insets.top + 60 - 40, 40, 40);
                power_info.setBounds((this.getWidth()-35) / 2 - 25 + 46-2, frame_insets.top + 51 - 40, 150, 50);

                ext_panel.setBounds(frame_insets.left + getWidth()-150-150+50+6+2,frame_insets.top+40+40-20+2,220+8-10-2,this.getHeight()-150-60);
                extend_panel.setBounds(frame_insets.left  + this.getWidth()-70+10+2 , frame_insets.top + 10 , 35,35);

                update_light_panel();
                this.repaint();
                this.getContentPane().revalidate(); this.getContentPane().repaint();

            }


        }
    }

    public void componentMoved(ComponentEvent e){

    }

    public void componentShown(ComponentEvent e){

    }

    public void componentHidden(ComponentEvent e){

    }


}