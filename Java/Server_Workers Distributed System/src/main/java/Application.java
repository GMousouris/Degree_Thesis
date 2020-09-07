
import javax.swing.*;
import javax.swing.filechooser.FileSystemView;
import javax.swing.text.DefaultCaret;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.util.ArrayList;
import java.util.TimerTask;
import java.util.Timer;


public class Application extends JFrame implements ActionListener,KeyListener,WindowListener{

    private MasterNode master;
    private ArrayList<ClientWorker> workers;
    private ArrayList<Line> lines;

    private Timer timer;
    private int opt;

    private MyButton init_data,get_entry,info,init_server;
    private MyButton server,redistribute,start;
    private MyButton accept_server , stop , reccomend;

    private ArrayList<MyButton> menu_buttons;

    private JPanel main,left,right,bottom;
    private JPanel temp_panel;
    private MyJPanel workers_panel;

    private MyNode serverNode;

    private JTextArea text;
    private JTextField user_text,user_txt_label,port_input;
    private JLabel port_label;

    private JScrollPane ct_scroll;
    private int final_width;

    private boolean rec = false;
    private Timer Timer;

    public Application(int width,int height){
        super("Application");

        lines = new ArrayList<Line>();

        final_width = width;
        this.addWindowListener(this);
        setSize(new Dimension(width,height));
        initGUI();

        master = new MasterNode(this);

        setMaximumSize(new Dimension(1000,650));
        setMinimumSize(new Dimension(600,600));
        setVisible(true);
        setDefaultCloseOperation(EXIT_ON_CLOSE);

    }


