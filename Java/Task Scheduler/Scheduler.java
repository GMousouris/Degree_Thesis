import javax.swing.*;
import javax.swing.Timer;
import javax.swing.event.MouseInputListener;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.lang.reflect.Array;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.text.SimpleDateFormat;
import java.util.*;


public class Scheduler extends JFrame implements ActionListener,MouseListener,MouseInputListener,KeyListener{

    private JPanel mainPanel;
    private JPanel leftPanel;
    private JPanel righPanel; private JPanel tasks,projects,exams;



    private JButton addLesson,addLessonOk,addTask,addProject,addExams;
    private JButton removeLesson;

    private JTextField addLessonText;
    private JFrame frame;

    private ArrayList<Lesson> lessons;
    private ArrayList<JPanel> panels;
    private ArrayList<JLabel> tabs;


    private int activeLesson = -1;
    private int entry_choice = -1;

    private JTextField entryTitle;
    private JTextArea entryDescription;
    private JTextField entryDeadline;
    private JButton entryOK,entryOK2;
    private JLabel l1,l12,l2;
    private JScrollPane scroll;

    private infoPanel infoPanel;
    private boolean infoPanelUp = false;




    private int width = 617;
    private int height = 400-30;

    private Timer timer;
    private Boolean timerRunning = false;

    private boolean entryisActive = false;
    private myButton activeEntry = null;

    private JLabel l122,l1222,l22;
    private JTextField entryTitle2,entryDeadline2;
    private JTextArea entryDescription2;
    private JScrollPane scroll2;

    private JScrollPane tasksJ,projectsJ,examsJ;
    private ArrayList<JScrollPane> panelsJ;
    private JButton Done;

    private int currentPanel = -1;
    private int currentSize = 0;

    private int tasksS = 0;
    private int projectsS = 0;
    private int examsS = 0;

    private ArrayList<ArrayList<ArrayList<myButton>>> data = new ArrayList<ArrayList<ArrayList<myButton>>>();

    private myButton pressedEntry = null;

    private boolean pressed = false;

    private int[] sizes = {0,0,0};

    private int x = 0; private int y = 0;
    private myButton tempEntry = null;
    private JComponent parent = null;
    private Container container = null;
    private int state = 0; private int counter = 0;
    private myButton2 trashcan;

    private JTextField renameField;

    private JMenuItem rename,delete;

    private JTextField infoField; private JTextField infoField2; private int infoCounter = 0;

    private Timer timer2,timer3;

    private boolean faderState = false;

    private ArrayList<String> filesToDelete = new ArrayList<String>();

    private myButton3 expand;
    private JButton calendar;

    private ArrayList<int[]> dates = new ArrayList<int[]>();