    void initGUI(){

        /*main jpanel*/
        main = new JPanel(null);

        main.setBackground(new Color(64,64,64));
        main.setSize(new Dimension(this.getWidth(),this.getHeight()));
        main.setBounds(this.getInsets().left+1,this.getInsets().top+1,getWidth(),getHeight());

        Insets m_insets = main.getInsets();


        /*left Jpanel*/
        left = new JPanel(null);
        left.setBackground(new Color(64,64,64));
        left.setBorder(BorderFactory.createLineBorder(Color.black));
        left.setSize(new Dimension(this.getWidth()-250-200,this.getHeight()-240));
        left.setBounds(m_insets.left+0,m_insets.top+0,left.getWidth(),left.getHeight()+50);

        /*right Jpanel*/
        right = new JPanel(null);
        right.setBackground(new Color(32,32,32));
        right.setBorder(BorderFactory.createLineBorder(Color.black));
        right.setSize(new Dimension(this.getWidth()-150-20,this.getHeight()-240));
        right.setBounds(m_insets.left+left.getWidth()+2,m_insets.top,right.getWidth(),right.getHeight()+50);

        /*bottom Jpanel*/
        bottom = new JPanel(null);
        bottom.setBackground(Color.lightGray);
        //bottom.setBorder(BorderFactory.createLineBorder(Color.black));
        bottom.setSize(new Dimension(this.getWidth()-20,this.getHeight()-400));
        bottom.setBounds(m_insets.left+0,m_insets.top+left.getHeight()+2+50,bottom.getWidth(),bottom.getHeight()+50);
        /* */

        /*Menu Buttons*/
        menu_buttons = new ArrayList<MyButton>();

        init_data = new MyButton("Initialize Data");
        init_data.addActionListener(this);
        menu_buttons.add(init_data);


        server = new MyButton("Server");
        server.addActionListener(this);
        server.setEnabled(true);
        menu_buttons.add(server);

        start = new MyButton("Start Process");
        start.addActionListener(this);
        start.setEnabled(false);
        menu_buttons.add(start);

        stop = new MyButton("Stop Process");
        stop.addActionListener(this);
        stop.setEnabled(false);
        menu_buttons.add(stop);

        reccomend = new MyButton("Reccomend");
        reccomend.addActionListener(this);
        reccomend.setEnabled(false);
        menu_buttons.add(reccomend);


        //setting buttons bounds//
        int dist = 0; int dist2 = 0;
        int count = 0;
        for(MyButton button:menu_buttons){

            button.setBounds(left.getInsets().left+3,left.getInsets().top+dist+dist2+2,button.getWidth()-4-4,button.getHeight());
            left.add(button);
            if(count>=3){
                dist2 = 31;
            }
            dist+=31;
            count++;

        }
        //...//

        /*Client Text*/
        text = new JTextArea();
        DefaultCaret caret = (DefaultCaret)text.getCaret();
        caret.setUpdatePolicy(DefaultCaret.ALWAYS_UPDATE);

        text.setEditable(false);
        text.setLineWrap(true);
        text.setBackground(new Color(19,12,10));
        text.setForeground(new Color(190,250,180));
        text.setFont(new Font(text.getFont().getFontName(),Font.PLAIN,16));

        //client text JScrollpane//
        ct_scroll = new JScrollPane(text);
        ct_scroll.setBounds(bottom.getInsets().left+1,bottom.getInsets().top+1,bottom.getWidth(),bottom.getHeight()-25);
        ct_scroll.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        ct_scroll.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
        ct_scroll.setBorder(null);
        bottom.add(ct_scroll);
        //

        user_txt_label = new JTextField(" /Command>");
        user_txt_label.setEditable(true);
        user_txt_label.setSize(new Dimension(74,22));
        user_txt_label.setBackground(new Color(19,12,10));
        user_txt_label.setOpaque(true);
        user_txt_label.setForeground(new Color(170,240,170));
        user_txt_label.setBorder(null);
        user_txt_label.setBounds(bottom.getInsets().left+1,bottom.getInsets().top+bottom.getHeight()-3-25+4,user_txt_label.getWidth(),user_txt_label.getHeight());
        bottom.add(user_txt_label);

        user_text = new JTextField();
        user_text.setFocusable(true);
        user_text.addKeyListener(this);
        user_text.setEditable(true);
        user_text.setSize(new Dimension(bottom.getWidth()-2-user_txt_label.getWidth(),22));
        user_text.setBorder(null);
        user_text.setBackground(new Color(19,12,10));
        user_text.setForeground(new Color(170,240,170));
        user_text.setBounds(bottom.getInsets().left+user_txt_label.getWidth()+1,bottom.getInsets().top+bottom.getHeight()-5-25+4+2,user_text.getWidth()-1,user_text.getHeight());
        bottom.add(user_text);
        //


        //

        temp_panel = new JPanel(null);
        temp_panel.setBackground(new Color(32,32,32));
        temp_panel.setBounds(right.getInsets().left+1-5,right.getInsets().top+1,right.getWidth()-4+5,80);
        temp_panel.setBorder(BorderFactory.createLineBorder(Color.black));
        //
        port_label = new JLabel("Port Number : ");
        port_label.setOpaque(false);
        port_label.setBackground(Color.darkGray);
        port_label.setForeground(Color.lightGray);
        port_label.setBounds(temp_panel.getInsets().left+4+5,temp_panel.getInsets().top+4,100,30);

        port_input = new JTextField("");
        port_input.setBackground(Color.darkGray);
        port_input.setForeground(Color.lightGray);
        port_input.setFocusable(true);
        port_input.setHorizontalAlignment(JTextField.CENTER);
        port_input.setBounds(temp_panel.getInsets().left+4+port_label.getWidth()-5,temp_panel.getInsets().top+4,60,25);
        port_input.addKeyListener(this);

        accept_server = new MyButton("OK");
        accept_server.setBounds(temp_panel.getInsets().left+temp_panel.getWidth()-104,temp_panel.getInsets().top+4,100,30);
        accept_server.addActionListener(this);

        //


        main.add(left); main.add(right); main.add(bottom);
        this.add(main);
        this.addComponentListener(new ComponentAdapter() {
            @Override
            public void componentResized(ComponentEvent e) {
                //super.componentResized(e);
                //setSize(new Dimension(final_width, getHeight()));
                //super.componentResized(e);




                bottom.setBounds(main.getInsets().left-1,main.getInsets().top+left.getHeight()+2,getWidth()-17+1,getHeight()-400-50+2);
                ct_scroll.setBounds(bottom.getInsets().left+1,bottom.getInsets().top-1,bottom.getWidth()+2,bottom.getHeight()-23);

                user_txt_label.setBounds(bottom.getInsets().left+1,bottom.getInsets().top+bottom.getHeight()-3-25+4,user_txt_label.getWidth(),user_txt_label.getHeight());
                user_text.setBounds(bottom.getInsets().left+user_txt_label.getWidth()+1,bottom.getInsets().top+bottom.getHeight()-5-25+4+2,getWidth(),user_text.getHeight());

                DefaultCaret caret = (DefaultCaret)text.getCaret();
                caret.setUpdatePolicy(DefaultCaret.ALWAYS_UPDATE);

                //main.setBounds(getInsets().left+1,getInsets().top+1,getWidth(),getHeight());


                //repaint();
                //revalidate();

            }
        });

        workers_panel = new MyJPanel();
        workers_panel.setLayout(null);
        workers_panel.setBounds(right.getInsets().left+1,right.getInsets().top+temp_panel.getHeight()+1,right.getWidth()-4,right.getHeight()-3);
        workers_panel.setBackground(new Color(22,22,22));

        timer = new Timer();
        timer.schedule(new dashPhase(),0,80);

        serverNode = new MyNode("server");
        serverNode.setBounds(workers_panel.getInsets().left+workers_panel.getWidth()/2-44,workers_panel.getInsets().top,70,50);
    }