    public Scheduler(){


        super("Scheduler");
        setSize(new Dimension(width,height));
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setVisible(true);

        addWindowListener( new WindowAdapter()
        {
            public void windowClosing(WindowEvent e)
            {
                try{

                    Path currentRelativePath = Paths.get("");
                    String s = currentRelativePath.toAbsolutePath().toString();
                    s += "/Lessons/delete.txt";


                    //PrintWriter pw = new PrintWriter(s);
                    //pw.close();


                    BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(s,false),"UTF-8"));

                    for(String entry : filesToDelete){

                        writer.write(entry);
                        writer.newLine();
                    }

                    writer.close();

                }
                catch(Exception ee){}
            }
        });



        panels = new ArrayList<JPanel>();

        rename = new JMenuItem("rename");
        rename.addActionListener(this);
        delete = new JMenuItem("delete");
        delete.addActionListener(this);



        renameField = new JTextField("Enter a name...");
        renameField.setBackground(Color.darkGray);
        renameField.setForeground(Color.white);
        renameField.addKeyListener(this);
        renameField.setCaretColor(Color.lightGray);
        renameField.addMouseListener(this);
        renameField.setHorizontalAlignment(JTextField.CENTER);

        l1 = new JLabel("Title :");
        l12 = new JLabel("Deadline :");
        l2 = new JLabel("Description :");
        scroll = new JScrollPane();

        entryDescription = new JTextArea();
        entryTitle = new JTextField();
        entryOK = new JButton("OK");
        entryOK.setSize(60,25);
        entryOK.addActionListener(this);

        entryOK2 = new JButton("OK");
        entryOK2.setSize(60,25);
        entryOK2.addActionListener(this);

        entryDeadline = new JTextField();

        addLessonText = new JTextField();
        addLessonOk = new JButton("OK");
        addLessonOk.addActionListener(this);

        Done = new JButton();
        Done.addActionListener(this);


        mainPanel = new JPanel(null);
        mainPanel.setBounds(getInsets().left,getInsets().top,this.getWidth(),this.getHeight());
        mainPanel.setBackground(new Color(64,64,64));
        add(mainPanel);
        Insets insets = mainPanel.getInsets();


        //mainPanel.add(infoPanel,0);



        int dist = 2;
        int offset_y = 27;

        leftPanel = new JPanel(null);
        leftPanel.setSize(new Dimension(150,800));
        leftPanel.setBounds(insets.left,insets.top+offset_y,leftPanel.getWidth(),leftPanel.getHeight()-offset_y);
        leftPanel.setBackground(new Color(80,80,80));
        mainPanel.add(leftPanel);

        righPanel = new JPanel(null);
        righPanel.setSize(new Dimension(getWidth()-leftPanel.getWidth()-dist,800));
        righPanel.setBounds(insets.left+leftPanel.getWidth()+dist-1,insets.top+offset_y,righPanel.getWidth()-18,righPanel.getHeight()-offset_y);
        righPanel.setBackground(new Color(72,72,72));

        infoField = new JTextField(" /Info_Bot > ");
        infoField.setBackground(Color.darkGray);
        infoField.setForeground(new Color(0,167,115));
        infoField.setFont(infoField.getFont().deriveFont(14.0f));
        infoField.setBorder(null);
        infoField.setBounds(righPanel.getInsets().left+1,righPanel.getInsets().top+280+4,75,20);

        infoField2 = new JTextField("");
        infoField2.setBackground(Color.darkGray);
        infoField2.setForeground(new Color(0,167,115));
        infoField2.setFont(infoField.getFont().deriveFont(14.0f));
        infoField2.setBorder(null);
        infoField2.setBounds(righPanel.getInsets().left+1+infoField.getWidth(),righPanel.getInsets().top+280+4,righPanel.getWidth()-1,20);

        timer3 = new Timer(800, new ActionListener(){

            public void actionPerformed(ActionEvent ae){

                infoField2.setText(infoField2.getText()+" . ");
                infoCounter++;
                if(infoCounter == 5){
                    infoCounter = 0;
                    infoField2.setText("");
                }

                //infoField.revalidate(); infoField.repaint();
            }
        }); timer3.start();

        righPanel.add(infoField);
        righPanel.add(infoField2);

        mainPanel.add(righPanel);


        //----------------//



        Insets r_insets = righPanel.getInsets();


        tasks = new JPanel(null);
        projects = new JPanel(null);
        exams = new JPanel(null);

        tasksJ = new JScrollPane(tasks);
        projectsJ = new JScrollPane(projects);
        examsJ = new JScrollPane(exams);

        panelsJ = new ArrayList<JScrollPane>();
        panelsJ.add(tasksJ); panelsJ.add(projectsJ); panelsJ.add(examsJ);

        panels.add(tasks);
        panels.add(projects);
        panels.add(exams);

        int distx = 0;
        int c = 0;
        int size = 0;

        for(int i = 0; i<3; i++){



           // panels.get(i).setSize(new Dimension(149,getHeight()));
            //p.setBounds(r_insets.left+distx,r_insets.top,p.getWidth(),p.getHeight()-100);
            panels.get(i).setBackground(new Color(44,44,44));
            //p.setBorder(BorderFactory.createLineBorder(Color.black));
            panelsJ.get(i).setBounds(r_insets.left+distx,r_insets.top,149,280);
            panelsJ.get(i).setBorder(null);
            panelsJ.get(i).getVerticalScrollBar().setPreferredSize (new Dimension(0,0));
            panelsJ.get(i).getVerticalScrollBar().setUnitIncrement(8);
            //panelsJ.get(i).setSize(new Dimension(149,getHeight()));

            panels.get(i).setPreferredSize(new Dimension(panels.get(i).getWidth(),51*sizes[i]));
            panelsJ.get(i).setViewportView(panels.get(i));

            righPanel.add(panelsJ.get(i));
            distx += 150;
            c++;
        }

        //----------------//



        //----------------//
        tabs = new ArrayList<JLabel>();
        tabs.add(new JLabel("Lessons"));
        tabs.add(new JLabel("Tasks"));
        tabs.add(new JLabel("Projects"));
        tabs.add(new JLabel("Exams"));

        distx = 0;
        int count = 0;



        for(JLabel t:tabs){

            t.setBackground(null);
            t.setForeground(Color.lightGray);
            t.setOpaque(false);
            t.setHorizontalAlignment(SwingConstants.CENTER);
            t.setVerticalAlignment(SwingConstants.CENTER);
            t.setBorder(BorderFactory.createLineBorder(Color.black));


            t.setBounds(insets.left+distx,insets.top+1,150,25);
            mainPanel.add(t);
            distx+=150;
        }


        expand = new myButton3();
        expand.addActionListener(this);
        expand.setBounds(insets.left+574,insets.top+1,expand.getWidth(),expand.getHeight());
        mainPanel.add(expand,0);


        calendar = new JButton("Calendar");
        calendar.setBackground(Color.darkGray);
        calendar.setForeground(Color.lightGray);
        calendar.setSize(new Dimension(110,25));
        calendar.setBounds(insets.left+600,insets.top,calendar.getWidth(),calendar.getHeight());
        calendar.setBorder(BorderFactory.createLineBorder(new Color(32,32,32)));
        calendar.setFocusPainted(false);
        calendar.addActionListener(this);
        calendar.addMouseListener(this);
        mainPanel.add(calendar);

        //----------------//

        trashcan = new myButton2();
        trashcan.setBounds(insets.left+width-80,insets.top+height-115,trashcan.getWidth(),trashcan.getHeight());
        trashcan.setVisible(false);
        mainPanel.add(trashcan,0);


        addLesson = new JButton("");
        ImageIcon icon = new ImageIcon("Images/addButton.png");
        addLesson.setIcon(icon);
        addLesson.setSize(new Dimension(25,25));
        addLesson.setBounds(insets.left,insets.top+1,addLesson.getWidth(),addLesson.getHeight());
        addLesson.addActionListener(this);
        addLesson.setBackground(new Color(84,84,84));
        //addLesson.setBorderPainted(false);
        //addLesson.setFocusPainted(false);
        mainPanel.add(addLesson);

        addTask = new JButton("");
        addTask.setIcon(icon);
        addTask.setBackground(new Color(84,84,84));
        addTask.setBounds(insets.left+149,insets.top+1,addLesson.getWidth(),addLesson.getHeight());
        addTask.addActionListener(this);
        addTask.setSize(new Dimension(25,25));
        mainPanel.add(addTask);

        addProject = new JButton("");
        addProject.setIcon(icon);
        addProject.setBackground(new Color(84,84,84));
        addProject.setBounds(insets.left+149+149,insets.top+1,addLesson.getWidth(),addLesson.getHeight());
        addProject.addActionListener(this);
        addProject.setSize(new Dimension(25,25));
        mainPanel.add(addProject);

        addExams = new JButton("");
        addExams.setIcon(icon);
        addExams.setBackground(new Color(84,84,84));
        addExams.setBounds(insets.left+149+149+149,insets.top+1,addLesson.getWidth(),addLesson.getHeight());
        addExams.addActionListener(this);
        addExams.setSize(new Dimension(25,25));
        mainPanel.add(addExams);

        infoPanel = new infoPanel();
        infoPanel.setBounds(insets.left+150,insets.top+347-40,getWidth()-150-17,379-100-50-10-7-2);
        infoPanel.setBorder(BorderFactory.createLineBorder(new Color(32,32,32)));
        mainPanel.add(infoPanel,0);
        infoPanel.setVisible(false);
        //----------------//
        loadData();

        //
        setResizable(false);
        revalidate();
        repaint();
    }

    public void addNewEntry(String title,String desc,String deadline,int ch){

        lessons.get(activeLesson).addEntry(title,desc,deadline,ch);
        data.get(activeLesson).set(ch-1,new ArrayList<myButton>(lessons.get(activeLesson).getArray(ch)));
        myButton entry = data.get(activeLesson).get(ch-1).get(data.get(activeLesson).get(ch-1).size()-1);
        entry.addActionListener(this);
        entry.addMouseListener(this);
        entry.addMouseMotionListener(this);

        lessons.get(activeLesson).save();
        updateAreas(activeLesson);
    }

    public void loadData(){

        Path currentRelativePath = Paths.get("");
        String s = currentRelativePath.toAbsolutePath().toString();
        s += "/Lessons/";


        BufferedReader br = null;

        File file2 = new File(s+"delete.txt");
        try{
            br = new BufferedReader(
                    new InputStreamReader(
                            new FileInputStream(file2), "UTF8"));

            String line = "";
            while((line = br.readLine())!= null){
                filesToDelete.add(line);
            }
            br.close();

        }catch(Exception e){}


        try{
            PrintWriter pw = new PrintWriter(new File(s+"delete.txt"));
            pw.close();
        }catch(Exception e){}




        lessons = new ArrayList<Lesson>();
        File folder = new File(s);

        for(File file : folder.listFiles() ){

            if (filesToDelete.contains(file.getName())) {
                try{
                    deleteDirectory(file);
                }catch (Exception e){}
                continue;
            }
            if(file.getName().equals("delete.txt")){ continue;}

            Lesson l = new Lesson(file.getName(),false);
            l.addMouseListener(new PopClickListener());
            l.addMouseListener(this);
            lessons.add(l);
            l.load();
            ArrayList<ArrayList<myButton>> temp = new ArrayList<ArrayList<myButton>>(l.getData());
            data.add(temp);
        }

        filesToDelete.clear();

        for(ArrayList<ArrayList<myButton>> list : data){
            for(ArrayList<myButton> entriesList : list){
                for(myButton entry : entriesList){

                    entry.setText(entry.getTitle());
                    entry.addMouseMotionListener(this);
                    entry.addMouseListener(this);
                    entry.addActionListener(this);

                }
            }
        }


        updateGUI();

    }

    void deleteDirectory(File file) throws IOException {
        if (file.isDirectory()) {
            File[] entries = file.listFiles();
            if (entries != null) {
                for (File entry : entries) {
                    deleteDirectory(entry);
                }
            }
        }
        if (!file.delete()) {
            throw new IOException("Failed to delete " + file);
        }
    }

    public void updateAreas(int pointer){



        sizes[0] = 0; sizes[1] = 0; sizes[2] = 0;

        for(int i = 0; i<panels.size(); i++){

            panels.get(i).removeAll();
            int disty = 1;


            for(myButton entry : data.get(pointer).get(i)){

                //617 x 400
                Insets ins = panels.get(i).getInsets();
                //entry.setSize(new Dimension(getWidth()-217-255,getHeight()-350));
                entry.setBounds(ins.left+1,ins.top+disty,entry.getWidth(),entry.getHeight()); //145 x  50
                panels.get(i).add(entry);
                disty += entry.getHeight()+1;
                //entries.add(entry);
                sizes[i]++;

            }
            panels.get(i).setPreferredSize(new Dimension(panels.get(i).getWidth(),51*sizes[i]));
            panels.get(i).revalidate();
            panels.get(i).repaint();
        }


    }

    public void ChangePos(myButton entry,int c){

        int x = entry.getX();
        int y = entry.getY();


        entry.setAdjusting(true);
        AnimationHandler animHandler = new AnimationHandler(entry,pressedEntry.getBound().x,pressedEntry.getBound().y);
        animHandler.start();

        //entry.setBounds(pressedEntry.getBound().x,pressedEntry.getBound().y,entry.getWidth(),entry.getHeight());
        pressedEntry.setBound(new Rectangle(x,y,pressedEntry.getWidth(),pressedEntry.getHeight()));
        //entry.revalidate();
        //entry.repaint();


        int p = 0;
        for(myButton b : data.get(activeLesson).get(currentPanel)){
            if(b.equals(pressedEntry)){
                break;
            }
            p++;
        }

        //System.out.println("pressed : "+p+"\ntraded : "+c);
        Collections.swap(data.get(activeLesson).get(currentPanel),p,c);

    }


    public void refreshEntry(myButton entry){

    }

    public void updateGUI(){

        leftPanel.removeAll();

        Insets l_insets = leftPanel.getInsets();
        int disty = 1;

        for(Lesson l : lessons){
            l.setBounds(l_insets.left+2,l_insets.top+disty,l.getWidth(),l.getHeight());
            leftPanel.add(l);
            disty += l.getHeight()+1;
        }

        leftPanel.revalidate();
        leftPanel.repaint();

    }



    //------------------------//

    class PopUpDemo extends JPopupMenu {
        JMenuItem anItem;
        public PopUpDemo(){
            add(rename);
            add(delete);
        }
    }

    class PopClickListener extends MouseAdapter {
        public void mousePressed(MouseEvent e){
            if (e.isPopupTrigger()) {
                for(Lesson l : lessons){
                    if(l.equals(e.getSource())){
                        if(l.isFocused()){
                            doPop(e);
                        }
                        else{
                            break;
                        }
                    }
                }
                //doPop(e);
            }
        }

        public void mouseReleased(MouseEvent e){
            if (e.isPopupTrigger())
                for(Lesson l : lessons){
                    if(l.equals(e.getSource())){
                        if(l.isFocused()){
                            doPop(e);
                        }
                        else{
                            break;
                        }
                    }
                }
        }

        private void doPop(MouseEvent e){
            PopUpDemo menu = new PopUpDemo();
            menu.show(e.getComponent(), e.getX(), e.getY());
        }
    }

    class infoPanel extends JPanel {



        public infoPanel(){



            super();
            this.setLayout(null);
            this.setSize(new Dimension(getWidth(),379-100-2));
            this.setBackground(new Color(70,70,82));
            //this.setBackground(Color.red);

            Insets ins = this.getInsets();

            l122 = new JLabel("Title : ");
            l122.setBounds(ins.left+5,ins.top+10,40,25);
            l122.setForeground(Color.white);

            entryTitle2 = new JTextField();
            entryTitle2.setFont(entryTitle2.getFont().deriveFont(16.0f));
            entryTitle2.setBounds(ins.left+5+60+2-25,ins.top+10,200+137-2,25);
            entryTitle2.setBackground(Color.darkGray);
            entryTitle2.setForeground(Color.white);
            entryTitle2.setBorder(BorderFactory.createBevelBorder(1));
            entryTitle2.setCaretColor(Color.lightGray);
            this.add(l122);
            this.add(entryTitle2);

            l1222 = new JLabel("DeadLine : ");
            l1222.setBounds(ins.left+5,ins.top+10+25+5,70,25);
            l1222.setForeground(Color.white);

            entryDeadline2 = new JTextField();
            entryDeadline2.setFont(new Font("default",Font.BOLD,14));
            entryDeadline2.setHorizontalAlignment(JTextField.CENTER);
            entryDeadline2.setBounds(ins.left+5+60+8,ins.top+10+25+5,100,25);
            entryDeadline2.setBackground(Color.darkGray);
            entryDeadline2.setForeground(Color.white);

            //entryDeadline.setBorder(BorderFactory.createLineBorder(Color.lightGray));
            entryDeadline2.setBorder(BorderFactory.createBevelBorder(0));
            entryDeadline2.setCaretColor(Color.lightGray);
            this.add(l1222);
            this.add(entryDeadline2);


            entryDescription2 = new JTextArea();
            entryDescription2.setFont(entryDescription2.getFont().deriveFont(16.0f));

            entryDescription2.setBackground(new Color(244, 212, 137));
            entryDescription2.setForeground(Color.black);
            scroll2 = new JScrollPane(entryDescription2);
            scroll2.setBorder(BorderFactory.createLineBorder(Color.lightGray));
            scroll2.setBounds(ins.left+5,ins.top+10+25+10+25,200+174+40+20+7,100);
            this.add(scroll2);

            //entryOK2 = new JButton("OK");
            entryOK2.setText("SAVE");
            entryOK2.setBounds(ins.left+5,ins.top+310-100-30-5,80,25);
            this.add(entryOK2);


        }
    }

    class AnimationHandler extends Thread{

        private myButton entry = null;
        private int x,y;

        public AnimationHandler(myButton entry, int x , int y){
            this.entry = entry;
            this.x = x;
            this.y = y;
        }

        public void run(){
            while(entry.getY()!=y){
                if(entry.getY()>y){
                    entry.setBounds(entry.getX(),entry.getY()-1,entry.getWidth(),entry.getHeight());
                }
                else{
                    entry.setBounds(entry.getX(),entry.getY()+1,entry.getWidth(),entry.getHeight());
                }
                mainPanel.revalidate();
                mainPanel.repaint();
                try{
                    Thread.sleep(2);
                }catch (Exception e){}
            }
            entry.setAdjusting(false);
        }
    }

    class TextFader extends Thread{

        private boolean state = true;
        private int r = Color.darkGray.getRed();
        private int g = Color.darkGray.getGreen();
        private int b = Color.darkGray.getBlue();


        public TextFader(){
            faderState = true;
        }

        public void run(){
            try{
                Thread.sleep(2000);
            }catch (Exception e){}

            while(faderState){

                int r2 = infoField2.getForeground().getRed();
                int g2 = infoField2.getForeground().getGreen();
                int b2 = infoField2.getForeground().getBlue();

                if(r2 < r ){
                    r2++;
                }
                else if(r2 > r){
                    r2--;
                }
                if(g2 < g){
                    g2++;
                }
                else if(g2 > g){
                    g2--;
                }
                if(b2 < b){
                    b2++;
                }
                else if(b2 > b){
                    b2--;
                }

                infoField2.setForeground(new Color(r2,g2,b2));

                if(r2 == r && g2 == g && b2 == b){
                    break;
                }
                try{
                    Thread.sleep(8);
                }catch(Exception e){}
            }
            infoField2.setText("");
            infoField2.setForeground(new Color(0,167,115));
            timer3.start();
            faderState = false;
            //System.out.println("A");
        }
    }

    //------------------------//

    public void popUpWindow(){

        //617 x 400
        frame = new JFrame("Creating new Entry");
        frame.setSize(400,379);  //400 x 379
        frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        frame.setVisible(true);
        frame.requestFocus();

        Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
        frame.setLocation(dim.width/2-frame.getSize().width/2, dim.height/2-frame.getSize().height/2);

        JPanel panel = new JPanel(null);
        panel.setBackground(new Color(84,84,84));
        Insets ins = panel.getInsets();

        //JLabel l1 = new JLabel("Title : ");
        l1.setBounds(ins.left+5,ins.top+10,40,25);
        l1.setForeground(Color.lightGray);

        entryTitle.setBounds(ins.left+5+60+2-25,ins.top+10,200+137-2,25);
        entryTitle.setBackground(Color.darkGray);
        entryTitle.setForeground(Color.lightGray);
        entryTitle.setBorder(BorderFactory.createBevelBorder(1));
        entryTitle.setCaretColor(Color.lightGray);
        panel.add(l1);
        panel.add(entryTitle);

        //JLabel l12 = new JLabel("DeadLine : ");
        l12.setBounds(ins.left+5,ins.top+10+25+10,70,25);
        l12.setForeground(Color.lightGray);

        entryDeadline.setBounds(ins.left+5+60+8,ins.top+10+25+10,100,25);
        entryDeadline.setBackground(Color.darkGray);
        entryDeadline.setForeground(Color.lightGray);
        //entryDeadline.setBorder(BorderFactory.createLineBorder(Color.lightGray));
        entryDeadline.setBorder(BorderFactory.createBevelBorder(0));
        entryDeadline.setCaretColor(Color.lightGray);
        panel.add(l12);
        panel.add(entryDeadline);

        //JLabel l2 = new JLabel("Description : ");
        l2.setBounds(ins.left+5,ins.top+10+25+10+30,100,25);
        l2.setForeground(Color.lightGray);

        entryDescription.setBackground(new Color(244, 212, 137));
        entryDescription.setForeground(Color.black);
        JScrollPane scroll = new JScrollPane(entryDescription);
        scroll.setBorder(BorderFactory.createLineBorder(Color.lightGray));

        scroll.setBounds(ins.left+5,ins.top+10+25+10+25+2+30,200+174,200);
        panel.add(l2);
        panel.add(scroll);

        entryOK.setBounds(ins.left+318,ins.top+310,60,25);
        panel.add(entryOK);

        frame.add(panel);
    }

    public boolean checkErrors(String title, String deadline){

        if(title.equals("")){
            return false;
        }
        if(deadline.toLowerCase().matches(".*[a-z].*")){
            return false;
        }
        else{
            int count = 0;
            for(int i = 0; i<deadline.length(); i++){
                if(deadline.charAt(i) == '/'){
                    count++;
                }
            }
            if(count != 2){
                return false;
            }
            String[] tokens = deadline.split(("/"));
            int day = Integer.parseInt(tokens[0]);
            int month = Integer.parseInt(tokens[1]);
            int year = Integer.parseInt(tokens[2]);

            if(month<1 || month > 12){
                return false;
            }

            Calendar cal = Calendar.getInstance();
            cal.set(Calendar.YEAR, year);
            cal.set(Calendar.MONTH,month);
            if(day > cal.getActualMaximum(Calendar.DATE)){
                return false;
            }
        }

        return true;
    }

    public void processEntryEvent(myButton entry){



        //case 1 : entry opens - panels close //
        if(entry.isActive()){


            //infoPanel.setBounds(mainPanel.getInsets().left+150,mainPanel.getInsets().top+getHeight()-100+40,getWidth()-150,0);

            timer = new Timer(1, new ActionListener(){

                public void actionPerformed(ActionEvent ae){

                    int y = getHeight();
                    if(y < 592-10 && entryisActive){
                        setSize(new Dimension(getWidth(),getHeight()+4));
                        //infoPanel.setBounds(mainPanel.getInsets().left+150,infoPanel.getY()-4,getWidth()-150,infoPanel.getHeight()+4);
                    }
                    else{
                        setSize(new Dimension(getWidth(),592-10));
                        infoField.setLocation(infoField.getBounds().x,infoField.getBounds().y+infoPanel.getHeight());
                        infoField2.setLocation(infoField2.getBounds().x,infoField2.getBounds().y+infoPanel.getHeight());
                        timerRunning = false;
                        //entry.setEnabled(true);
                        infoPanel.setVisible(true);
                        timer.stop();
                    }

                    mainPanel.revalidate();
                    mainPanel.repaint();



                }
            });
            timer.start();
            timerRunning = true;
            //entry.setEnabled(false);

        }
        else{

            timer = new Timer(1, new ActionListener(){

                public void actionPerformed(ActionEvent ae){

                    int y = getHeight();
                    if(y > 400-30 && !entryisActive){
                        setSize(new Dimension(getWidth(),getHeight()-4));
                    }
                    else{
                        setSize(new Dimension(getWidth(),400-30));
                        infoField.setBounds(righPanel.getInsets().left+1,righPanel.getInsets().top+280+4,75,20);
                        infoField2.setBounds(righPanel.getInsets().left+1+infoField.getWidth(),righPanel.getInsets().top+280+4,righPanel.getWidth()-1,20);
                        timerRunning = false;
                        infoPanel.setVisible(false);
                        timer.stop();
                    }

                    mainPanel.revalidate();
                    mainPanel.repaint();


                }
            });
            timer.start();
            timerRunning = true;


        }

        revalidate();
        repaint();

    }

    public void updateEntryData(myButton entry){
        entryTitle2.setText(entry.getTitle());
        entryDeadline2.setText(entry.getDeadline());
        entryDescription2.setText(entry.getDesc());
    }

    public void removeEntry(myButton entry2){

        int parentt;
        if(container.equals(tasks)){
            parentt = 0;
        }
        else if(container.equals(projects)){
            parentt = 1;
        }
        else{
            parentt = 2;
        }

        for(ArrayList<myButton> list : data.get(activeLesson)){
            for(myButton entry : list){

                if(entry.equals(pressedEntry)){

                    list.remove(entry);
                    lessons.get(activeLesson).updateEntry(data.get(activeLesson).get(parentt),parentt);
                    break;
                }
            }
        }

        lessons.get(activeLesson).save();
        updateAreas(activeLesson);

    }

    public void expandFrame(){
        if(!expand.isExpanded()){


            //infoPanel.setBounds(mainPanel.getInsets().left+150,mainPanel.getInsets().top+getHeight()-100+40,getWidth()-150,0);

            timer = new Timer(1, new ActionListener(){

                public void actionPerformed(ActionEvent ae){

                    int x = getWidth();
                    if(x < 617+150){
                        setSize(new Dimension(getWidth()+4,getHeight()));
                        //infoPanel.setBounds(mainPanel.getInsets().left+150,infoPanel.getY()-4,getWidth()-150,infoPanel.getHeight()+4);
                    }
                    else{
                        setSize(new Dimension(617 + 150,getHeight()));
                        expand.setEnabled(true);
                        timer.stop();
                    }

                    mainPanel.revalidate();
                    mainPanel.repaint();



                }
            });
            timer.start();
            expand.setEnabled(false);
            //entry.setEnabled(false);

        }
        else{

            timer = new Timer(1, new ActionListener(){

                public void actionPerformed(ActionEvent ae){

                    int x = getWidth();
                    if(x > 617){
                        setSize(new Dimension(getWidth()-4,getHeight()));
                    }
                    else{
                        setSize(new Dimension(617,getHeight()));
                        expand.setEnabled(true);
                        timer.stop();
                    }

                    mainPanel.revalidate();
                    mainPanel.repaint();


                }
            });
            timer.start();
            expand.setEnabled(false);
        }

        revalidate();
        repaint();
    }

    public void initCalendar(){

        dates.clear();

        int maxDate = -1;
        int minDate = 50;

        for(ArrayList<ArrayList<myButton>> lessons : data){
            for(ArrayList<myButton> entry_list : lessons ){
                for(myButton entry : entry_list){

                    int[] temp = new int[3];
                    String deadline = entry.getDeadline();
                    temp[0] = Integer.parseInt(deadline.split("/")[0]);
                    temp[1] = Integer.parseInt(deadline.split("/")[1]);
                    temp[2] = Integer.parseInt(deadline.split("/")[2]);
                    dates.add(temp);

                    if(temp[1] > maxDate){
                        maxDate = temp[1];
                    }
                    if(temp[1] < minDate){
                        minDate = temp[1];
                    }
                }
            }
        }





    }



    //-------------------------------------------//
    @Override
    public void actionPerformed(ActionEvent e) {

        if(e.getSource().equals(calendar)){
            initCalendar();
        }

        if(e.getSource().equals(expand)){
            expandFrame();
        }

        if(e.getSource().equals(rename)){

            renameField.setText("Enter a name...");
            renameField.setBounds(lessons.get(activeLesson).getBounds().x,lessons.get(activeLesson).getBounds().y,lessons.get(activeLesson).getWidth(),lessons.get(activeLesson).getHeight());
            leftPanel.add(renameField,0);

            leftPanel.revalidate(); leftPanel.repaint();

        }
        if(e.getSource().equals(delete)){
            System.out.println("B");
        }

        if(e.getSource().equals(entryOK2)){

            activeEntry.updateData(entryTitle2.getText(),entryDeadline2.getText(),entryDescription2.getText());
            lessons.get(activeLesson).save();

            /*activeEntry.setActive(false);
            activeEntry.setBackground(new Color(42,42,42));
            entryisActive = false;
            processEntryEvent(activeEntry);
            activeEntry = null;
            */
        }
        if(e.getSource() instanceof myButton){

            if(timerRunning){
                return;
            }
            if(state == 1){return;}
            for(ArrayList<myButton> list : data.get(activeLesson)){
                for(myButton entry : list){
                    if(entry.equals(e.getSource())){
                        //entry closes
                        if(entry.isActive()){
                            entry.setActive(false);
                            entryisActive = false;
                            activeEntry = null;
                            processEntryEvent(entry);

                        }
                        else{
                            //entry anoigei
                            if(activeEntry == null){
                                entry.setActive(true);
                                entryisActive = true;
                                activeEntry = entry;
                                processEntryEvent(entry);
                                updateEntryData(entry);
                            }
                            else{

                                activeEntry.setBackground(new Color(42,42,42));
                                activeEntry.setActive(false);
                                activeEntry = null;
                                entryisActive = false;
                                processEntryEvent(entry);

                            }

                            //entry opens
                        }
                    }
                }
            }


        }

        if(e.getSource().equals(entryOK)){

            if(checkErrors(entryTitle.getText(),entryDeadline.getText())){

                String title = entryTitle.getText();
                String desc = entryDescription.getText();
                String deadline = entryDeadline.getText();
                addNewEntry(title,desc,deadline,entry_choice);
                entryTitle.setText("");
                entryDeadline.setText("");
                entryDescription.setText("");
                entry_choice = -1;
                frame.dispose();
            }
            else{

                JOptionPane.showMessageDialog(null, "Invalid parameters!","Error",2);
            }
        }


        if(e.getSource().equals(addTask) && activeLesson!=-1){
            entry_choice = 1;
            popUpWindow();

        }
        if(e.getSource().equals(addProject)&& activeLesson!=-1){
            entry_choice = 2;
            popUpWindow();
        }

        if(e.getSource().equals(addExams)&& activeLesson!=-1){
            entry_choice = 3;
            popUpWindow();
        }

        if(e.getSource().equals(addLesson)){
            frame = new JFrame("Add new Lesson!");
            frame.setSize(new Dimension(300,100));
            frame.setDefaultCloseOperation(frame.DISPOSE_ON_CLOSE);

            Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
            frame.setLocation(dim.width/2-frame.getSize().width/2, dim.height/2-frame.getSize().height/2);

            JPanel temp = new JPanel(null);
            temp.setBounds(frame.getInsets().left,frame.getInsets().top,frame.getWidth(),frame.getHeight());

            addLessonText.setBounds(temp.getInsets().left,temp.getInsets().top,220,25);
            temp.add(addLessonText);


            addLessonOk.setBounds(temp.getInsets().left+220+1,temp.getInsets().top,62,25);
            temp.add(addLessonOk);

            frame.add(temp);
            frame.setVisible(true);

        }

        if(e.getSource().equals(addLessonOk)){

            frame.dispose();



            Lesson lesson = new Lesson(addLessonText.getText(),true);
            addLessonText.setText("");

            lesson.addMouseListener(new PopClickListener());
            lesson.addMouseListener(this);
            lessons.add(lesson);

            ArrayList<myButton> list1 = new ArrayList<myButton>();
            ArrayList<myButton> list2 = new ArrayList<myButton>();
            ArrayList<myButton> list3 = new ArrayList<myButton>();
            ArrayList<ArrayList<myButton>> list = new ArrayList<ArrayList<myButton>>();
            list.add(list1); list.add(list2); list.add(list3);

            data.add(list);
            updateGUI();
        }


    }

    @Override
    public void mouseClicked(MouseEvent e) {


        if(e.getSource() instanceof Lesson){
            if(timerRunning){return;}

            if(entryisActive){
                for(ArrayList<myButton> list : data.get(activeLesson)){
                    for(myButton entry : list){
                        entry.setBackground(new Color(42,42,42));
                        if(entry.isActive()){
                            entry.setActive(false);
                            entryisActive = false;
                            activeEntry = null;
                            processEntryEvent(entry);

                        }
                    }
                }
            }

            for(Lesson l : lessons){
                if(!l.equals(e.getSource()) && l.isFocused() ){
                    for(int i = 0; i<3; i++){
                        l.updateEntry(data.get(activeLesson).get(i),i);
                    }
                    l.save();
                }
            }
        }
        if(e.getSource() instanceof Lesson){

            int pointer = 0;
            for(Lesson l : lessons){
                if(e.getSource().equals(l)){
                    l.setFocused(true);
                    activeLesson = pointer;
                    updateAreas(pointer);
                }
                else{
                    l.setFocused(false);
                }
                pointer++;
            }
        }
    }

    @Override
    public void mousePressed(MouseEvent e) {

        if(e.getSource().equals(renameField)){
            renameField.setText("");
        }

        if(e.getSource() instanceof myButton){
            x = e.getX(); y = e.getY();

            pressed = true;
            for(ArrayList<myButton> list : data.get(activeLesson)){
                for(myButton entry : list){
                    if(e.getSource().equals(entry)){
                        pressedEntry = entry;
                        pressedEntry.setBound(pressedEntry.getBounds());
                        container = entry.getParent();
                        if(container.equals(tasks)){
                            currentPanel = 0;
                            currentSize = data.get(activeLesson).get(currentPanel).size();
                        }
                        else if(container.equals(projects)){
                            currentPanel = 1;
                            currentSize = data.get(activeLesson).get(currentPanel).size();
                        }
                        else{
                            currentPanel = 2;
                            currentSize = data.get(activeLesson).get(currentPanel).size();
                        }
                        //pressedEntry.setVisible(false);

                        //mainPanel.revalidate(); mainPanel.repaint();
                        break;
                    }
                }

            }
        }
        else{
            pressed = false;
            pressedEntry = null;
            currentPanel = -1;

        }

    }

    @Override
    public void mouseReleased(MouseEvent e) {

        if(pressedEntry!=null){



            //pressedEntry.setActive(false);
            if(pressedEntry.isActive()){
                pressedEntry.setBackground(new Color(80,80,80));
            }
            else{
                pressedEntry.setBorder(BorderFactory.createLineBorder(Color.gray));
                pressedEntry.setBackground(new Color(42,42,42));
            }


            state = 0;
            mainPanel.remove(pressedEntry);
            container.add(pressedEntry);
            pressedEntry.setBounds(pressedEntry.getBound());
            //pressedEntry.setVisible(false);
            //pressedEntry = null;
            counter = 0;

            if(trashcan.isEntered()){
                pressedEntry.setVisible(false);
                int dialogButton = JOptionPane.YES_NO_OPTION;
                int dialogResult = JOptionPane.showConfirmDialog(this, "Do you want to delete this entry?", "Warning", dialogButton);
                if(dialogResult == 0) { //yes
                    removeEntry(pressedEntry);
                }
                else {
                    pressedEntry.setVisible(true);
                }
            }
            else{
                pressedEntry.setVisible(true);
            }



            currentPanel = -1;
            pressedEntry = null;
            trashcan.setVisible(false);
            mainPanel.revalidate(); mainPanel.repaint();

        }

    }

    @Override
    public void mouseEntered(MouseEvent e) {

    }

    @Override
    public void mouseExited(MouseEvent e) {

    }

    @Override
    public void mouseDragged(MouseEvent e) {


        if(e.getSource() instanceof myButton && pressed){

            counter++;
            e.translatePoint(e.getComponent().getLocation().x-x, e.getComponent().getLocation().y-y);


            pressedEntry.setLocation(e.getX(),e.getY());
            if(trashcan.isEntered()){
                pressedEntry.setBackground(new Color(175, 61, 49));
            }

            if(state == 0){
                container.remove(pressedEntry);
                mainPanel.add(pressedEntry,2);
                state = 1;
            }
            if(counter<2){
                pressedEntry.setVisible(false);
            }
            else{
                trashcan.setVisible(true);
                pressedEntry.setVisible(true);
                pressedEntry.setBorder(BorderFactory.createLineBorder(new Color(191, 252, 202)));
                mainPanel.revalidate(); mainPanel.repaint();
            }

            int c = 0;
            for(myButton entry : data.get(activeLesson).get(currentPanel)){

                if(entry.equals(pressedEntry)){
                    c++;
                    continue;
                }
                int xx = 150*(currentPanel+1); int yy = entry.getY();


                if((e.getX()>= xx-20 && e.getX()<xx+entry.getWidth()+20) && (e.getY()>= yy-45 && e.getY()<yy+entry.getHeight()+45)){
                    if(!trashcan.isEntered()) pressedEntry.setBackground(new Color(80,80,80,200));

                }
                else{
                    if(!trashcan.isEntered()) pressedEntry.setBackground(new Color(80,80,80));
                }
                if((e.getX()>= xx-30 && e.getX()<xx+entry.getWidth()) && (e.getY()>= yy && e.getY()<yy+entry.getHeight())){
                   //entry.setBackground(new Color(22,22,22));
                    if(entry.isAdjusting()) continue;
                    ChangePos(entry,c);
                    break;
                }

                c++;

            }



        }
    }

    @Override
    public void mouseMoved(MouseEvent e) {

    }


    @Override
    public void keyTyped(KeyEvent e) {

    }

    @Override
    public void keyPressed(KeyEvent e) {

        if(e.getKeyCode() == KeyEvent.VK_ENTER && e.getSource().equals(renameField)){

            String newName = renameField.getText();
            if(lessons.get(activeLesson).getText().equals(newName)){
                leftPanel.remove(renameField);
                leftPanel.revalidate(); leftPanel.repaint();
                return;
            }
            if(newName.equals("")){
                if(timer3.isRunning()){
                    timer3.stop();
                }
                if(faderState){
                    faderState = false;
                }
                infoField2.setForeground(Color.red);
                infoField2.setText("Lesson name cannot be null!");
                TextFader fader = new TextFader();
                fader.start();

                leftPanel.remove(renameField);
                leftPanel.revalidate(); leftPanel.repaint();
                return;
            }
            //System.out.println(newName);
            for(Lesson l : lessons){

                if(l.getText().equals(newName)){
                    if(faderState){
                        faderState = false;
                    }
                    if(timer3.isRunning()) {
                        timer3.stop();
                    }

                    infoField2.setForeground(Color.red);
                    infoField2.setText("Lesson name already exists!");


                    TextFader fader = new TextFader();
                    fader.start();

                    leftPanel.remove(renameField);
                    leftPanel.revalidate(); leftPanel.repaint();
                    return;
                }
            }

            filesToDelete.add(lessons.get(activeLesson).getText());

            lessons.get(activeLesson).rename(newName);
            lessons.get(activeLesson).setText(newName);
            leftPanel.remove(renameField);
            leftPanel.revalidate(); leftPanel.repaint();


        }
    }

    @Override
    public void keyReleased(KeyEvent e) {

    }



}