    public void deactivateStart(){
        start.setEnabled(false);
    }
    public void activateStart(){start.setEnabled(true);}
    public void activateStop(){
        stop.setEnabled(true);
    }
    public void deactivateStop(){stop.setEnabled(false);}
    public void activateReccomendations(){
        reccomend.setEnabled(true);
    }

    public void paintPanel(int opt){


        right.removeAll();
        workers_panel.clear_lines();
        workers_panel.removeAll();

        //Server//
        if(opt == 1){



            for(int i=0; i<menu_buttons.size(); i++){
                if(i!=opt){
                    menu_buttons.get(i).select(false);
                }
                else{
                    menu_buttons.get(i).select(true);
                }

            }

            temp_panel.add(port_label);
            temp_panel.add(port_input);
            temp_panel.add(accept_server);
            workers = new ArrayList<ClientWorker>(master.getClients());

            //workers_panel = new MyJPanel();
            workers_panel.add(serverNode);


            int distx = 0;
            int disty = 0;
            for(int i = 0; i<workers.size(); i++){

                MyNode node = new MyNode();
                node.setBounds(workers_panel.getInsets().left+20+15+distx,workers_panel.getInsets().top+10+95+30+disty,58,69);
                infoArea info_area = node.getInfoArea();
                info_area.setBounds(workers_panel.getInsets().left+20+15+distx-20,workers_panel.getInsets().top+10+95+30+disty+70,100,65);
                workers.get(i).setNode(node);

                Line line = new Line(node.getX()+node.getWidth()/2,node.getY()+node.getHeight()+2-20,serverNode.getX()+serverNode.getWidth()/2,serverNode.getY()+10+10);
                workers.get(i).setLine(line);
                workers_panel.add_Line(line);
                workers_panel.add(node);
                workers_panel.add(node.getInfoArea(),0);


                distx+= 58+20+20;

                if(distx>330){
                    distx = 0;
                    disty = 69+20;
                }
            }

            //workers_panel.setLines(lines);


            right.add(temp_panel);
            right.add(workers_panel);

            right.revalidate();
            right.repaint();
            port_input.requestFocus();

        }

    }
    ///

    private void initiateServer(int port) {
        text.append("\n-Initiating Server . . .");
        master.openServer(port);
        if(master.isServer()){
            text.append("\n-Server initiated succesfully on port : "+master.getServerPort()+" !");
            text.append("\n-Listening for Clients . . .");
            accept_server.setEnabled(false);
            //init_server.setEnabled(false);
            port_input.setText(String.valueOf(master.getServerPort()));
            port_input.setEditable(false);
            port_input.setEnabled(false);
            server.setEnabled(true);
            if(master.isData())start.setEnabled(true);
            //redistribute.setEnabled(true);
        }
        else{
            text.append("\nOps!\n"+master.getMessage());
        }
    }

    class dashPhase extends TimerTask {


        public dashPhase(){

        }

        public void run(){
            workers_panel.movePhase();
            workers_panel.revalidate();
            workers_panel.repaint();
        }
    }
    /*MyButton class*/
    class MyButton extends JButton implements MouseListener {

        boolean hovered = false;
        boolean selected = false;

        public MyButton(String title){
            super(title);
            addMouseListener(this);
            setSize(new Dimension(150,30));
            setBackground(Color.darkGray);
            setForeground(Color.lightGray);
            setBorder(BorderFactory.createLineBorder(Color.black));
            setFocusPainted(false);

        }

        public void select(boolean s){
            selected = s;
            if(selected){
                setBackground(new Color(37,161,124));
                setForeground(Color.white);
                setBorder(BorderFactory.createLineBorder(Color.black));
            }
            else{
                setBackground(Color.darkGray);
                setForeground(Color.lightGray);
                setBorder(BorderFactory.createLineBorder(Color.black));
            }
        }

        public void mouseClicked(MouseEvent e)
        {

        }

        public void mouseEntered(MouseEvent e)
        {
            if(isEnabled()){
                setBackground(new Color(0,255,171));
                setForeground(Color.black);
                hovered = true;
            }

        }

        public void mouseExited(MouseEvent e)
        {
            if(isEnabled()) {
                setBackground(Color.darkGray);
                setForeground(Color.lightGray);
                hovered = false;
                if (selected) {
                    select(true);
                }
            }
        }

        public void mousePressed(MouseEvent e)
        {
            if(isEnabled()) {
                setBackground(new Color(37, 161, 124));
                setForeground(Color.white);
            }
        }

        public void mouseReleased(MouseEvent e)
        {
            if(isEnabled()) {
                if (hovered) {
                    setBackground(new Color(0, 255, 171));
                    setForeground(Color.black);
                } else {
                    setBackground(Color.darkGray);
                    setForeground(Color.lightGray);
                    hovered = false;
                }
            }


        }


    }
    /* */






    /*-----------------MAIN-------------------*/
    public static void main(String[] args){
        Application app = new Application(600,600);
    }




    /*ActionListeners*/
    public void actionPerformed(ActionEvent e) {

        if(e.getSource().equals(stop)){
            master.StopTraining();
        }

        if(e.getSource().equals(reccomend)){
            rec = true;

            user_txt_label.setText(" USER :");
            //user_text.setText("ENTER USER : [0 - "+master.getRsizeX()+"]");

            //user_text.setText("ENTER Preferred number of POIS to reccomend : [1 - "+master.getRsizeY()+"+]");
            //input
        }
        //case : initialize Data//
        else if(e.getSource().equals(init_data)){

            //master.readFile("c:/java/dataset.csv");

            JFileChooser jfc = new JFileChooser(FileSystemView.getFileSystemView().getHomeDirectory());
            int returnValue = jfc.showOpenDialog(null);

            if (returnValue == JFileChooser.APPROVE_OPTION) {
                File selectedFile = jfc.getSelectedFile();
                String file_path = selectedFile.getAbsolutePath();
                text.append("\n-Reading data from file : "+file_path+" . . .");

                //reading data from File
                if(master.readFile(file_path)!=0){  // data fail
                    text.append("\nOps!"+master.getMessage());
                }
                else{ // data success
                    text.append(master.getMessage());
                    init_data.setEnabled(false);
                }

            }//*/

            if(master.isData() && master.isServer()){start.setEnabled(true);}
        }
        //

        /*opening server on given port*/
        else if(e.getSource().equals(accept_server)){

            int port;
            try{
                port = Integer.parseInt(port_input.getText());

            }
            catch(Exception ee){
                text.append("\n-Not a valid Port number !");
                port_input.setText("");
                return;
            }
            initiateServer(port);

        }
        //
        else if(e.getSource().equals(server)){

            paintPanel(1);
        }

        else if(e.getSource().equals(start)){
            if(master.getClientsNumber()<2){
                text.append("\n~Workers must be at least : [2].\n    -Found only :"+master.getClientsNumber());
                return;
            }
            startWindow window = new startWindow();
            //master.trainData(); //a , f , l , error //
        }

    }

    public class startWindow extends JFrame{

        JLabel a,f,l,t,er;
        JTextField aa,ff,ll,tt,err;
        JPanel panel;
        JButton ok;
        ArrayList<JTextField> objs;

        int alpha;
        double lambda;
        int fsize;
        int times;
        double error;

        double[] values;

        public startWindow(){
            super();
            setSize(250,198);
            setDefaultCloseOperation(this.DISPOSE_ON_CLOSE);

            objs = new ArrayList<JTextField>();
            values = new double[5];
            initGUI();
            setVisible(true);
            setResizable(false);
        }


        void initGUI(){


            panel = new JPanel(null);
            panel.setBounds(this.getInsets().left,this.getInsets().top,this.getWidth(),this.getHeight());
            Insets insets = panel.getInsets();

            a = new JLabel(" alpha : ");
            a.setOpaque(true);
            a.setBackground(Color.darkGray);
            a.setForeground(Color.white);
            a.setBounds(insets.left+1,insets.top+1,70,23);

            aa = new JTextField("40");
            aa.setBackground(Color.lightGray);
            aa.setForeground(Color.black);
            aa.setBounds(insets.left+1+100+1-20,insets.top+1,70,23);
            objs.add(aa);

            l = new JLabel(" lamba : ");
            l.setOpaque(true);
            l.setBackground(Color.darkGray);
            l.setForeground(Color.white);
            l.setBounds(insets.left+1,insets.top+1+25,70,23);

            ll = new JTextField("0.1");
            ll.setBackground(Color.lightGray);
            ll.setForeground(Color.black);
            ll.setBounds(insets.left+1+100+1-20,insets.top+1+25,70,23);
            objs.add(ll);

            f = new JLabel(" f[size] : ");
            f.setOpaque(true);
            f.setBackground(Color.darkGray);
            f.setForeground(Color.white);
            f.setBounds(insets.left+1,insets.top+1+25+25,70,23);

            ff = new JTextField("30");
            ff.setBackground(Color.lightGray);
            ff.setForeground(Color.black);
            ff.setBounds(insets.left+1+100+1-20,insets.top+1+25+25,70,23);
            objs.add(ff);

            t = new JLabel(" iterations : ");
            t.setOpaque(true);
            t.setBackground(Color.darkGray);
            t.setForeground(Color.white);
            t.setBounds(insets.left+1,insets.top+1+25+25+25,70,23);

            tt = new JTextField("10");
            tt.setBackground(Color.lightGray);
            tt.setForeground(Color.black);
            tt.setBounds(insets.left+1+100+1-20,insets.top+1+25+25+25,70,23);
            objs.add(tt);

            er = new JLabel(" max error : ");
            er.setOpaque(true);
            er.setBackground(Color.darkGray);
            er.setForeground(Color.white);
            er.setBounds(insets.left+1,insets.top+1+25+25+25+25,70,23);

            err = new JTextField("0.5");
            err.setBackground(Color.lightGray);
            err.setForeground(Color.black);
            err.setBounds(insets.left+1+100+1-20,insets.top+1+25+25+25+25,70,23);
            objs.add(err);

            ok = new JButton("Accept");
            ok.setBackground(Color.lightGray);
            ok.setForeground(Color.darkGray);
            ok.setBorder(BorderFactory.createLineBorder(Color.darkGray));
            ok.addActionListener(new ActionListener() {
                public void actionPerformed(ActionEvent e) {
                    if(e.getSource().equals(ok)){
                        if(checkValues()){

                            dispose();

                            master.trainData(values[0],values[1],(int)values[2],(int)values[3],values[4]);
                        }
                        else{
                            return;
                        }
                    }
                }
            });
            ok.setBounds(insets.left+1+100+1-20+50+20+10,insets.top+1+25+25+25+25+25+5,70,23);


            panel.add(a); panel.add(aa);
            panel.add(l); panel.add(ll);
            panel.add(f); panel.add(ff);
            panel.add(t); panel.add(tt);
            panel.add(er); panel.add(err);
            panel.add(ok);
            add(panel);
        }

        boolean checkValues(){

            for(int i=0; i<objs.size(); i++){
                try {
                    double d = Double.parseDouble(objs.get(i).getText());
                    values[i] = d;
                }
                catch(Exception e){
                    JOptionPane.showMessageDialog(null, "Invalid value(s)!");
                    return false;
                }
            }
            return true;
        }

    }


    void parseLine(String code){

        String er ="\n~Not a valid command.\n";
        boolean distri = false;
        boolean trainer = false;
        boolean synch = false;

        boolean error = false;
        boolean iterations = false;

        boolean set = false;
        boolean treshold = false;
        boolean priority = false;
        boolean state = false;

        if(rec){
            try{
                int val = Integer.parseInt(user_text.getText());
                if(val<0 || val>=master.getRsizeX()){
                    text.append("\n~USER[u] must be >= 0 && <"+master.getRsizeX()+"\n");
                    user_text.setText(""); rec = false; user_txt_label.setText("/Command>"); return;
                }
                master.Reccomend(val);
                user_text.setText("");
                rec = false;
                user_txt_label.setText("/Command>");
                return;
            }
            catch(Exception e){text.append("\n~USER[u] must be >= 0 && <"+master.getRsizeX()+"\n"); user_text.setText(""); rec = false; user_txt_label.setText("/Command>"); return;}
        }

        String[] args = code.split(" ");
        if(args.length<=1){
            text.append(er);
            user_text.setText("");
            return;
        }

        String var = args[0];

        if("reccomend".contains(var.toLowerCase())){
            if(args[1].toLowerCase().equals("set")){

                try{
                    master.setRecSize(Integer.parseInt(args[2]));
                    text.append("\n~Reccomendation List's size se to : "+args[2]);
                }
                catch (Exception e){text.append(er);user_text.setText(""); return;}
            }
        }
        else if("distribute".contains(var.toLowerCase())){
            distri = true;
            if(args[1].toLowerCase().equals("set")){

                try{
                    master.setDistri(Integer.parseInt(args[2]));
                }
                catch(Exception e){
                    text.append(er);
                    user_text.setText("");
                    return;
                }
            }
            else if("treshold".contains(args[1].toLowerCase()) || "priority".contains(args[1].toLowerCase())){
                if(args[2].toLowerCase().equals("set")){

                    double val = 0;
                    try{
                        val = Double.parseDouble(args[3]);
                    }
                    catch(Exception e){
                        text.append(er);
                        user_text.setText("");
                        return;
                    }
                    if("treshold".contains(args[1].toLowerCase())){master.setDistri_treshold(val);}
                    else{master.setDistri_priority((int)val);}
                }
            }
            else{
                text.append(er);
                user_text.setText("");
                return;
            }
        }
        else if("synchronize".contains(var.toLowerCase())){
            if(args[1].toLowerCase().equals("set")){
                try{
                    int val = Integer.parseInt(args[2]);
                    master.setSynch(val);
                }
                catch (Exception e){
                    text.append(er); user_text.setText(""); return;
                }
            }
        }
        else if("trainer".contains(var.toLowerCase())){
            trainer = true;
            if(args.length==2){
                text.append(er); user_text.setText("");
                return;

            }
            else{
                if("error".contains(args[1].toLowerCase()) || "iterations".contains(args[1].toLowerCase())){
                    if(args[2].toLowerCase().equals("set")){
                        double val = 0;
                        try{
                            val = Double.parseDouble(args[3]);
                        }
                        catch(Exception e){ text.append(er);user_text.setText(""); return;
                    }

                    if("error".contains(args[1].toLowerCase())) {master.setError(val); }
                    else{master.setIterations((int)val);}

                    }
                    else{
                        text.append(er);user_text.setText(""); return;
                    }
                }
            }
        }
        else{
            text.append(er);
            user_text.setText("");
            return;
        }
        user_text.setText("");
    }

    /*keyListeners*/
    public void keyTyped(KeyEvent e) {

    }
    public void keyPressed(KeyEvent e)
    {
        if(e.getSource().equals(user_text)){
            if(e.getKeyCode()==KeyEvent.VK_ENTER){

                String code = user_text.getText();
                parseLine(code);

                //text.append("\n\n"+user_txt_label.getText()+user_text.getText());
                //user_text.setText("");
            }
        }
        else if(e.getSource().equals(port_input) && e.getKeyCode()== KeyEvent.VK_ENTER){
            int port;
            try{
                port = Integer.parseInt(port_input.getText());
            }
            catch(Exception ee){
                text.append("\n-Not a valid Port number !");
                port_input.setText("");
                return;
            }
            initiateServer(port);
        }
    }
    public void keyReleased(KeyEvent e) {

    }
    //


    /*windowListeners*/
    public void windowOpened(WindowEvent e) {

    }
    public void windowClosing(WindowEvent e) {
        if(e.getSource().equals(this)){
            master.exit();
        }
    }
    public void windowClosed(WindowEvent e) {

    }
    public void windowIconified(WindowEvent e) {

    }
    public void windowDeiconified(WindowEvent e) {

    }
    public void windowActivated(WindowEvent e) {

    }
    public void windowDeactivated(WindowEvent e) {

    }
    //


    public JTextArea getPipe(){
        return this.text;
    }

    public MyNode getServerNode(){
        return this.serverNode;
    }



}
