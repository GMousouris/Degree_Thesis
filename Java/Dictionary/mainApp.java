import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import javax.swing.SwingUtilities;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Scanner;
import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.awt.*;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.awt.Toolkit;
import java.io.FileInputStream;
import javax.swing.SwingWorker;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.nio.file.Files;
import java.lang.Math;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Vector;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import javax.swing.JComboBox;
import javax.swing.JSplitPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.UIManager;
import javax.swing.border.Border;
import javax.swing.border.*;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.plaf.ColorUIResource;
import javax.imageio.ImageIO;
import javax.swing.BorderFactory;
import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.Clip;
import javax.sound.sampled.AudioSystem;
import java.awt.event.WindowEvent;
import java.awt.event.WindowListener;
import javax.swing.text.Caret;
import java.util.Vector;
import javax.swing.JComboBox;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import javax.swing.text.DefaultCaret;
import javax.swing.text.JTextComponent;
import javax.swing.text.BadLocationException;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.awt.event.*;
import javax.swing.GroupLayout.Alignment;
import javax.swing.JButton;
import javax.swing.SwingConstants;
import java.awt.Container;
import java.awt.GridLayout;
import javax.swing.JFrame;
import javax.swing.JTextField;
import javax.swing.JToolTip;
import java.awt.Cursor;
import java.util.Arrays;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import java.awt.event.FocusListener;
import javax.swing.text.Document;
import javax.swing.ToolTipManager;
import javax.swing.text.*;
import javax.swing.Timer;
import javax.swing.text.BadLocationException;
import javax.swing.text.DefaultHighlighter.DefaultHighlightPainter;
import javax.swing.text.Highlighter;
import javax.swing.text.Highlighter.Highlight;
import java.awt.event.ComponentEvent;
import java.awt.event.ComponentListener;
import java.awt.event.ItemEvent;
import java.awt.event.ItemListener;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JCheckBox;



public class mainApp extends JFrame implements ActionListener,MouseListener,WindowListener,KeyListener,FocusListener,ComponentListener{

    Database data = new Database();
	ArrayList<ArrayList<word>> database = new ArrayList<ArrayList<word>>();
	ArrayList<ArrayList<MyJTextField>> database_translations = new ArrayList<ArrayList<MyJTextField>>();
	ArrayList<ArrayList<MyJTextField>> database_synonyms = new ArrayList<ArrayList<MyJTextField>>();
	ArrayList<ArrayList<MyButton2>> database_words = new ArrayList<ArrayList<MyButton2>>();

    ArrayList<ArrayList<MyCheckBox>> checkbox_list = new ArrayList<ArrayList<MyCheckBox>>();

	ArrayList<Pointer> search_database = new ArrayList<Pointer>();

	String alphabet[] = new String[]{"A","B","C","D","E","F","G","H","I","J","K","L","M","N","O","P","Q","R","S","T","U","V","W","X","Y","Z"};
    String temp_word;

    int distance = 0;
	MenuButton[] menu;

	int pointer = -1;
	String label = "";
	int last_pointer = -1;
	String last_label ="";
	String clicked_label="";
	Boolean closed = true;

	boolean searching=false;

	ArrayList<MyButton2> words = new ArrayList<MyButton2>();
	ArrayList<MyJTextField> synonyms = new ArrayList<MyJTextField>();
	ArrayList<MyJTextField> translations = new ArrayList<MyJTextField>();
	ArrayList<String> syn = new ArrayList<String>();
	ArrayList<String> trans = new ArrayList<String>();
	MyJTextField clicked;

	JTextField info_word,info_synonyms,info_translations;
	JTextArea info_notes;
	JLabel info_word_label,info_synonyms_label,info_translations_label;
    JTextField help;


    JLabel az,leksh,synwnymo,transl;
	JPanel data_panel,left_data;
    MyPanel left,right,main;
    JScrollPane data_scroll,left_scroll,info_notes_scroll;
    JTextField writing;
    JTextField search;

    MyButton add,remove,update,update_info;
    JLabel search_button;

    int current_state = -1;
    int last_current_state = -1;
    int clicked_obj = -1;

    String typing = "";
    boolean char_type = false;
    boolean box_pressed = false;
    boolean box_exited = false;
    
    DocumentListener documentListener;
    boolean add_focused = false;

    JPanel command,info;
    JTextField command_text;

    Pointer clicked_word;
    Border border;

    int x,y;

    word temp_wordd;

    final int main_height = 687-100-15-10;
    final int main_width = 879-40-40+3-10;
    boolean menu_clicked = false;
    boolean word_clicked = false;
    /*-----------------------------------*/
    MyCheckBox2 expand_button;
    MyPanel statistics;
    MyPanel statistics_bottom;
    boolean expanded = false;
    MyPanel charts,charts_data;
    Chart_button paint_chart;
    MyChartField chart_array[] = new MyChartField[26];
    Timer timer;
    boolean done = false;
    int w = 0;
    int total_words = 0;
    int xi[] = new int[26];
    int xi_max[] = new int[26];
    int all_done = 26;
    int count0 = 0;
    int count1 = 0;
    
    JCheckBox dynamic_search;
    boolean auto = false;
    boolean check = false;
    

	public mainApp(){

       
        addWindowListener(this);
 
        /*-------------------------Database~Initialization----------------------------*/
        data.read_database();
        database = new ArrayList<ArrayList<word>>(data.get_database());
        initialize_labels_buttons();
        initialize_gui();
        /*-----------------------------------------------------------------------------*/




        /*----------------------------------------------------------------------------*/
        JCheckBox dynamic_search = new JCheckBox("Dynamic Search");
        dynamic_search.setToolTipText("Dynamic search is Inactive");
        info_word = new JTextField();
        info_word.addKeyListener(this);
        info_translations = new JTextField();
        info_translations.addKeyListener(this);
        info_synonyms = new JTextField();
        info_synonyms.addKeyListener(this);

        this.setTitle("Virtual Dictionairy");
        
        /*----------------------TIMER--------------*/

        timer = new Timer(1, new ActionListener(){
            public void actionPerformed(ActionEvent ae){

                Insets insets = charts_data.getInsets();
                for(int i=0; i<26; i++){
                    if(xi[i]<xi_max[i]){
                        xi[i]++;
                        chart_array[i].setBounds(insets.left,insets.top+i*15,15+xi[i],13);
                        
                    }
                    else if(xi[i]>xi_max[i]){
                        xi[i]--;
                        chart_array[i].setBounds(insets.left,insets.top+i*15,15+xi[i],13);
                        
                    }
                }
                for(int i=0; i<26; i++){
                    if(xi[i]==xi_max[i]){
                        count0++;
                    }
                    else{
                        count0 = 0;
                        break;   
                    }
                }
                if(count0==26){
                    timer.stop();
                }
            
            
                
            
                
            }
        });

        clicked_word = new Pointer(-1,-1);
        /*------------------------------------*/
        ToolTipManager.sharedInstance().setInitialDelay(50);
        ToolTipManager.sharedInstance().setDismissDelay(1500);
		/*----------------------------------------------------------------------------*/
		this.setLayout(null);
		main = new MyPanel();
		Insets insets = main.getInsets();
		main.setBounds(this.getInsets().left-14,this.getInsets().top-20,879+200,878); //670 -> 861
		main.setBackground(new Color(43,43,43));
        main.setLayout(null);
        main.addMouseListener(this);

        

        left = new MyPanel("Images\\left_data_closed.png"); /*-------------------------LEFT PANEL---------------------*/
        left.setBounds(12+insets.left,20+insets.top,200-40-40,878); //-40
        left.setBackground(Color.darkGray);
        left.setLayout(null);
        left.setFocusable(true);
        left.addMouseListener(this);
        left.addKeyListener(this);

        /*---------------------------------------------SCROLL_LEFT----------------*/

        left_data = new JPanel();
        left_data.setFocusable(true);
        left_data.setLayout(null);
        left_data.setBackground(new Color(22,40,50));
        left_data.setBounds(left.getInsets().left+20+20+8+2-40,left.getInsets().top-40-20+70-1+30+3-15,111,26*19);
        
        /* --------------------MENU CREATION------------------*/

        az = new JLabel("Database");
        az.setOpaque(true);
        az.setBackground(Color.gray);
        az.setForeground(Color.white);
        az.setFont(new Font("default",Font.BOLD,16));
        az.setHorizontalAlignment(SwingConstants.LEADING);
        az.setBounds(left.getInsets().left+9+5+6+20+8+2-40,left.getInsets().top-40-20+100-22-22-10+30+4-1-15,110,22);
        left.add(az);

        menu = new MenuButton[26]; 
        for(int i=0; i<menu.length; i++){
        	menu[i] = new MenuButton(" "+alphabet[i],i);
        	menu[i].setBounds(left_data.getInsets().left,left_data.getInsets().top+distance,110,18);
            menu[i].setFont(new Font("default",Font.BOLD,16));
            menu[i].setHorizontalAlignment(SwingConstants.LEADING);
        	distance+=19;
        	menu[i].addActionListener(this);
        	menu[i].addMouseListener(this);
            menu[i].addKeyListener(this);
            menu[i].setFocusable(true);
        	left_data.add(menu[i]);

        }
        
        left.add(left_data);
        
        
        /*------------------------DOCUMENT_LISTENER-----------------------*/

        documentListener = new DocumentListener() {
              public void changedUpdate(DocumentEvent documentEvent) {
                
              }
              public void insertUpdate(DocumentEvent documentEvent) {
                        if(!search.getText().equals("") && search.getText()!=null){
                            searching = true;
                            typing = search.getText();
                            
                            if(auto){
                                update_search_database(0);
                                check_clicked_word_search();
                                create_gui(typing);
                                if(expanded){
                                //restart_charts();
                                //search_graph();
                                }   

                            }
                            dynamic_search.setEnabled(false);
                            dynamic_search.setForeground(Color.gray);
                             
                        }
            
               
              }
              public void removeUpdate(DocumentEvent documentEvent) {


                        if(!search.getText().equals("") && search.getText()!=null){
                             searching = true;
                             typing = search.getText();
                             
                             
                             if(auto){
                                update_search_database(0);
                                check_clicked_word_search();
                                create_gui(typing);
                                if(expanded){
                                   // restart_charts();
                                    //search_graph();
                                }  

                             }
                                 
                         }
                         else{
                             searching = false;
                             restart_charts();
                                check_clicked_word_search();
                                update_search_database(0);
                                if(last_current_state>=0){
                                    create_gui(last_current_state);
                                }
                                else{
                                    restart_gui();
                                }
                                if(expanded){
                                   restart_gui_charts();
                                   evaluate();
                              

                             }
                             dynamic_search.setEnabled(true);
                             if(dynamic_search.isSelected()){
                                dynamic_search.setForeground(Color.lightGray);
                             }
                             else{
                                dynamic_search.setForeground(Color.gray);
                             }
                             

                             
                         }
              	
              }  
        }; 
        

        /*--------------------------------SEARCH--------------------------->>>>>>>>>>>>----------*/

        search = new JTextField("",200);
        search.setBackground(Color.lightGray);
        search.setBounds(main.getInsets().left+219-40+6-40,main.getInsets().top+674-40-20-100-15,160,21);
        search.setBorder(BorderFactory.createLineBorder(Color.darkGray));
        search.addActionListener(this);
        search.addKeyListener(this);
        search.addMouseListener(this);
        search.getDocument().addDocumentListener(documentListener);
        search.setToolTipText("Search..");

        search_button = new JLabel("Search");
        search_button.setOpaque(true);
        search_button.setBackground(Color.darkGray);
        search_button.setForeground(Color.white);
        
        search_button.setBounds(main.getInsets().left+215+160-40+6-40,main.getInsets().top+674-40-20-100-15,80,21);
        search_button.setHorizontalAlignment(SwingConstants.CENTER);
        search_button.setFont(new Font("default",Font.PLAIN,14));
        search_button.setFocusable(false);

        
        dynamic_search.setBackground(Color.darkGray);
        dynamic_search.setForeground(Color.gray);
        dynamic_search.setFocusPainted(false);
        dynamic_search.setBounds(main.getInsets().left+219-40+6-40+160+80+4-4,main.getInsets().top+674-40-20-100-15,120,21);
        dynamic_search.addItemListener(new ItemListener() {
                public void itemStateChanged(ItemEvent e) {
                    if(e.getStateChange() == ItemEvent.SELECTED) {//checkbox has been selected
                        auto = true;
                        dynamic_search.setForeground(Color.lightGray);
                        dynamic_search.setToolTipText("Dynamic Search is Active");
                        
                    } else {
                        auto = false;
                        dynamic_search.setForeground(Color.gray);
                        dynamic_search.setToolTipText("Dynamic Search is Inactive");
                        
                    };
                }
            });


        /*--------------------INFO_PANEL---------------------------------*/
        info = new JPanel();
        info.setLayout(null);
		info.setBounds(main.getInsets().left+219-40-40,main.getInsets().top+674+25+2-40-20-100-15,652,78-5);
		info.setBackground(Color.white);


        /*-------------------RIGHT_PANEL_INITIALIZATION-----------------*/
        right = new MyPanel("Images\\right52.png");
        right.setBounds(12+200+insets.left-40-40,71+insets.top-40-20-15,879-200-12,596-100); //71+insets.top-40-20-15
        right.setBackground(Color.darkGray);
        //right.setBorder(BorderFactory.createLineBorder(Color.lightGray));
        right.setLayout(null);
        right.addMouseListener(this);

        leksh = new JLabel("WORD");
		transl = new JLabel("TRANSLATION");
		transl.setOpaque(true);
		transl.setHorizontalAlignment(SwingConstants.CENTER);
		transl.setBackground(Color.gray);
		transl.setForeground(Color.white);
		transl.setFont(new Font("default",Font.BOLD,16));
		transl.setBounds(right.getInsets().left+4+201+6,right.getInsets().top+8+20,201,22);
        synwnymo = new JLabel("SYNONYM");
        leksh.setFont(new Font("default",Font.BOLD,16));
        synwnymo.setFont(new Font("default",Font.BOLD,16));
        leksh.setHorizontalAlignment(SwingConstants.CENTER);
        synwnymo.setHorizontalAlignment(SwingConstants.CENTER);
        leksh.setBackground(Color.gray);
        leksh.setOpaque(true);
        synwnymo.setBackground(Color.gray);
        synwnymo.setOpaque(true);
        leksh.setForeground(Color.white);
        synwnymo.setForeground(Color.white);
        leksh.setBounds(right.getInsets().left+3+6,right.getInsets().top+8+20,201,22);
        synwnymo.setBounds(right.getInsets().left+5+6+201+201,right.getInsets().top+8+20,199,22);
        right.add(leksh);
        right.add(synwnymo);
        right.add(transl);

        
        expand_button.setBounds(right.getInsets().left+5+6+201+201+200,right.getInsets().top+8+20,54,22);

        data_panel = new JPanel();
		data_panel.setLayout(null);
		data_panel.addMouseListener(this);
		data_panel.setPreferredSize(new Dimension(380+211,0*26));
		data_panel.setBackground(new Color(31,67,85));

		data_scroll = new JScrollPane(data_panel);
		data_scroll.getVerticalScrollBar().setUnitIncrement(20);
		data_scroll.setBorder(null);
		data_scroll.addMouseListener(this);
		data_scroll.setBorder(BorderFactory.createLineBorder(Color.black));
		data_scroll.setBounds(right.getInsets().left+2+6,right.getInsets().top+50,450+211,400);
		data_scroll.setViewportView(data_panel);
		//data_scroll.setPreferredSize(new Dimension(450+211,500));


        
        writing = new JTextField("",230);
        writing.setToolTipText("Type a word to add here..");
        writing.addFocusListener(this);
		writing.addKeyListener(this);
		writing.addActionListener(this);
		writing.addMouseListener(this);
		writing.setPreferredSize(new Dimension(230,24));
		writing.setBounds(right.getInsets().left+5+6+2,right.getInsets().top+564-100,180,21);
		writing.setBorder(BorderFactory.createLoweredSoftBevelBorder());
		writing.setText("Add word...");

        add = new MyButton("Add Word");
		add.setBounds(right.getInsets().left+244+6-30-20,right.getInsets().top+562-100+1,90,24);
		add.addFocusListener(this);
		add.addMouseListener(this);
		add.addActionListener(this);
		add.setFocusable(false);

		update = new MyButton("Update Data");
		update.setBounds(right.getInsets().left+244+6-30-20+300+5,right.getInsets().top+562-100+1,105,24);
		update.addMouseListener(this);
		update.addActionListener(this);
		
        right.add(expand_button);
        right.add(data_scroll);
        right.add(writing);
		right.add(add);
		right.add(update);
        /*--------------statistics-----------------------------------*/

        statistics = new MyPanel("Images\\statistics223.png");
        statistics.setBounds(12+200+insets.left-40-40+667+1,24+insets.top+23-23,300,570-42+200);
        statistics.setBackground(Color.darkGray);
        statistics.setLayout(null);

        charts = new MyPanel("Images\\charts2.png");
        charts.setBackground(new Color(43,43,43));
        charts.setBounds(statistics.getInsets().left,statistics.getInsets().top+1,299,400);
        charts.setLayout(null);
        
        charts_data = new MyPanel();
        //charts_data.setBackground(new Color(43,43,43));
        charts_data.setBackground(new Color(200,210,196));
        charts_data.setBounds(charts.getInsets().left+16,charts.getInsets().top,258,389);
        charts_data.setLayout(null);

        int distan = 0;
        for(int i=0; i<chart_array.length; i++){

            chart_array[i] = new MyChartField(" "+alphabet[i]);
            chart_array[i].setForeground(Color.black);
            chart_array[i].setEditable(false);
            chart_array[i].setCursor(Cursor.getDefaultCursor());
            //chart_array[i].setBackground(new Color(63,128,193)); //91,155,219
            chart_array[i].setBackground(new Color(228,150,73));
            //chart_array[i].setBackground(Color.white);
            chart_array[i].setBorder(BorderFactory.createLineBorder(Color.black));
            chart_array[i].setFont(new Font("default",Font.PLAIN,12));
            chart_array[i].setBounds(charts_data.getInsets().left,charts_data.getInsets().top+distan,15,13);
            chart_array[i].addMouseListener(this);
            chart_array[i].addActionListener(this);
            distan = distan + 15;
            charts_data.add(chart_array[i]);
        }

        charts.add(charts_data);

        /*paint_chart = new Chart_button("Evaluate");
        paint_chart.setBounds(statistics.getInsets().left+30,statistics.getInsets().top+400+30,100,20);
        paint_chart.addActionListener(this);
        paint_chart.addMouseListener(this);

        Vector list_choices = new Vector();
        list_choices.add("Automatic");
        list_choices.add("Total words deleted");
        list_choices.add("Total words added");

        JComboBox choices = new JComboBox(list_choices);
        choices.setSelectedIndex(0);
        choices.setBackground(new Color(200,210,196));
        choices.setForeground(Color.darkGray);
        choices.setBorder(null);
        choices.setBounds(statistics.getInsets().left+30,statistics.getInsets().top+400+30-1,200,21);*/

       
        //initialize_charts_panel(charts.getInsets());
		statistics.add(charts);
        //statistics.add(paint_chart);
        //statistics.add(choices);
		
        initialize_temps();
		/*-------------------RIGHT_PANEL_COMPLETED_______________________*/

      
        main.add(search);
        main.add(search_button);
        main.add(dynamic_search);
        main.add(left);
        main.add(right);
        main.add(statistics);
        
        
		add(main);
		pack();
        setResizable(false);
		setSize(879-40-40+3-10,686-100-15-10); //650 -> 861
        Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
        this.setLocation(dim.width/2-this.getSize().width/2, dim.height/2-this.getSize().height/2);
		
		setVisible(true);
		setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
      
		
	}

    /*------------------------GUI_INFO_FOR_WORD_CLICKED________________________*/
	public void gui_word_info(word wordd,int code){
       
        info.removeAll();

        word temp_word = wordd;
        temp_wordd = wordd;

        //command_text.setBounds(main.getInsets().left+219-40-40,main.getInsets().top+684+43+4-5-100-15,652,20);
        int text_height = 684+43;
        int height = main_height;
        int width = this.getWidth();
        int k = 1;
        if(code==1){/*-----------OPENING---------------*/
        	//command_text.setBounds(main.getInsets().left+219-40-40,main.getInsets().top+684+43+105-40-20-20+10+4-5-100-15,652,20);
        	if(closed){
        	    for(int i=height; i<806-5-100-15-20-10; i=i+1){
			        setSize(width,i);
			        k++;
			        
		        }
                setSize(width,getHeight()+1);
		    }

		    info.setBounds(main.getInsets().left+219-40-40,main.getInsets().top+674+27-40-20-100-15,648,124-5);
            info.addKeyListener(this);
            

		    info_word_label = new JLabel("  WORD");
		    info_word_label.setOpaque(true);
		    info_word_label.setBackground(new Color(26,26,26));
		    info_word_label.setForeground(Color.white);
		    info_word_label.setBounds(info.getInsets().left+5+3,info.getInsets().top+5+7,100,22);

		    info_translations_label = new JLabel("  TRANSLATION");
		    info_translations_label.setOpaque(true);
		    info_translations_label.setBackground(Color.lightGray);
		    info_translations_label.setForeground(Color.black);
		    info_translations_label.setBounds(info.getInsets().left+5+3,info.getInsets().top+5+7+26,100,22);

		    info_synonyms_label = new JLabel("  SYNONYM");
		    info_synonyms_label.setOpaque(true);
		    info_synonyms_label.setBackground(new Color(176,179,255));
		    info_synonyms_label.setForeground(Color.black);
		    info_synonyms_label.setBounds(info.getInsets().left+5+3,info.getInsets().top+5+26+26+7,100,22);


            /*info_word = new JTextField();
            info_word.addKeyListener(this);
            info_translations = new JTextField();
            info_translations.addKeyListener(this);
            info_synonyms = new JTextField();
            info_synonyms.addKeyListener(this);*/
            
            info_word.setBackground(Color.lightGray);
            info_word.setForeground(Color.black);
            info_word.setBounds(info.getInsets().left+5+100+5+3,info.getInsets().top+5+7,200,22);
            info_word.setText(temp_word.get_word());

            info_translations.setBackground(Color.lightGray);
            info_translations.setForeground(Color.black);
            info_translations.setBounds(info.getInsets().left+5+100+5+3,info.getInsets().top+5+26+7,200,22);
            ArrayList<String> trans2 = new ArrayList<String>(temp_word.get_translations());
            if(trans2.size()>0){
            	info_translations.setText(trans2.get(0));
            }
            ArrayList<String> syn2 = new ArrayList<String>(temp_word.get_synonyms());
            if(syn2.size()>0){
            	info_synonyms.setText(syn2.get(0));
            }
            info_synonyms.setBackground(Color.lightGray);
            info_synonyms.setForeground(Color.black);
            info_synonyms.setBounds(info.getInsets().left+5+100+5+3,info.getInsets().top+5+26+26+7,200,22);

            help = new JTextField("*Remember to always SAVE your modifications before leaving this Field!");
            help.setForeground(Color.red);
            help.setBackground(Color.white);
            help.setBorder(null);
            help.setBounds(info.getInsets().left+5+3,info.getInsets().top+5+26+26+7+26+4+3,400,22);
            help.setEditable(false);

            info_notes = new JTextArea();
            ArrayList<String> notes2 = new ArrayList<String>(temp_word.get_notes());
            if(notes2.size()>0){
                for(int q=0; q<notes2.size(); q++){
                    
                    if(q==notes2.size()-1){
                        info_notes.append(notes2.get(q));
                    }
                    else{
                        info_notes.append(notes2.get(q)+"\n");
                    }
                    
                }
            }
            info_notes.setBackground(new Color(233,209,161));
            info_notes.setForeground(Color.black);

           
            info_notes_scroll = new JScrollPane(info_notes,
            JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
            JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);

            info_notes_scroll.getViewport().setOpaque(false);
            info_notes_scroll.getVerticalScrollBar().setUnitIncrement(15);
            info_notes_scroll.setViewportView(info_notes);
            info_notes_scroll.setBounds(info.getInsets().left+5+100+5+200+8+10+3+3,info.getInsets().top+5+7,300,74);

            update_info = new MyButton("Save");
            update_info.addMouseListener(this);
            update_info.addActionListener(this);
            update_info.setFocusable(true);
            update_info.setBounds(info.getInsets().left+5+3+200+200+100+60-4,info.getInsets().top+6+26+26+7+24+3-2,70,25);


            info.add(help);
            info.add(update_info);
            info.add(info_word_label);
            info.add(info_translations_label);
            info.add(info_synonyms_label);

            info.add(info_word);
            info.add(info_translations);
            info.add(info_synonyms);

            info.add(info_notes_scroll);
		    
		    closed = false;

		    


        }
        

        else{/*--------------CLOSING----------------*/
        	height = this.getHeight();
        	for(int i=height; i>main_height; i=i-1){
        		setSize(width,i);
        		//command_text.setBounds(main.getInsets().left+219-40-40,main.getInsets().top+684+43-i-60-100-15,652,20);
        	}
        	setSize(width,this.getHeight()-2);

        	//command_text.setBounds(main.getInsets().left+219-40-40,main.getInsets().top+684+43-1-60-20-100-15,652,20);
        	info.setBounds(main.getInsets().left+219-40-40,main.getInsets().top+674+27+50-40-20-20-100-15,652,120-5);
            closed = true;
        }


        main.add(info);
        main.revalidate();
        main.repaint();
		


	}


    /*-------------------------RESTART_GUI_TO_CURRENT_STATE = -1 ------------------------*/
	public void restart_gui(){

        searching = false;

		right.removeAll();
		right.setLayout(null);
		words.clear();
		synonyms.clear();
		syn.clear();
		trans.clear();
		translations.clear();
		search_database.clear();

		transl.setBounds(right.getInsets().left+4+201+6,right.getInsets().top+8+20,201,22);
        leksh.setBounds(right.getInsets().left+3+6,right.getInsets().top+8+20,201,22);
        synwnymo.setBounds(right.getInsets().left+6+5+201+201,right.getInsets().top+8+20,199,22);
        right.add(leksh);
        right.add(synwnymo);
        right.add(transl);
        
		writing.setBounds(right.getInsets().left+5+6+2,right.getInsets().top+564-100,180,21);
		writing.setText("Add word...");

		add.setBounds(right.getInsets().left+244+6-30-20,right.getInsets().top+562-100+1,90,24);

		update.setBounds(right.getInsets().left+244+6-30-20+300+5,right.getInsets().top+562-100+1,105,24);
		
		data_panel = new JPanel();
		data_panel.setLayout(null);
		data_panel.addMouseListener(this);
		data_panel.setPreferredSize(new Dimension(380+211,0*26));
		data_panel.setBackground(new Color(31,67,85));
        data_panel.setBorder(BorderFactory.createLineBorder(Color.darkGray));

		data_scroll = new JScrollPane(data_panel);
		data_scroll.getVerticalScrollBar().setUnitIncrement(20);
		data_scroll.setBorder(null);
		data_scroll.addMouseListener(this);
		data_scroll.setBorder(BorderFactory.createLineBorder(Color.black));
		data_scroll.setBounds(right.getInsets().left+2+6,right.getInsets().top+50,450+211,400);
		data_scroll.setViewportView(data_panel);

        writing.setBorder(BorderFactory.createLoweredSoftBevelBorder());
        writing.setText("Add word...");
        writing.setFocusable(true);
        writing.setEnabled(true);
        writing.setBackground(Color.white);


        expand_button.setBounds(right.getInsets().left+5+6+201+201+200,right.getInsets().top+8+20,54,22);
        right.add(expand_button);
		right.add(data_scroll);        
        right.add(writing);
		right.add(add);
		right.add(update);

		current_state = -1;
		
		right.revalidate();
		right.repaint();

	}


	/*---------------------------------Add Components Function------------------------*/
	public void create_gui(int id){

        System.out.println(getHeight());

        current_state = id;
        searching = false;

		right.removeAll();
		right.setLayout(null);
		words.clear();
		synonyms.clear();
		syn.clear();
		trans.clear();
		translations.clear();
		search_database.clear();
		typing="";

        leksh.setBounds(right.getInsets().left+3+6,right.getInsets().top+8+20,201,22);
        synwnymo.setBounds(right.getInsets().left+5+6+201+201,right.getInsets().top+8+20,199,22);
        transl.setBounds(right.getInsets().left+4+201+6,right.getInsets().top+8+20,201,22);

        right.add(leksh);
        right.add(synwnymo);
        right.add(transl);

		data_panel = new JPanel();
        
		data_panel.setLayout(null);
		data_panel.addMouseListener(this);
		data_panel.setPreferredSize(new Dimension(380+211,database.get(id).size()*26));
		data_panel.setBackground(new Color(31,67,85));

        distance = 0;
       
        /*-------------------------------[WORD][TRANSLATION][SYNONYM]_CREATION---------------------*/
		for(int i=0; i<database_words.get(id).size(); i++){

			database_words.get(id).get(i).setBounds(data_panel.getInsets().left+1,data_panel.getInsets().top+1+distance,200,22);
			data_panel.add(database_words.get(id).get(i));
            /*-------------------TRANSLATIONS---------------*/
            database_translations.get(id).get(i).setBounds(data_panel.getInsets().left+3+200,data_panel.getInsets().top+1+distance,200,22);
            data_panel.add(database_translations.get(id).get(i));
			 /*--------------------SYNONYMS----------------------*/
            database_synonyms.get(id).get(i).setBounds(data_panel.getInsets().left+3+200+200,data_panel.getInsets().top+1+distance,200,22);
            data_panel.add(database_synonyms.get(id).get(i));
            /*---------------------CHECK_BOXES---------------*/
            checkbox_list.get(id).get(i).setBounds(data_panel.getInsets().left+3+200+200+200+4,data_panel.getInsets().top+3+distance,30,20);
			data_panel.add(checkbox_list.get(id).get(i));

			distance+=23;
            
		}
        /*---------------------------END OF FOR_LOOP---------------------*/

		add.setBounds(right.getInsets().left+244+6-30-20,right.getInsets().top+562-100+1,90,24);
		update.setBounds(right.getInsets().left+244+6-30-20+300+5,right.getInsets().top+562-100+1,105,24);
		writing.setBounds(data_panel.getInsets().left+5+6+2,data_panel.getInsets().top+564-100,180,21);
		data_scroll = new JScrollPane(data_panel);
		data_scroll.getVerticalScrollBar().setUnitIncrement(20);
		data_scroll.setBorder(null);
		data_scroll.addMouseListener(this);
		data_scroll.setBorder(BorderFactory.createLineBorder(Color.black));
		data_scroll.setBounds(right.getInsets().left+2+6,right.getInsets().top+50,450+211,400);
		data_scroll.setViewportView(data_panel);


        writing.setBorder(BorderFactory.createLoweredSoftBevelBorder());
        writing.setText("Add word...");
        writing.setFocusable(true);
        writing.setEnabled(true);
        writing.setBackground(Color.white);

     
        expand_button.setBounds(right.getInsets().left+5+6+201+201+200,right.getInsets().top+8+20,54,22);
        right.add(expand_button);
		right.add(data_scroll);
		right.add(add);
		right.add(remove);
		right.add(writing);
		right.add(update);

		right.revalidate();
		right.repaint();

	}


/*-------------------------------------------------------CREATE_GUI_ON_SEARCHING-------------------------*/
public void create_gui(String name){
        

        current_state = -1;
        searching = true;

		right.removeAll();
		right.setLayout(null);
		words.clear();
		synonyms.clear();
		syn.clear();
		trans.clear();
		translations.clear();
		search_database.clear();

		transl.setBounds(right.getInsets().left+4+201+6,right.getInsets().top+8+20,201,22);
        leksh.setBounds(right.getInsets().left+3+6,right.getInsets().top+8+20,201,22);
        synwnymo.setBounds(right.getInsets().left+5+201+201+6,right.getInsets().top+8+20,199,22);

       
        expand_button.setBounds(right.getInsets().left+5+6+201+201+200,right.getInsets().top+8+20,54,22);
        right.add(expand_button);

        right.add(leksh);
        right.add(synwnymo);
        right.add(transl);

		data_panel = new JPanel();
		data_panel.setLayout(null);
		data_panel.addMouseListener(this);
		data_panel.setBackground(new Color(31,67,85));
        

        
        /*--------------------------Search_Database_ArrayList-----------------------------*/

		for(int i=0; i<database_words.size(); i++){
			int word_added = 0;
			for(int j=0; j<database_words.get(i).size(); j++){
				word_added = 0;

				ArrayList<String> syns = new ArrayList<String>(database_words.get(i).get(j).get_word().get_synonyms());
				ArrayList<String> transls = new ArrayList<String>(database_words.get(i).get(j).get_word().get_translations());

				if(syns.size()>0){
					if(syns.get(0).toLowerCase().contains(name.toLowerCase())){
						search_database.add(new Pointer(database_words.get(i).get(j).get_word(),i,j));
						word_added = 1;
					}
				}
				if(transls.size()>0 && word_added==0){
					if(transls.get(0).toLowerCase().contains(name.toLowerCase())){
						search_database.add(new Pointer(database_words.get(i).get(j).get_word(),i,j));
						word_added = 1;
					}
				}
				if(database.get(i).get(j).get_word().toLowerCase().contains(name.toLowerCase()) && word_added ==0){
					search_database.add(new Pointer(database_words.get(i).get(j).get_word(),i,j));
					word_added = 1;
				}
			}
		}
		/*-------------------------------[Search_Database_Assortment by increasing Order]----------*/
		//search_database = new ArrayList<Pointer>(sort_by_increasing(search_database));
		data_panel.setPreferredSize(new Dimension(380+211,search_database.size()*26));

		/*-------------------------------[WORD][TRANSLATION][SYNONYM]_CREATION---------------------*/

		distance = 0;
		for(int i=0; i<search_database.size(); i++){

			int x = search_database.get(i).getX();
            int y= search_database.get(i).getY();
            /*----------------------WORDS---------------------------*/
			database_words.get(x).get(y).setBounds(data_panel.getInsets().left+1,data_panel.getInsets().top+1+distance,200,22);
			data_panel.add(database_words.get(x).get(y));
			 /*--------------------SYNONYMS----------------------*/
			database_synonyms.get(x).get(y).setBounds(data_panel.getInsets().left+3+200+200,data_panel.getInsets().top+1+distance,200,22);  
            data_panel.add(database_synonyms.get(x).get(y));
            /*--------------------TRANSLATIONS---------------------*/
            database_translations.get(x).get(y).setBounds(data_panel.getInsets().left+3+200,data_panel.getInsets().top+1+distance,200,22);
            data_panel.add(database_translations.get(x).get(y));
			/*---------------------MyCheckBox----------------------*/
            checkbox_list.get(x).get(y).setBounds(data_panel.getInsets().left+3+200+200+200+4,data_panel.getInsets().top+3+distance,30,20);
            data_panel.add(checkbox_list.get(x).get(y));


			distance+=23;

		}


        /*---------------------------END OF FOR_LOOP---------------------*/
		

		add.setBounds(right.getInsets().left+244+6-30-20,right.getInsets().top+562-100+1,90,24);
		update.setBounds(right.getInsets().left+244+6-30-20+300+5,right.getInsets().top+562-100+1,105,24);

		writing.setBounds(data_panel.getInsets().left+5+6+2,data_panel.getInsets().top+564-100,180,21);
		Border bord = new SoftBevelBorder(1,Color.darkGray,Color.darkGray);
		writing.setBorder(bord);
		writing.setText("Add word...");
		writing.setFocusable(false);
        writing.setEnabled(false);
		writing.setBackground(Color.gray);


		data_scroll = new JScrollPane(data_panel);
		data_scroll.getVerticalScrollBar().setUnitIncrement(20);
		data_scroll.setBorder(null);
		data_scroll.addMouseListener(this);
		data_scroll.setBorder(BorderFactory.createLineBorder(Color.black));
		data_scroll.setBounds(right.getInsets().left+2+6,right.getInsets().top+50,450+211,400);
		data_scroll.setViewportView(data_panel);

        if(expanded){
            restart_charts();
            search_graph();
        }

		right.add(data_scroll);
		right.add(add);
		right.add(remove);
		right.add(writing);
		right.add(update);

		right.revalidate();
		right.repaint();

	}
	/*--------------------------------------SEARCHING_COMPLETED--------------------------------------*/

		
		
    public static void main(String[] args){
        
        SwingUtilities.invokeLater(new Runnable()
        {
            public void run()
            {
                new mainApp();
            }
        });
    	

    }


		

 /*----------------------------------------LISTENERS--------------------------------------------------*/
	
    @Override
	public void keyTyped(KeyEvent e) {
	
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void keyPressed(KeyEvent e) {


		if(e.getKeyCode()==KeyEvent.VK_UP){
            
            if(menu_clicked){
                if(current_state>=1){
                    update_database(0);
                    
                    menu[current_state-1].setBackground(Color.darkGray);
                    menu[current_state-1].setForeground(Color.white);
                    menu[current_state-1].setFocus(true);

                    menu[current_state].setFocus(false);
                    menu[current_state].setBackground(Color.lightGray);
                    menu[current_state].setForeground(Color.darkGray);
                    check_clicked_word();
                    create_gui(current_state-1);
                }
                else{
                    update_database(0);
                    menu[current_state].setBackground(Color.lightGray);
                    menu[current_state].setForeground(Color.darkGray);
                    menu[current_state].setFocus(false);

                    menu[25].setBackground(Color.darkGray);
                    menu[25].setFocus(true);
                    menu[25].setForeground(Color.white);
                    check_clicked_word();
                    create_gui(25);
                }
            }

           
        }

        if(e.getKeyCode()==KeyEvent.VK_DOWN){
            
            if(menu_clicked){
                if(current_state<=menu.length-2){
                    update_database(0);
                    
                    menu[current_state+1].setBackground(Color.darkGray);
                    menu[current_state+1].setForeground(Color.white);

                    menu[current_state].setFocus(false);
                    menu[current_state].setBackground(Color.lightGray);
                    menu[current_state].setForeground(Color.darkGray);
                    menu[current_state+1].setFocus(true);
                    check_clicked_word();
                    create_gui(current_state+1);
                }
                else{
                    update_database(0);
                    menu[current_state].setBackground(Color.lightGray);
                    menu[current_state].setForeground(Color.darkGray);
                    menu[current_state].setFocus(false);

                    menu[0].setBackground(Color.darkGray);
                    menu[0].setForeground(Color.white);
                    menu[0].setFocus(true);
                    check_clicked_word();
                    create_gui(0);
                }
            }
        }

		if(e.getKeyCode()==(KeyEvent.VK_ENTER)){
            if(e.getSource().equals(info_translations) || e.getSource().equals(info_word) || e.getSource().equals(info_synonyms) && !searching){
                update_info(1);
            }
            if(e.getSource().equals(info_translations) || e.getSource().equals(info_word) || e.getSource().equals(info_synonyms) && !search.getText().equals(null) && !search.getText().equals("") && current_state==-1){
                update_search_info(1);
               
            }

            if(e.getSource().equals(search) && !auto && !search.getText().equals("") && !search.getText().equals(null)){
                check_clicked_word();
                create_gui(search.getText());
                 if(expanded){
                    //restart_charts();
                    //search_graph();
                }   
            }

			if(e.getSource() instanceof MyJTextField && current_state!=-1){
        	    update_database(0);
                create_gui(current_state);
            }

            else if(searching && e.getSource() instanceof MyJTextField && current_state==-1 && !(typing.equals(""))) {
            	update_search_database(0);
            }

            if(e.getSource().equals(writing)){
            	if(!(writing.getText().equals("") || writing.getText()==null || writing.getText().equals("Add word..."))) {
            		if(current_state==-1){
            			update_database(0);
                        if(check_word(writing.getText())){
                          check_clicked_word();
                          add_word2(writing.getText());
                          create_gui(current_state);
                        }

            			
            		}
            		else{
                        if(check_word(writing.getText())){
                            check_clicked_word();
                            add_word(writing.getText());
                            create_gui(current_state);
                        }
            		}
            		for(int j=0; j<menu.length; j++){
					    if(j==current_state){
					    	menu[j].setBackground(Color.darkGray);//new Color(31,67,85));
					    	menu[j].setFocus(true);
					    	menu[j].setForeground(Color.white);
    
					    }
					    if(j!=current_state){
					    	menu[j].setBackground(Color.lightGray);
					    	menu[j].setForeground(Color.darkGray);
					    	menu[j].setFocus(false);
					    }
				    }
                    writing.setText("Add word...");	
            	}
            }
		}
		// TODO Auto-generated method stub
		
	}

	@Override
	public void keyReleased(KeyEvent e) {
		// TODO Auto-generated method stub	
	}
	@Override
	public void windowOpened(WindowEvent e) {
		// TODO Auto-generated method stub	
	}
	@Override
	public void windowClosing(WindowEvent e) {


        if(e.getSource().equals(this)){
            
            int confirmed = JOptionPane.showConfirmDialog(null, 
            "Are you sure you want to exit the program?", "Exit Program Message Box",
            JOptionPane.YES_NO_OPTION);

            if(confirmed == JOptionPane.YES_OPTION) {
            	data.write_database(database);
                System.exit(0);
            }
        }
    
        
		// TODO Auto-generated method stub	
	}
	@Override
	public void windowClosed(WindowEvent e) {
		// TODO Auto-generated method stub	
	}
	@Override
	public void windowIconified(WindowEvent e) {
		// TODO Auto-generated method stub	
	}
	@Override
	public void windowDeiconified(WindowEvent e) {
		// TODO Auto-generated method stub	
	}

	@Override
	public void windowActivated(WindowEvent e) {
		// TODO Auto-generated method stub	
	}
	@Override
	public void windowDeactivated(WindowEvent e) {
		// TODO Auto-generated method stub	
	}


	@Override
	public void mouseClicked(MouseEvent e) {

        if(e.getSource() instanceof MyCheckBox && !searching && current_state>=0){
            for(int i=0; i<checkbox_list.get(current_state).size(); i++){
                if(checkbox_list.get(current_state).get(i).equals(e.getSource()) && !database_words.get(current_state).get(i).isFocused()){
                    update_database(0);

                    database_words.get(current_state).get(i).setBackground(new Color(169,68,68));
                    database_translations.get(current_state).get(i).setBackground(new Color(169,68,68));
                    database_synonyms.get(current_state).get(i).setBackground(new Color(169,68,68));

                    String name = database_words.get(current_state).get(i).get_word().get_word();
                    if(dialogue_ok_cancel("word '"+name+"' will get DELETED permanently!\nAre you sure?")==JOptionPane.OK_OPTION){
                        delete_word(i);
                    }
                    else{
                    }
                    
                }
            }
        }
        else if(e.getSource() instanceof MyCheckBox && searching && current_state==-1){

            for(int i=0; i<search_database.size(); i++){
                int x0 = search_database.get(i).getX();
                int y0 = search_database.get(i).getY();
                if(checkbox_list.get(x0).get(y0).equals(e.getSource()) && !database_words.get(x0).get(y0).isFocused()){
                    update_search_database(0);

                    database_words.get(x0).get(y0).setBackground(new Color(169,68,68));
                    database_translations.get(x0).get(y0).setBackground(new Color(169,68,68));
                    database_synonyms.get(x0).get(y0).setBackground(new Color(169,68,68));

                    String name = database_words.get(x0).get(y0).get_word().get_word();
                    
                    if(dialogue_ok_cancel("word '"+name+"' will get DELETED permanently!\nAre you sure?")==JOptionPane.OK_OPTION){
                        delete_word2(x0,y0);
                    }
                    else{
                    }
                    
                }
            }

        }

		if(e.getSource() instanceof MyButton2 && !searching){
            update_database(0);
			for(int i=0; i<database_words.get(current_state).size(); i++){

				if( database_words.get(current_state).get(i).equals(e.getSource()) && !database_words.get(current_state).get(i).isFocused()){ //OPEN BOTTOM TAB
                    
                    database_words.get(current_state).get(i).setFocus(true);
					database_words.get(current_state).get(i).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
					database_words.get(current_state).get(i).setBackground(Color.darkGray);
					database_words.get(current_state).get(i).setForeground(Color.white);
					database_synonyms.get(current_state).get(i).setBackground(new Color(153,156,246)); //138,154,255//153,156,246
					database_synonyms.get(current_state).get(i).setForeground(Color.black);
					database_synonyms.get(current_state).get(i).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
					database_synonyms.get(current_state).get(i).setFocusable(false);
					database_translations.get(current_state).get(i).setBackground(new Color(195,195,195));
					database_translations.get(current_state).get(i).setForeground(Color.black);
					database_translations.get(current_state).get(i).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
					database_translations.get(current_state).get(i).setFocusable(false);
                    checkbox_list.get(current_state).get(i).setEnabled(false);
                    checkbox_list.get(current_state).get(i).setFocusable(false);
					gui_word_info(database_words.get(current_state).get(i).get_word(),1);
                    


				}
			    else if(database_words.get(current_state).get(i).equals(e.getSource()) && database_words.get(current_state).get(i).isFocused()) {
                    
                    
					database_words.get(current_state).get(i).setFocus(false); // CLOSE BOTTOM TAB
					database_words.get(current_state).get(i).setBorder(null);
					database_words.get(current_state).get(i).setBackground(Color.white);
					database_words.get(current_state).get(i).setForeground(Color.black);
				    database_synonyms.get(current_state).get(i).setBackground(new Color(176,179,255));
				    database_synonyms.get(current_state).get(i).setForeground(Color.black);
				    database_synonyms.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
				    database_synonyms.get(current_state).get(i).setFocusable(true);
			        database_translations.get(current_state).get(i).setBackground(Color.white);
			        database_translations.get(current_state).get(i).setForeground(Color.black);
			        database_translations.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
			        database_translations.get(current_state).get(i).setFocusable(true);
                    checkbox_list.get(current_state).get(i).setEnabled(true);
                    checkbox_list.get(current_state).get(i).setFocusable(true);
					gui_word_info(database_words.get(current_state).get(i).get_word(),0);
                    
                    

				}
				else{
                    
                   
                    database_words.get(current_state).get(i).setFocus(false);
					database_words.get(current_state).get(i).setBorder(null);
					database_words.get(current_state).get(i).setBackground(Color.white);
					database_words.get(current_state).get(i).setForeground(Color.black);
					database_synonyms.get(current_state).get(i).setBackground(new Color(176,179,255));
					database_synonyms.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
					database_synonyms.get(current_state).get(i).setForeground(Color.black);
					database_synonyms.get(current_state).get(i).setFocusable(true);
					database_translations.get(current_state).get(i).setBackground(Color.white);
					database_translations.get(current_state).get(i).setForeground(Color.black);
					database_translations.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                    checkbox_list.get(current_state).get(i).setEnabled(true);
                    checkbox_list.get(current_state).get(i).setFocusable(true);
                    database_translations.get(current_state).get(i).setFocusable(true);

				}
			}
		}

        else if(e.getSource() instanceof MyButton2 && searching && current_state ==-1){/*---------------SEARCHING_INFO_PANEL--------------------*/
            update_search_database(0);
            for(int i=0; i<search_database.size(); i++){

                int x = search_database.get(i).getX();
                int y = search_database.get(i).getY();

                if( database_words.get(x).get(y).equals(e.getSource()) && !database_words.get(x).get(y).isFocused()){ //OPEN BOTTOM TAB
                    
                    database_words.get(x).get(y).setFocus(true);
                    database_words.get(x).get(y).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
                    database_words.get(x).get(y).setBackground(Color.darkGray);
                    database_words.get(x).get(y).setForeground(Color.white);
                    database_synonyms.get(x).get(y).setBackground(new Color(153,156,246));
                    database_synonyms.get(x).get(y).setForeground(Color.black);
                    database_synonyms.get(x).get(y).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
                    database_synonyms.get(x).get(y).setFocusable(false);
                    database_translations.get(x).get(y).setBackground(new Color(195,195,195));
                    database_translations.get(x).get(y).setForeground(Color.black);
                    database_translations.get(x).get(y).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
                    database_translations.get(x).get(y).setFocusable(false);
                    checkbox_list.get(x).get(y).setEnabled(false);
                    gui_word_info(database_words.get(x).get(y).get_word(),1);
                    


                }
                else if(database_words.get(x).get(y).equals(e.getSource()) && database_words.get(x).get(y).isFocused()) {
                    
              
                    database_words.get(x).get(y).setFocus(false); // CLOSE BOTTOM TAB
                    database_words.get(x).get(y).setBorder(null);
                    database_words.get(x).get(y).setBackground(Color.white);
                    database_words.get(x).get(y).setForeground(Color.black);
                    database_synonyms.get(x).get(y).setBackground(new Color(176,179,255));
                    database_synonyms.get(x).get(y).setForeground(Color.black);
                    database_synonyms.get(x).get(y).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                    database_synonyms.get(x).get(y).setFocusable(true);
                    database_translations.get(x).get(y).setBackground(Color.white);
                    database_translations.get(x).get(y).setForeground(Color.black);
                    database_translations.get(x).get(y).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                    database_translations.get(x).get(y).setFocusable(true);
                    checkbox_list.get(x).get(y).setEnabled(true);
                    gui_word_info(database_words.get(x).get(y).get_word(),0);
                   

                }
                else{
                   
                    database_words.get(x).get(y).setFocus(false);
                    database_words.get(x).get(y).setBorder(null);
                    database_words.get(x).get(y).setBackground(Color.white);
                    database_words.get(x).get(y).setForeground(Color.black);
                    database_synonyms.get(x).get(y).setBackground(new Color(176,179,255));
                    database_synonyms.get(x).get(y).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                    database_synonyms.get(x).get(y).setForeground(Color.black);
                    database_synonyms.get(x).get(y).setFocusable(true);
                    database_translations.get(x).get(y).setBackground(Color.white);
                    database_translations.get(x).get(y).setForeground(Color.black);
                    database_translations.get(x).get(y).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                    checkbox_list.get(x).get(y).setEnabled(true);
                    database_translations.get(x).get(y).setFocusable(true);

                }
            }


        }




        /*-----------------------------------------------------------------------------------------*/
		if(e.getSource().equals(add)){
			typing="";
			search.setText("");
		}

        if(e.getSource().equals(writing) && !searching){
            typing ="";
            search.setText("");
        }

		if(e.getSource() instanceof MyButton && !e.getSource().equals(update) && !e.getSource().equals(update_info)){
			typing="";
			search.setText("");
		}

        if(e.getSource().equals(search)){
        	search.requestFocus();
        	if(current_state!=-1){
        		last_current_state = current_state;
        	}	
        }
        
        if(e.getSource() instanceof MyJTextField){
            

        	for(int i=0; i<synonyms.size(); i++){

        		if(synonyms.get(i).equals(e.getSource())){
        			synonyms.get(i).setFocus(true);
        			pointer = i;
        			label ="synonyms";

        		}
        		else if(!(synonyms.get(i).equals(e.getSource()))){
        			synonyms.get(i).setFocus(false); 
        		}
        	}
        	for(int i=0; i<translations.size(); i++){
        		if(translations.get(i).equals(e.getSource())){
        			translations.get(i).setFocus(true);
        			pointer = i;
        			label ="translations";
        		}
        		else{
        			translations.get(i).setFocus(false);
        		}
        	}
        }
        else if(e.getSource().equals(writing)){
           
            
        }
        else if(e.getSource().equals(search)){
        	search.requestFocus();
        }
        else if(current_state==-1){
        	main.requestFocus();
        	pointer = -1;
        	label="";
        }
        else{
        	data_panel.requestFocus();
        	pointer = -1;
        	label="";
        }
        if(!(e.getSource() instanceof MyJTextField)){
        	label="";
        	pointer=-1;
        }

        if(e.getSource() instanceof MyButton2){
            word_clicked = true;
        }
        else{
            word_clicked = false;
        }
        if(e.getSource() instanceof MenuButton){
            for(int i=0; i<menu.length; i++){
                if(menu[i].equals(e.getSource())){
                    menu[i].requestFocus();

                    menu_clicked = true;
                    break;
                }
            }
            
        }
        else{
            menu_clicked = false;
        }


	}

	@Override
	public void mousePressed(MouseEvent e) {




        if(e.getSource() instanceof MyChartField){
            for(int i=0; i<chart_array.length; i++){
                if(e.getSource().equals(chart_array[i])){
                    chart_array[i].setBackground(new Color(213,133,53));
                    
                }
            }
        }

        


        if(e.getSource() instanceof MyCheckBox && !searching){
            for(int i=0; i<checkbox_list.get(current_state).size(); i++){
                if(checkbox_list.get(current_state).get(i).equals(e.getSource()) && checkbox_list.get(current_state).get(i).isEnabled()){
                    checkbox_list.get(current_state).get(i).setBorder(BorderFactory.createLineBorder(Color.darkGray));

                    database_words.get(current_state).get(i).setBackground(new Color(169,68,68));
                    database_translations.get(current_state).get(i).setBackground(new Color(169,68,68));
                    database_synonyms.get(current_state).get(i).setBackground(new Color(169,68,68));
                    box_pressed = true;
                }
            }
            box_pressed = true;
        }/*--------------------------------------SEARCHING---------------------------------------*/
        else if(e.getSource() instanceof MyCheckBox && searching && current_state==-1){

            for(int i=0; i<search_database.size(); i++){
                int x = search_database.get(i).getX();
                int y = search_database.get(i).getY();

                if( checkbox_list.get(x).get(y).equals(e.getSource()) && checkbox_list.get(x).get(y).isEnabled()){
                    checkbox_list.get(x).get(y).setBorder(BorderFactory.createLineBorder(Color.darkGray));
                    database_words.get(x).get(y).setBackground(new Color(169,68,68));
                    database_translations.get(x).get(y).setBackground(new Color(169,68,68));
                    database_synonyms.get(x).get(y).setBackground(new Color(169,68,68));
                    box_pressed = true;
                }
            }
            box_pressed = true;
        }

        /*----------------------------------------------------------------------------------------*/
        
        if(!e.getSource().equals(add) && !e.getSource().equals(writing)){
            writing.setText("Add word...");
        }

        if(e.getSource().equals(writing) && writing.getText().equals("Add word...") && !searching){
            writing.setText("");
        }

		if(e.getSource().equals(search)){
			search.setBackground(Color.white);
		}
		else{
			search.setBackground(Color.lightGray);
		}


		if(!(e.getSource() instanceof MyJTextField)){
        	label="";
        	pointer=-1;
        }

		if(e.getSource().equals(add)){
			typing="";
			search.setText("");
		}

		if(e.getSource() instanceof MyButton && !e.getSource().equals(update_info) && !e.getSource().equals(update)) {
			typing="";
			search.setText("");
		}

		if(e.getSource().equals(search)){
			search.requestFocus();
			if(current_state!=-1){
				last_current_state = current_state;
			}
        	update_database(0);
            check_clicked_word();
            check_clicked_word_search();
        }

		if(e.getSource() instanceof MyJTextField){
        	for(int i=0; i<synonyms.size(); i++){
        		if(synonyms.get(i).equals(e.getSource())){
        			synonyms.get(i).setFocus(true);
        			pointer = i;
        			label ="synonyms";
        		}
        		else if(!(synonyms.get(i).equals(e.getSource()))){
        			synonyms.get(i).setFocus(false);	
        		}
        	}
        	for(int i=0; i<translations.size(); i++){
        		if(translations.get(i).equals(e.getSource())){
        			translations.get(i).setFocus(true);
        			pointer = i;
        			label ="translations";
        		}
        		else{
        			translations.get(i).setFocus(false);	
        		}
        	}	
        }
		
	}

	@Override
	public void mouseReleased(MouseEvent e) {


        if(e.getSource() instanceof MyChartField){
            for(int i=0; i<chart_array.length; i++){
                if(e.getSource().equals(chart_array[i])){
                    chart_array[i].setBackground(new Color(228,150,73));
                    chart_array[i].setForeground(Color.black);
                }
            }
        }

        if(e.getSource() instanceof MyCheckBox && !searching){
            for(int i=0; i<checkbox_list.get(current_state).size(); i++){
                if(checkbox_list.get(current_state).get(i).equals(e.getSource()) && checkbox_list.get(current_state).get(i).isEnabled()){
                    checkbox_list.get(current_state).get(i).setBorder(BorderFactory.createLineBorder(new Color(39,83,93)));
                    if(box_exited){
                        
                        database_words.get(current_state).get(i).setForeground(Color.black);
                        database_words.get(current_state).get(i).setBackground(Color.white);
                        database_translations.get(current_state).get(i).setBackground(Color.white);
                        database_synonyms.get(current_state).get(i).setBackground(new Color(176,179,255));
                    }
                    else{
                         database_words.get(current_state).get(i).setBackground(new Color(223,97,97));
                         database_translations.get(current_state).get(i).setBackground(new Color(223,97,97));
                         database_synonyms.get(current_state).get(i).setBackground(new Color(223,97,97));
                    }
                   
                }   
            }box_pressed = false;
        }
        else if(e.getSource() instanceof MyCheckBox && searching && current_state==-1){

            for(int i=0; i<search_database.size(); i++){
                int x = search_database.get(i).getX();
                int y = search_database.get(i).getY();

                if( checkbox_list.get(x).get(y).equals(e.getSource()) && checkbox_list.get(x).get(y).isEnabled()){
                    checkbox_list.get(x).get(y).setBorder(BorderFactory.createLineBorder(new Color(39,83,93)));
                    if(box_exited){
                        
                        database_words.get(x).get(y).setForeground(Color.black);
                        database_words.get(x).get(y).setBackground(Color.white);
                        database_translations.get(x).get(y).setBackground(Color.white);
                        database_synonyms.get(x).get(y).setBackground(new Color(176,179,255));
                    }
                    else{
                         database_words.get(x).get(y).setBackground(new Color(223,97,97));
                         database_translations.get(x).get(y).setBackground(new Color(223,97,97));
                         database_synonyms.get(x).get(y).setBackground(new Color(223,97,97));
                    }
                }   
            }box_pressed = false;
        }


	}

	@Override
	public void mouseEntered(MouseEvent e) {

        if(e.getSource() instanceof MyChartField){
            for(int i=0; i<chart_array.length; i++){
                if(e.getSource().equals(chart_array[i])){
                    chart_array[i].setBackground(new Color(255,181,105));
                    chart_array[i].setForeground(Color.black);
                }
            }
        }

        if(e.getSource() instanceof MyCheckBox && !searching && !box_pressed){
            for(int i=0; i<checkbox_list.get(current_state).size(); i++){
                if(e.getSource().equals(checkbox_list.get(current_state).get(i)) && checkbox_list.get(current_state).get(i).isEnabled()){
                    database_words.get(current_state).get(i).setBackground(new Color(223,97,97));
                    database_words.get(current_state).get(i).setForeground(Color.white);
                    database_translations.get(current_state).get(i).setBackground(new Color(223,97,97));
                    database_synonyms.get(current_state).get(i).setBackground(new Color(223,97,97));
                    if(!box_pressed){
                        box_exited = false;
                    }
                    
                }
            }
        }
        else if(e.getSource() instanceof MyCheckBox && searching && !box_pressed && current_state==-1){

            for(int i=0; i<search_database.size(); i++){
                x = search_database.get(i).getX();
                y = search_database.get(i).getY();
                if(e.getSource().equals(checkbox_list.get(x).get(y)) && checkbox_list.get(x).get(y).isEnabled()){
                    database_words.get(x).get(y).setBackground(new Color(223,97,97));
                    database_words.get(x).get(y).setForeground(Color.white);
                    database_translations.get(x).get(y).setBackground(new Color(223,97,97));
                    database_synonyms.get(x).get(y).setBackground(new Color(223,97,97));
                    if(!box_pressed){
                        box_exited = false;
                    }
                    
                }
            }

        }


		if(e.getSource() instanceof MenuButton){
			for(int i=0; i<menu.length; i++){
				if(menu[i].equals(e.getSource()) && !menu[i].focused()){
                    
					menu[i].setBackground(Color.white);
					menu[i].setForeground(Color.black);
				}
			}
		}
		
		if(e.getSource() instanceof MyButton2 && !box_pressed){

            if(current_state>=0 && !searching){
                for(int i=0; i<database_words.get(current_state).size(); i++){
                    if(database_words.get(current_state).get(i).equals(e.getSource()) && !database_words.get(current_state).get(i).isFocused()){
                        database_words.get(current_state).get(i).setBackground(Color.darkGray);
                        database_words.get(current_state).get(i).setForeground(Color.white);
    
                        database_translations.get(current_state).get(i).setBackground(new Color(195,195,195));
                        database_translations.get(current_state).get(i).setForeground(Color.black);
    
                        database_synonyms.get(current_state).get(i).setBackground(new Color(153,156,246));
                        database_synonyms.get(current_state).get(i).setForeground(Color.black);
                    }
                }
            }
            else if(searching && !box_pressed){

                for(int i=0; i<search_database.size(); i++){

                     x = search_database.get(i).getX();
                     y = search_database.get(i).getY();

                     if(database_words.get(x).get(y).equals(e.getSource()) && !database_words.get(x).get(y).isFocused()){
                         database_words.get(x).get(y).setBackground(Color.darkGray);
                         database_words.get(x).get(y).setForeground(Color.white);
     
                         database_translations.get(x).get(y).setBackground(new Color(195,195,195));
                         database_translations.get(x).get(y).setForeground(Color.black);
     
                         database_synonyms.get(x).get(y).setBackground(new Color(153,156,246));
                         database_synonyms.get(x).get(y).setForeground(Color.black);
                     }
                }   
            }

		}
		if(e.getSource() instanceof MyJTextField && !(e.getSource().equals(clicked)) && !box_pressed){

            if(current_state>=0 && !searching){/*----TRANSLATIONS---*/
                for(int i=0; i<database_translations.get(current_state).size(); i++){ 
                      if( database_translations.get(current_state).get(i).equals(e.getSource()) && !database_words.get(current_state).get(i).isFocused()){
      
                          database_translations.get(current_state).get(i).setBorder(BorderFactory.createRaisedBevelBorder());
                          database_translations.get(current_state).get(i).setBackground(new Color(195,195,195));
                          database_translations.get(current_state).get(i).setForeground(Color.black);
      
                          database_words.get(current_state).get(i).setBackground(Color.darkGray);
                          database_words.get(current_state).get(i).setForeground(Color.white);
      
                          database_synonyms.get(current_state).get(i).setBackground(new Color(153,156,246));
                          database_synonyms.get(current_state).get(i).setForeground(Color.black);
                      }
                }
            }
            else if(searching && !box_pressed){

                for(int i=0; i<search_database.size(); i++){

                      int x1 = search_database.get(i).getX();
                      int y1 = search_database.get(i).getY();

                      if( database_translations.get(x1).get(y1).equals(e.getSource()) && !database_words.get(x1).get(y1).isFocused()){
      
                          database_translations.get(x1).get(y1).setBorder(BorderFactory.createRaisedBevelBorder());
                          database_translations.get(x1).get(y1).setBackground(new Color(195,195,195));
                          database_translations.get(x1).get(y1).setForeground(Color.black);
      
                          database_words.get(x1).get(y1).setBackground(Color.darkGray);
                          database_words.get(x1).get(y1).setForeground(Color.white);
      
                          database_synonyms.get(x1).get(y1).setBackground(new Color(153,156,246));
                          database_synonyms.get(x1).get(y1).setForeground(Color.black);
                      }
                }

            }
			
            if(current_state>=0 && !searching && !box_pressed){

                for(int i=0; i<database_synonyms.get(current_state).size(); i++){ /*----SYNONYMS----*/
                    if(database_synonyms.get(current_state).get(i).equals(e.getSource()) && !database_words.get(current_state).get(i).isFocused()){
                        
                        database_synonyms.get(current_state).get(i).setBorder(BorderFactory.createRaisedBevelBorder());
                        database_synonyms.get(current_state).get(i).setBackground(new Color(153,156,246));
                        database_synonyms.get(current_state).get(i).setForeground(Color.black);
    
                        database_words.get(current_state).get(i).setBackground(Color.darkGray);
                        database_words.get(current_state).get(i).setForeground(Color.white);
    
                        database_translations.get(current_state).get(i).setBackground(new Color(195,195,195));
                        database_translations.get(current_state).get(i).setForeground(Color.black);
                    }
                }
            }
            else if(searching && !box_pressed){

                for(int i=0; i<search_database.size(); i++){ /*----SYNONYMS----*/

                    int x2 = search_database.get(i).getX();
                    int y2 = search_database.get(i).getY();

                    if( database_synonyms.get(x2).get(y2).equals(e.getSource()) && !database_words.get(x2).get(y2).isFocused()){
                        
                        database_synonyms.get(x2).get(y2).setBorder(BorderFactory.createRaisedBevelBorder());
                        database_synonyms.get(x2).get(y2).setBackground(new Color(153,156,246));
                        database_synonyms.get(x2).get(y2).setForeground(Color.black);
    
                        database_words.get(x2).get(y2).setBackground(Color.darkGray);
                        database_words.get(x2).get(y2).setForeground(Color.white);
    
                        database_translations.get(x2).get(y2).setBackground(new Color(195,195,195));
                        database_translations.get(x2).get(y2).setForeground(Color.black);
                    }
                }
            }

		}


		
	}

	@Override
	public void mouseExited(MouseEvent e) {

        if(e.getSource() instanceof MyChartField){
            for(int i=0; i<chart_array.length; i++){
                if(e.getSource().equals(chart_array[i])){
                    chart_array[i].setBackground(new Color(228,150,73));
                    chart_array[i].setForeground(Color.black);
                }
            }
        }

        if(e.getSource() instanceof MyCheckBox && !searching && !box_pressed ){
            for(int i=0; i<checkbox_list.get(current_state).size(); i++){
                if(checkbox_list.get(current_state).get(i).equals(e.getSource()) && checkbox_list.get(current_state).get(i).isEnabled()){
                    checkbox_list.get(current_state).get(i).setBorder(BorderFactory.createLineBorder(new Color(39,83,93)));

                    database_words.get(current_state).get(i).setBackground(Color.white);
                    database_words.get(current_state).get(i).setForeground(Color.black);
                    database_translations.get(current_state).get(i).setBackground(Color.white);
                    database_synonyms.get(current_state).get(i).setBackground(new Color(176,179,255));
                    box_exited = true;
                   
                }
            }
        }
        else if(box_pressed && !searching && e.getSource() instanceof MyCheckBox ){
            box_exited = true;
        }
        /*-------------------searching_checkboxes------------------------*/

        if(e.getSource() instanceof MyCheckBox && searching && !box_pressed && current_state==-1 ){
            for(int i=0; i<search_database.size(); i++){
                int x = search_database.get(i).getX();
                int y = search_database.get(i).getY();
                if( checkbox_list.get(x).get(y).equals(e.getSource()) && checkbox_list.get(x).get(y).isEnabled()){
                    checkbox_list.get(x).get(y).setBorder(BorderFactory.createLineBorder(new Color(39,83,93)));

                    database_words.get(x).get(y).setBackground(Color.white);
                    database_words.get(x).get(y).setForeground(Color.black);
                    database_translations.get(x).get(y).setBackground(Color.white);
                    database_synonyms.get(x).get(y).setBackground(new Color(176,179,255));
                    box_exited = true;
                   
                }
            }
        }
        else if(box_pressed && searching && e.getSource() instanceof MyCheckBox && current_state==-1 ){
            box_exited = true;
        }

        /*------------------------------------------------------------------*/

		if(e.getSource() instanceof MenuButton){
			for(int i=0; i<menu.length; i++){
				if(menu[i].equals(e.getSource()) && !menu[i].focused()){
					menu[i].setBackground(Color.lightGray);
					menu[i].setForeground(Color.darkGray);
				}
			}
		}
		
		if(e.getSource() instanceof MyButton2 && !box_pressed){

            if(current_state>=0 && !searching){

                for(int i=0; i<database_words.get(current_state).size(); i++){
                    if(database_words.get(current_state).get(i).equals(e.getSource()) && !database_words.get(current_state).get(i).isFocused()){
                        database_words.get(current_state).get(i).setBackground(Color.white);
                        database_words.get(current_state).get(i).setForeground(Color.darkGray);
    
                        database_translations.get(current_state).get(i).setBackground(Color.white);
                        database_translations.get(current_state).get(i).setForeground(Color.darkGray);
                        
                        database_synonyms.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                        database_synonyms.get(current_state).get(i).setBackground(new Color(176,179,255));
                        database_synonyms.get(current_state).get(i).setForeground(Color.black);
                    }
                }

            }
            else if(searching && !box_pressed){

                for(int i=0; i<search_database.size(); i++){

                    int x3 = search_database.get(i).getX();
                    int y3 = search_database.get(i).getY();

                    if( database_words.get(x3).get(y3).equals(e.getSource()) && !database_words.get(x3).get(y3).isFocused()){
                        database_words.get(x3).get(y3).setBackground(Color.white);
                        database_words.get(x3).get(y3).setForeground(Color.darkGray);
    
                        database_translations.get(x3).get(y3).setBackground(Color.white);
                        database_translations.get(x3).get(y3).setForeground(Color.darkGray);
                        
                        database_synonyms.get(x3).get(y3).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                        database_synonyms.get(x3).get(y3).setBackground(new Color(176,179,255));
                        database_synonyms.get(x3).get(y3).setForeground(Color.black);
                    }
                }
            }
			
		}

		if(e.getSource() instanceof MyJTextField && !(e.getSource().equals(clicked)) && !box_pressed){

            if(current_state>=0 && !searching){

                for(int i=0; i<database_translations.get(current_state).size(); i++){

                    if( database_translations.get(current_state).get(i).equals(e.getSource()) && !database_words.get(current_state).get(i).isFocused()) {
                        database_translations.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                        database_translations.get(current_state).get(i).setBackground(Color.white);
                        database_translations.get(current_state).get(i).setForeground(Color.darkGray);
    
                        database_words.get(current_state).get(i).setBackground(Color.white);
                        database_words.get(current_state).get(i).setForeground(Color.darkGray);
    
                        database_synonyms.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                        database_synonyms.get(current_state).get(i).setBackground(new Color(176,179,255));
                        database_synonyms.get(current_state).get(i).setForeground(Color.black);
                    }
                }
            }
            else if(searching && !box_pressed) {

                for(int i=0; i<search_database.size(); i++){

                    int x4 = search_database.get(i).getX();
                    int y4 = search_database.get(i).getY();

                    if( database_translations.get(x4).get(y4).equals(e.getSource()) && !database_words.get(x4).get(y4).isFocused()) {
                        database_translations.get(x4).get(y4).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                        database_translations.get(x4).get(y4).setBackground(Color.white);
                        database_translations.get(x4).get(y4).setForeground(Color.darkGray);
    
                        database_words.get(x4).get(y4).setBackground(Color.white);
                        database_words.get(x4).get(y4).setForeground(Color.darkGray);
    
                        database_synonyms.get(x4).get(y4).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                        database_synonyms.get(x4).get(y4).setBackground(new Color(176,179,255));
                        database_synonyms.get(x4).get(y4).setForeground(Color.black);
                    }
                }
            }

            if(current_state>=0 && !searching && !box_pressed){

                for(int i=0; i<database_synonyms.get(current_state).size(); i++){
                    if( database_synonyms.get(current_state).get(i).equals(e.getSource()) && !database_words.get(current_state).get(i).isFocused()) {
                        database_synonyms.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                        database_synonyms.get(current_state).get(i).setBackground(new Color(176,179,255));
                        database_synonyms.get(current_state).get(i).setForeground(Color.black);
    
                        database_words.get(current_state).get(i).setBackground(Color.white);
                        database_words.get(current_state).get(i).setForeground(Color.darkGray);
    
                        database_translations.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                        database_translations.get(current_state).get(i).setBackground(Color.white);
                        database_translations.get(current_state).get(i).setForeground(Color.darkGray);
                    }
                }
            }

            else if(searching && !box_pressed){

                for(int i=0; i<search_database.size(); i++){

                    int x5 = search_database.get(i).getX();
                    int y5 = search_database.get(i).getY();

                    if( database_synonyms.get(x5).get(y5).equals(e.getSource()) && !database_words.get(x5).get(y5).isFocused()) {
                        database_synonyms.get(x5).get(y5).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                        database_synonyms.get(x5).get(y5).setBackground(new Color(176,179,255));
                        database_synonyms.get(x5).get(y5).setForeground(Color.black);
    
                        database_words.get(x5).get(y5).setBackground(Color.white);
                        database_words.get(x5).get(y5).setForeground(Color.darkGray);
    
                        database_translations.get(x5).get(y5).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                        database_translations.get(x5).get(y5).setBackground(Color.white);
                        database_translations.get(x5).get(y5).setForeground(Color.darkGray);
                    }
                }
            } 

	   }


	}


	@Override
	public void actionPerformed(ActionEvent e) {

        
        if(e.getSource().equals(paint_chart)){
           evaluate();
        }

        if(e.getSource().equals(expand_button)){
            expand();
        }
        

        if(e.getSource().equals(update_info) && !searching && current_state>=0){
            update_database(0);
            update_info(1);
            
        }
        else if(e.getSource().equals(update_info) && searching && current_state==-1){
            
            update_search_database(0);
            update_search_info(1);
        }

		if(current_state>=0 && e.getSource() instanceof MenuButton){
            for(int i=0; i<menu.length; i++){
		    	if(e.getSource().equals(menu[i])){
		    		typing="";
				    search.setText("");
		    		update_database(0);
		    		break;
		    	}
		    }
		}

		for(int i=0; i<menu.length; i++){
			if(e.getSource().equals(menu[i])){
                //playSound("Sounds\\switch34.wav");
                playSound("Sounds\\click.wav");
				typing="";
				search.setText("");
				menu[i].setBackground(Color.darkGray);
				menu[i].setForeground(Color.white);
				menu[i].setFocus(true);
				for(int j=0; j<menu.length; j++){
					if(menu[j]!=menu[i]){
						menu[j].setBackground(Color.lightGray);
						menu[j].setForeground(Color.darkGray);
						menu[j].setFocus(false);
					}
				}
                if(current_state>=0){
                    check_clicked_word();
                }
                
				create_gui(i);
			}
		}

		if(e.getSource().equals(add)){
			if(!writing.getText().equals("") && writing.getText()!=null && !writing.getText().equals("Add word...")){
				if(current_state!=-1){
					update_database(0);
                    if(check_word(writing.getText())){
                        check_clicked_word();
                        add_word(writing.getText());
                        create_gui(current_state);
                    }
					
				}
				else{
                     if(check_word(writing.getText())){
                        check_clicked_word();
                        add_word2(writing.getText());
                        create_gui(current_state);
                    }
				}
				for(int j=0; j<menu.length; j++){
					if(j==current_state){
						menu[j].setBackground(Color.darkGray);
						menu[j].setFocus(true);
						menu[j].setForeground(Color.white);

					}
					if(j!=current_state){
						menu[j].setBackground(Color.lightGray);
						menu[j].setForeground(Color.darkGray);
						menu[j].setFocus(false);
					}
				}
                writing.setText("Add word...");
			}
           
		}

		if(e.getSource().equals(update)){
			if(current_state>=0){
				update_database(1);
				create_gui(current_state);
			}
			else if(searching && current_state==-1){
				update_search_database(1);
			}

		}
	}

    /*-------------FOCUS_LISTENER_________________________*/
    
    public void focusGained(FocusEvent e) { 
    }
      
    public void focusLost(FocusEvent e) {
    }
    /*------------Component_listener----------------------*/
    public void componentHidden(ComponentEvent e) {
        
    }

    public void componentMoved(ComponentEvent e) {
       
    }

    public void componentResized(ComponentEvent e) {
      
    }

    public void componentShown(ComponentEvent e) {
        

    }
    
   /*----------------------------UPDATE_DATABASE_SEARCHING_---------------------------------------------------*/

    public void update_search_database(int option){

        for(int i=0; i<search_database.size(); i++){

            int x = search_database.get(i).getX();
            int y = search_database.get(i).getY();

            database.get(x).get(y).clear_synonyms();
            database.get(x).get(y).clear_translations();

            ArrayList<String> syn12 = new ArrayList<String>();
            ArrayList<String> trans12 = new ArrayList<String>();

            syn12.add(database_synonyms.get(x).get(y).getText());
            trans12.add(database_translations.get(x).get(y).getText());

            database_words.get(x).get(y).get_word().clear_synonyms();
            database_words.get(x).get(y).get_word().clear_translations();

            database_words.get(x).get(y).get_word().add_synonyms(syn12);
            database_words.get(x).get(y).get_word().add_translations(trans12);
            
            if(syn12.size()>0){
                database_synonyms.get(x).get(y).setText(syn12.get(0));
            }
            else{
                database_synonyms.get(x).get(y).setText("");
            }
            if(trans12.size()>0){
                database_translations.get(x).get(y).setText(trans12.get(0));
            }
            else{
                database_translations.get(x).get(y).setText("");
            }
            
            database.get(x).get(y).add_synonyms(syn12);
            database.get(x).get(y).add_translations(trans12);
        }
        if(option==1){
            
            data.write_database(database);
            playSound("Sounds\\save.wav");
        }
        

    }

    /*-----------------------------UPDATE_DATA_FUNCTION-------------------------------*/
	public void update_database(int option){

         if(current_state>=0){ 
            
		    for(int j=0; j<database.get(current_state).size(); j++){

		        			database.get(current_state).get(j).clear_synonyms();
		        			database.get(current_state).get(j).clear_translations();

		        			ArrayList<String> syn1 = new ArrayList<String>();
		        			ArrayList<String> trans1 = new ArrayList<String>();

                            syn1.add(database_synonyms.get(current_state).get(j).getText());
                            trans1.add(database_translations.get(current_state).get(j).getText());

		        			database.get(current_state).get(j).add_synonyms(syn1);
		        			database.get(current_state).get(j).add_translations(trans1);

                            database_words.get(current_state).get(j).get_word().clear_translations();
                            database_words.get(current_state).get(j).get_word().clear_synonyms();

                            database_words.get(current_state).get(j).get_word().add_synonyms(syn1);
                            database_words.get(current_state).get(j).get_word().add_translations(trans1);

                            if(syn1.size()>0){
                                database_synonyms.get(current_state).get(j).setText(syn1.get(0));
                            }
                            else{
                                database_synonyms.get(current_state).get(j).setText("");
                            }
                            if(trans1.size()>0){
                                database_translations.get(current_state).get(j).setText(trans1.get(0));
                            }
                            else{
                                database_translations.get(current_state).get(j).setText("");
                            }

		    }
            if(option==1){
            
         
            data.write_database(database);
            playSound("Sounds\\save.wav");
            }  		
           
		}

	}
    /*-------------------------------check_word-------------------------------------*/
    public boolean check_word(String wrd){

        if(wrd.matches("[a-zA-Z]+")) {
              String letter = String.valueOf(wrd.charAt(0)).toUpperCase();
              for(int i=0; i<alphabet.length; i++){
                  if(alphabet[i].equals(letter)){
                      for(int j=0; j<database_words.get(i).size(); j++){
                          if(database_words.get(i).get(j).get_word().get_word().toLowerCase().equals(wrd.toLowerCase())){
                              playSound("Sounds\\error.wav");
                              JOptionPane.showMessageDialog(null,"Word Already exists!","Invalid_Input_ERRORx01",  JOptionPane.WARNING_MESSAGE);
                              return false;
                              
                          }
                      }
                  }

              }
              return true;
        }
        else{
          playSound("Sounds\\error.wav");
          JOptionPane.showMessageDialog(null,"Word contains non-valid characters!","Invalid_Input_ERRORx02",  JOptionPane.WARNING_MESSAGE);
          return false;
        }

    }

    /*-------------------------------ADD_FUNCTIONS_---------------------------------*/
	public void add_word(String wordd){

		for(int i=0; i<alphabet.length; i++){
			if(alphabet[i].toLowerCase().equals(String.valueOf(wordd.charAt(0)).toLowerCase())){
				wordd = String.valueOf(wordd.charAt(0)).toUpperCase()+wordd.substring(1,wordd.length());
				ArrayList<String> s1 = new ArrayList<String>();
				database.get(i).add(new word(wordd,s1,s1,s1));

                MyButton2 but = new MyButton2(wordd,new word(wordd,s1,s1,s1));
                but.addMouseListener(this);
                but.addActionListener(this);
                database_words.get(i).add(but);
                
                MyJTextField jtxt = new MyJTextField("");
                jtxt.setFont(new Font("Arial", Font.PLAIN, 16));
                jtxt.setCursor(Cursor.getDefaultCursor());
                jtxt.setBorder(BorderFactory.createLoweredSoftBevelBorder());
                jtxt.setHorizontalAlignment(JTextField.CENTER);
                jtxt.setPreferredSize(new Dimension(200,22));
                jtxt.addActionListener(this);
                jtxt.addMouseListener(this);
                jtxt.addKeyListener(this);
                jtxt.setBackground(new Color(176,179,255));
                setForeground(Color.black);
                database_synonyms.get(i).add(jtxt);
                
                MyJTextField jtxt2 = new MyJTextField("");
                jtxt2.setFont(new Font("Arial", Font.PLAIN, 16));
                jtxt2.setCursor(Cursor.getDefaultCursor());
                jtxt2.setBorder(BorderFactory.createLoweredSoftBevelBorder());
                jtxt2.setHorizontalAlignment(JTextField.CENTER);
                jtxt2.setPreferredSize(new Dimension(200,22));
                jtxt2.addActionListener(this);
                jtxt2.addMouseListener(this);
                jtxt2.addKeyListener(this);
                jtxt2.setBackground(Color.white);
                jtxt2.setForeground(Color.black);
                database_translations.get(i).add(jtxt2);

                MyCheckBox temp_cb = new MyCheckBox();
                temp_cb.addMouseListener(this);
                temp_cb.addActionListener(this);
                checkbox_list.get(i).add(temp_cb);
                

				current_state = i;
                update_database(0);
                playSound("Sounds\\success2.wav");
                JOptionPane.showMessageDialog(null,"Word added successfully!","Update_Dialog",  JOptionPane.PLAIN_MESSAGE);
                check_clicked_word();

                /*sorting_method*/
                ArrayList<word> temp1 = new ArrayList<word>(database.get(i));
                ArrayList<MyButton2> temp2 = new ArrayList<MyButton2>(database_words.get(i));

                database.remove(i);
                database_words.remove(i);

                database.add(i,new ArrayList<word>(sort_words(temp1)));
                database_words.add(i,new ArrayList<MyButton2>(sort_buttons(temp2)));
                reload_translations_synonyms(i);
                //database.get(i) = new ArrayList<word>(sort_words(database.get(i)));
                //database_words.get(i) = new ArrayList<MyButton2>(sort_buttons(database_words.get(i)));
                total_words++;
              
                data.write_database(database);
                

				break;
			}
		}if(expanded){
            evaluate();}		
	}

	public void add_word2(String wordd){

		for(int i=0; i<alphabet.length; i++){
			if(alphabet[i].toLowerCase().equals(String.valueOf(wordd.charAt(0)).toLowerCase())){
				wordd = String.valueOf(wordd.charAt(0)).toUpperCase()+wordd.substring(1,wordd.length());
				ArrayList<String> s1 = new ArrayList<String>();
				database.get(i).add(new word(wordd,s1,s1,s1));

                MyButton2 but = new MyButton2(wordd,new word(wordd,s1,s1,s1));
                but.addMouseListener(this);
                but.addActionListener(this);
                database_words.get(i).add(but);
                
                MyJTextField jtxt = new MyJTextField("");
                jtxt.setFont(new Font("Arial", Font.PLAIN, 16));
                jtxt.setCursor(Cursor.getDefaultCursor());
                jtxt.setBorder(BorderFactory.createLoweredSoftBevelBorder());
                jtxt.setHorizontalAlignment(JTextField.CENTER);
                jtxt.setPreferredSize(new Dimension(200,22));
                jtxt.addActionListener(this);
                jtxt.addMouseListener(this);
                jtxt.addKeyListener(this);
                jtxt.setBackground(new Color(176,179,255));
                setForeground(Color.black);
                database_synonyms.get(i).add(jtxt);
                
                MyJTextField jtxt2 = new MyJTextField("");
                jtxt2.setFont(new Font("Arial", Font.PLAIN, 16));
                jtxt2.setCursor(Cursor.getDefaultCursor());
                jtxt2.setBorder(BorderFactory.createLoweredSoftBevelBorder());
                jtxt2.setHorizontalAlignment(JTextField.CENTER);
                jtxt2.setPreferredSize(new Dimension(200,22));
                jtxt2.addActionListener(this);
                jtxt2.addMouseListener(this);
                jtxt2.addKeyListener(this);
                jtxt2.setBackground(Color.white);
                jtxt2.setForeground(Color.black);
                database_translations.get(i).add(jtxt2);
                
                MyCheckBox temp_cb = new MyCheckBox();
                temp_cb.addMouseListener(this);
                temp_cb.addActionListener(this);
                checkbox_list.get(i).add(temp_cb);

				current_state = i;
                update_database(0);
                playSound("Sounds\\success2.wav");
                JOptionPane.showMessageDialog(null,"Word added successfully!","Update_Dialog",  JOptionPane.PLAIN_MESSAGE);
                check_clicked_word();

                /*sorting_method*/
                ArrayList<word> temp1 = new ArrayList<word>(database.get(i));
                ArrayList<MyButton2> temp2 = new ArrayList<MyButton2>(database_words.get(i));

                database.remove(i);
                database_words.remove(i);

                database.add(i,new ArrayList<word>(sort_words(temp1)));
                database_words.add(i,new ArrayList<MyButton2>(sort_buttons(temp2)));
                reload_translations_synonyms(i);
                //database.get(i) = new ArrayList<word>(sort_words(database.get(i)));
                //database_words.get(i) = new ArrayList<MyButton2>(sort_buttons(database_words.get(i)));

                
            
         
                data.write_database(database);
                
                total_words++;
				break;
			}
		}
        if(expanded){
            evaluate();}    
	}


	/*----------------Sorting_Function-----------------------------*/

	public ArrayList<Pointer> sort_by_increasing(ArrayList<Pointer> list){

		int size = list.size();
		String[] names = new String[size];
		for(int i=0; i<size; i++){
			names[i] = list.get(i).getWord().get_word();
		}
		Arrays.sort(names);
		ArrayList<Pointer> new_list = new ArrayList<Pointer>();

		for(int i=0; i<names.length; i++){
            for(int j=0; j<list.size(); j++){
                if(list.get(j).getWord().get_word().equals(names[i])){
                    new_list.add(list.get(j));
                    break;
                }
            }
        }

		return new_list;
	}


    

   /*---------------------Initialize_labels_buttons-------------------------*/

   public void initialize_labels_buttons(){

        expand_button = new MyCheckBox2();
        expand_button.addMouseListener(this);
        expand_button.addActionListener(this);
        expand_button.addKeyListener(this);

        add = new MyButton("Add Word");
        add.addMouseListener(this);
        add.addActionListener(this);
        add.addFocusListener(this);

        update = new MyButton("Update Data");
        update.addMouseListener(this);
        update.addActionListener(this);
        update.setFocusable(true);

        writing = new JTextField("",230);
        writing.setToolTipText("Type a word to add here..");
        writing.addFocusListener(this);
        writing.addKeyListener(this);
        writing.addActionListener(this);
        writing.addKeyListener(this);
        writing.addMouseListener(this);
        writing.setPreferredSize(new Dimension(230,24));
        writing.setBorder(BorderFactory.createLoweredSoftBevelBorder());
        writing.setText("Add word...");
        
        remove = new MyButton("Remove");

        leksh = new JLabel("WORD");
        leksh.setFont(new Font("default",Font.BOLD,16));
        leksh.setHorizontalAlignment(SwingConstants.CENTER);
        leksh.setBackground(Color.gray);
        leksh.setOpaque(true);
        leksh.setForeground(Color.white);

        transl = new JLabel("TRANSLATION");
        transl.setOpaque(true);
        transl.setHorizontalAlignment(SwingConstants.CENTER);
        transl.setBackground(Color.gray);
        transl.setForeground(Color.white);
        transl.setFont(new Font("default",Font.BOLD,16));

        synwnymo = new JLabel("SYNONYM");
        synwnymo.setFont(new Font("default",Font.BOLD,16));
        synwnymo.setHorizontalAlignment(SwingConstants.CENTER);
        synwnymo.setBackground(Color.gray);
        synwnymo.setOpaque(true);
        synwnymo.setForeground(Color.white);
        
        
   }
  

    /*-----------------initialize_GUI-----------------------*/

    public void initialize_temps(){

        for(ArrayList<word> w : database){
            total_words+=w.size();
        }
    }

    public void initialize_gui(){

        database_words.clear();
        database_synonyms.clear();
        database_translations.clear();
        checkbox_list.clear();

        for(int i=0; i<database.size(); i++){

            database_words.add(new ArrayList<MyButton2>());
            database_synonyms.add(new ArrayList<MyJTextField>());
            database_translations.add(new ArrayList<MyJTextField>());
            checkbox_list.add(new ArrayList<MyCheckBox>());

            for(int j=0; j<database.get(i).size(); j++){
                database_words.get(i).add(new MyButton2(database.get(i).get(j).get_word(),database.get(i).get(j)));
                database_words.get(i).get(j).addMouseListener(this);
                database_words.get(i).get(j).addActionListener(this);
                database_words.get(i).get(j).addKeyListener(this);

                if(database.get(i).get(j).get_translations().size()>0){
                    ArrayList<String> trl = new ArrayList<String>(database.get(i).get(j).get_translations());
                    database_translations.get(i).add(new MyJTextField(trl.get(0)));
                }
                else{
                    database_translations.get(i).add(new MyJTextField(""));
                }
                if(database.get(i).get(j).get_synonyms().size()>0){
                    ArrayList<String> synss = new ArrayList<String>(database.get(i).get(j).get_synonyms());
                    database_synonyms.get(i).add(new MyJTextField(synss.get(0)));
                }
                else{
                    database_synonyms.get(i).add(new MyJTextField(""));
                }
                database_synonyms.get(i).get(j).setFont(new Font("Arial", Font.PLAIN, 16));
                database_synonyms.get(i).get(j).setCursor(Cursor.getDefaultCursor());
                database_synonyms.get(i).get(j).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                database_synonyms.get(i).get(j).setHorizontalAlignment(JTextField.CENTER);
                database_synonyms.get(i).get(j).setPreferredSize(new Dimension(200,22));
                database_synonyms.get(i).get(j).addActionListener(this);
                database_synonyms.get(i).get(j).addMouseListener(this);
                database_synonyms.get(i).get(j).addKeyListener(this);
                database_synonyms.get(i).get(j).setBackground(new Color(176,179,255));
                database_synonyms.get(i).get(j).setForeground(Color.black);

                database_translations.get(i).get(j).setFont(new Font("Arial", Font.PLAIN, 16));
                database_translations.get(i).get(j).setCursor(Cursor.getDefaultCursor());
                database_translations.get(i).get(j).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                database_translations.get(i).get(j).setHorizontalAlignment(JTextField.CENTER);
                database_translations.get(i).get(j).setPreferredSize(new Dimension(200,22));
                database_translations.get(i).get(j).addActionListener(this);
                database_translations.get(i).get(j).addMouseListener(this);
                database_translations.get(i).get(j).addKeyListener(this);
                database_translations.get(i).get(j).setBackground(Color.white);
                database_translations.get(i).get(j).setForeground(Color.black);

                checkbox_list.get(i).add(new MyCheckBox());
                checkbox_list.get(i).get(j).addActionListener(this);
                checkbox_list.get(i).get(j).addKeyListener(this);
                checkbox_list.get(i).get(j).addMouseListener(this);

            }
        }


    }

    public void initialize_charts_panel(Insets insets){

        int distan = 0;
        for(int i=0; i<chart_array.length; i++){

            chart_array[i] = new MyChartField(" "+alphabet[i]);
            chart_array[i].setForeground(Color.lightGray);
            chart_array[i].setEditable(false);
            chart_array[i].setCursor(Cursor.getDefaultCursor());
            chart_array[i].setBackground(new Color(63,128,193)); //91,155,219
            chart_array[i].setBorder(null);
            chart_array[i].setBounds(insets.left,insets.top+1+distan,15,13);
            chart_array[i].addMouseListener(this);
            chart_array[i].addActionListener(this);
            distan = distan + 15;
            charts.add(chart_array[i]);
        }
    }

    public void check_clicked_word(){

        if(current_state>=0){
            for(int i=0; i<database_words.get(current_state).size(); i++){
               
                database_words.get(current_state).get(i).setBorder(null);
                database_synonyms.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                database_translations.get(current_state).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                database_words.get(current_state).get(i).setBackground(Color.white);
                database_words.get(current_state).get(i).setForeground(Color.black);
                database_translations.get(current_state).get(i).setBackground(Color.white);
                database_translations.get(current_state).get(i).setFocusable(true);
                database_synonyms.get(current_state).get(i).setFocusable(true);
                database_synonyms.get(current_state).get(i).setBackground(new Color(176,179,255));
                database_words.get(current_state).get(i).setFocus(false);
                checkbox_list.get(current_state).get(i).setEnabled(true);
               
            }
        }
        

        if(!closed){
            reset_info_panel();
        }
    }

    public void check_clicked_word_search(){

        if(current_state==-1){
                for(int i=0; i<search_database.size(); i++){
                    int x = search_database.get(i).getX();
                    int y = search_database.get(i).getY();
    
                    database_words.get(x).get(y).setBorder(null);
                    database_synonyms.get(x).get(y).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                    database_translations.get(x).get(y).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                    database_words.get(x).get(y).setBackground(Color.white);
                    database_words.get(x).get(y).setForeground(Color.black);
                    database_translations.get(x).get(y).setBackground(Color.white);
                    database_translations.get(x).get(y).setFocusable(true);
                    database_synonyms.get(x).get(y).setFocusable(true);
                    database_synonyms.get(x).get(y).setBackground(new Color(176,179,255));
                    database_words.get(x).get(y).setFocus(false);
                    checkbox_list.get(x).get(y).setEnabled(true);
                   
                }
            if(!closed){
                reset_info_panel();
            }
        }
    }


    public void reset_info_panel(){

        info.removeAll();
        int height = getHeight();
        for(int i=height; i>main_height; i=i-1){
                setSize(getWidth(),i);
                
        }
        setSize(getWidth(),this.getHeight()-2);
        
        info.setBounds(main.getInsets().left+219-40-40,main.getInsets().top+674+27+50-40-20-100-15,652,120-5);
        closed = true;

        main.revalidate();
        main.repaint();
    }

    public void delete_word(int x){

        if(current_state>=0){

            String word_name = database_words.get(current_state).get(x).get_word().get_word();


            database_words.get(current_state).remove(x);
            database_synonyms.get(current_state).remove(x);
            database_translations.get(current_state).remove(x);
            checkbox_list.get(current_state).remove(x);

            database.get(current_state).remove(x);
            data.delete_word_files(word_name,current_state);
            
        
            data.write_database(database);
            
            total_words--;
            create_gui(current_state);
            if(expanded && !searching){
            evaluate();
            }

        }
    }

    public void delete_word2(int x,int y){

        if(current_state==-1 && searching){

            String word_name = database_words.get(x).get(y).get_word().get_word();


            database_words.get(x).remove(y);
            database_synonyms.get(x).remove(y);
            database_translations.get(x).remove(y);
            checkbox_list.get(x).remove(y);

            database.get(x).remove(y);
            data.delete_word_files(word_name,x);
      
            data.write_database(database);

            create_gui(typing);
            
        }
    }


    public static int dialogue_ok_cancel(String theMessage) {
    int result = JOptionPane.showConfirmDialog((Component) null, theMessage,
        "", JOptionPane.YES_NO_OPTION);
    return result;
  }

  public void update_info(int option){
    if(current_state>=0){

    String name="";
    String translation="";
    String synonym ="";
    String notes ="";
    word word0 =temp_wordd;
    int x0 =0;
    int y0 =0;
    int pointer = -1;
    boolean changed =false;
    boolean error = false;

    for(int i=0; i<database_words.get(current_state).size(); i++){
        if(database_words.get(current_state).get(i).get_word().equals(word0)){
            x0 = current_state;
            y0 = i;
            changed = false;
            break;
        }
    }
    name = info_word.getText();
    if(name.equals("")){
        error = true;
    }
    else{
        name = String.valueOf(name.charAt(0)).toUpperCase()+name.substring(1,name.length());
    }
    

    translation = info_translations.getText();
    synonym = info_synonyms.getText();
    notes = info_notes.getText();

    if(!name.equals(database.get(x0).get(y0)) && !error){
        changed = true;
        if( String.valueOf(name.charAt(0)).toLowerCase().equals(alphabet[x0].toLowerCase()) && name.matches("[a-zA-Z]+")){
            for(int k=0; k<database_words.get(x0).size(); k++){
                if(name.toLowerCase().equals(database_words.get(x0).get(k).get_word().get_word().toLowerCase()) && !database_words.get(x0).get(k).get_word().equals(database_words.get(x0).get(y0).get_word()) ){
                    error = true;
                    break;
                }
                else{
                    error = false;
                }
            }    
        }
        else{
            error = true;
        }
    }


    database.get(x0).get(y0).clear_translations();
    database.get(x0).get(y0).clear_synonyms();
    database.get(x0).get(y0).clear_notes();

    ArrayList<String> trans = new ArrayList<String>();
    ArrayList<String> syns = new ArrayList<String>();
    ArrayList<String> nots = new ArrayList<String>();

    trans.add(info_translations.getText());
    syns.add(info_synonyms.getText());
    nots.add(info_notes.getText());
    
    
    database.get(x0).get(y0).add_translations(trans);
    database.get(x0).get(y0).add_synonyms(syns);
    database.get(x0).get(y0).add_notes(nots);

    if(!error){
        if(option == 1){
            playSound("Sounds\\save.wav");
            help.setForeground(Color.green);
            help.setText("~Thank you!");
        }
        database.get(x0).get(y0).set_Name(name);
        database_words.get(x0).get(y0).get_word().set_Name(name);
        database_words.get(x0).get(y0).setText(name); 
    }
    else{
            playSound("Sounds\\error.wav");
        
        
        JOptionPane.showMessageDialog(null,"Invalid word entered!\nPlease try again!","Invalid_Input_ERRORx03",  JOptionPane.WARNING_MESSAGE);
        info_word.setText(database_words.get(x0).get(y0).get_word().get_word());
    }
    database_words.get(x0).get(y0).get_word().clear_translations();
    database_words.get(x0).get(y0).get_word().clear_synonyms();
    database_words.get(x0).get(y0).get_word().clear_notes();
    
    database_words.get(x0).get(y0).get_word().add_synonyms(syns);
    database_words.get(x0).get(y0).get_word().add_translations(trans);
    database_words.get(x0).get(y0).get_word().add_notes(nots);

    if(syns.size()>0){
        database_synonyms.get(x0).get(y0).setText(syns.get(0));
    }
    if(trans.size()>0){
        database_translations.get(x0).get(y0).setText(trans.get(0));
    }
    
    database_words.get(x0).get(y0).get_word().add_notes(nots);
    //data.write_database(database);    
    if(changed && !error){
         info_word.setText(name);
         ArrayList<word> temp1 = new ArrayList<word>(database.get(x0));
         ArrayList<MyButton2> temp2 = new ArrayList<MyButton2>(database_words.get(x0));
         database.remove(x0);
         database_words.remove(x0);
         database.add(x0,new ArrayList<word>(sort_words(temp1)));
         database_words.add(x0,new ArrayList<MyButton2>(sort_buttons(temp2)));

         for(int i=0; i<database_words.get(x0).size(); i++){
             if(database_words.get(x0).get(i).get_word().get_word().equals(name)){
                 pointer = i;
                 database_words.get(x0).get(i).setFocus(true);
                 database_words.get(x0).get(i).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
                 database_words.get(x0).get(i).setBackground(Color.darkGray);
                 database_words.get(x0).get(i).setForeground(Color.white);
                 database_synonyms.get(x0).get(i).setBackground(new Color(153,156,246)); //138,154,255//153,156,246
                 database_synonyms.get(x0).get(i).setForeground(Color.black);
                 database_synonyms.get(x0).get(i).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
                 database_synonyms.get(x0).get(i).setFocusable(false);
                 database_translations.get(x0).get(i).setBackground(new Color(195,195,195));
                 database_translations.get(x0).get(i).setForeground(Color.black);
                 database_translations.get(x0).get(i).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
                 database_translations.get(x0).get(i).setFocusable(false);
                 checkbox_list.get(x0).get(i).setEnabled(false);
                 
             }
             else{
                 database_words.get(x0).get(i).setBorder(null);
                 database_synonyms.get(x0).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                 database_translations.get(x0).get(i).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                 database_words.get(x0).get(i).setBackground(Color.white);
                 database_words.get(x0).get(i).setForeground(Color.black);
                 database_translations.get(x0).get(i).setBackground(Color.white);
                 database_translations.get(x0).get(i).setFocusable(true);
                 database_synonyms.get(x0).get(i).setFocusable(true);
                 database_synonyms.get(x0).get(i).setBackground(new Color(176,179,255));
                 database_words.get(x0).get(i).setFocus(false);
                 checkbox_list.get(x0).get(i).setEnabled(true);
                 
             }

        
        }
        reload_translations_synonyms(x0); 
    }


    create_gui(x0);
  }

 }

  public void update_search_info(int option){
  if(current_state==-1){
    String name="";
    String translation="";
    String synonym ="";
    String notes ="";
    word word0 =temp_wordd;
    int x0 =0;
    int y0 =0;
    boolean changed = false;
    boolean error = false;

    for(int i=0; i<search_database.size(); i++){
        x0 = search_database.get(i).getX();
        y0 = search_database.get(i).getY();
        if(database_words.get(x0).get(y0).get_word().equals(word0)){
            x0 = search_database.get(i).getX();
            y0 = search_database.get(i).getY();
            break;
        }                  
    }
    

    name = info_word.getText();
    if(name.equals("")){
        error = true;
    }
    else{
        name = String.valueOf(name.charAt(0)).toUpperCase()+name.substring(1,name.length());
    }
    

    translation = info_translations.getText();
    synonym = info_synonyms.getText();
    notes = info_notes.getText();

    if(!name.equals(database.get(x0).get(y0)) && !error){
        changed = true;
        if( String.valueOf(name.charAt(0)).toLowerCase().equals(alphabet[x0].toLowerCase()) && name.matches("[a-zA-Z]+")){
            for(int k=0; k<database_words.get(x0).size(); k++){
                if(name.toLowerCase().equals(database_words.get(x0).get(k).get_word().get_word().toLowerCase()) && !database_words.get(x0).get(k).get_word().equals(database_words.get(x0).get(y0).get_word()) ){
                    error = true;
                    break;
                }
                else{
                    error = false;
                }
            }    
        }
        else{
            error = true;
        }
    }

    database.get(x0).get(y0).clear_translations();
    database.get(x0).get(y0).clear_synonyms();
    database.get(x0).get(y0).clear_notes();

    ArrayList<String> trans = new ArrayList<String>();
    ArrayList<String> syns = new ArrayList<String>();
    ArrayList<String> nots = new ArrayList<String>();

    trans.add(info_translations.getText());
    syns.add(info_synonyms.getText());
    nots.add(info_notes.getText());
    
    
    database.get(x0).get(y0).add_translations(trans);
    database.get(x0).get(y0).add_synonyms(syns);
    database.get(x0).get(y0).add_notes(nots);

    if(!error){
        if(option==1){
            playSound("Sounds\\save.wav");
            help.setForeground(Color.green);
            help.setText("~Thank you!");
        }
        database.get(x0).get(y0).set_Name(name);
        database_words.get(x0).get(y0).get_word().set_Name(name);
        database_words.get(x0).get(y0).setText(name);
    }
    else{
        playSound("Sounds\\error.wav");
        JOptionPane.showMessageDialog(null,"Invalid word entered!\nPlease try again!","Invalid_Input_ERRORx03",  JOptionPane.WARNING_MESSAGE);
        info_word.setText(database_words.get(x0).get(y0).get_word().get_word());
    }
    
    database_words.get(x0).get(y0).get_word().clear_translations();
    database_words.get(x0).get(y0).get_word().clear_synonyms();
    database_words.get(x0).get(y0).get_word().clear_notes();
    
    database_words.get(x0).get(y0).get_word().add_synonyms(syns);
    database_words.get(x0).get(y0).get_word().add_translations(trans);
    database_words.get(x0).get(y0).get_word().add_notes(nots);

    if(syns.size()>0){
        database_synonyms.get(x0).get(y0).setText(syns.get(0));
    }
    if(trans.size()>0){
        database_translations.get(x0).get(y0).setText(trans.get(0));
    }
    
    database_words.get(x0).get(y0).get_word().add_notes(nots);

    if(changed && !error && name.toLowerCase().contains(typing.toLowerCase())){
        info_word.setText(name);
        info_word.setText(name);
        ArrayList<word> temp1 = new ArrayList<word>(database.get(x0));
        ArrayList<MyButton2> temp2 = new ArrayList<MyButton2>(database_words.get(x0));
        database.remove(x0);
        database_words.remove(x0);
        database.add(x0,new ArrayList<word>(sort_words(temp1)));
        database_words.add(x0,new ArrayList<MyButton2>(sort_buttons(temp2)));

        for(int j=0; j<search_database.size(); j++){
            int x1 = search_database.get(j).getX();
            int y1 = search_database.get(j).getY();

            if(database_words.get(x1).get(y1).get_word().get_word().equals(name)){
                 
                 database_words.get(x1).get(y1).setFocus(true);
                 database_words.get(x1).get(y1).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
                 database_words.get(x1).get(y1).setBackground(Color.darkGray);
                 database_words.get(x1).get(y1).setForeground(Color.white);
                 database_synonyms.get(x1).get(y1).setBackground(new Color(153,156,246)); //138,154,255//153,156,246
                 database_synonyms.get(x1).get(y1).setForeground(Color.black);
                 database_synonyms.get(x1).get(y1).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
                 database_synonyms.get(x1).get(y1).setFocusable(false);
                 database_translations.get(x1).get(y1).setBackground(new Color(195,195,195));
                 database_translations.get(x1).get(y1).setForeground(Color.black);
                 database_translations.get(x1).get(y1).setBorder(BorderFactory.createLineBorder(new Color(75,255,178)));
                 database_translations.get(x1).get(y1).setFocusable(false);
                 checkbox_list.get(x1).get(y1).setEnabled(false);
                 
             }
             else{
                 database_words.get(x1).get(y1).setBorder(null);
                 database_synonyms.get(x1).get(y1).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                 database_translations.get(x1).get(y1).setBorder(BorderFactory.createLoweredSoftBevelBorder());
                 database_words.get(x1).get(y1).setBackground(Color.white);
                 database_words.get(x1).get(y1).setForeground(Color.black);
                 database_translations.get(x1).get(y1).setBackground(Color.white);
                 database_translations.get(x1).get(y1).setFocusable(true);
                 database_synonyms.get(x1).get(y1).setFocusable(true);
                 database_synonyms.get(x1).get(y1).setBackground(new Color(176,179,255));
                 database_words.get(x1).get(y1).setFocus(false);
                 checkbox_list.get(x1).get(y1).setEnabled(true);
                 
             }
        }
        
        reload_translations_synonyms(x0); 
        
        
    }else if(changed && !error && !name.toLowerCase().contains(typing.toLowerCase()) && !translation.toLowerCase().contains(typing.toLowerCase()) && !synonym.toLowerCase().contains(typing.toLowerCase())){
        check_clicked_word_search();
        reset_info_panel();
    }
    
    //data.write_database(database);
    create_gui(typing);
  }
}





  public void playSound(String soundName){
   try 
   {
    AudioInputStream audioInputStream = AudioSystem.getAudioInputStream(new File(soundName).getAbsoluteFile( ));
    Clip clip = AudioSystem.getClip( );
    clip.open(audioInputStream);
    clip.start( );
   }
   catch(Exception ex)
   {
     System.out.println("Error with playing sound.");
     ex.printStackTrace( );
   }
 }
  

 public ArrayList<word> sort_words(ArrayList<word> list){

    int size = list.size();
    String[] array = new String[size];
    for(int i=0; i<size; i++){
        array[i] = list.get(i).get_word();
    }
    Arrays.sort(array);
    ArrayList<word> sorted_list = new ArrayList<word>();
    for(int i=0; i<size; i++){
        for(int j=0; j<list.size(); j++){
            if(list.get(j).get_word().equals(array[i])){
                sorted_list.add(list.get(j));
            }
        }
    }
    return sorted_list;
 }

 public ArrayList<MyButton2> sort_buttons(ArrayList<MyButton2> list){

    int size = list.size();
    String[] array = new String[size];
    for(int i=0; i<size; i++){
        array[i] = list.get(i).get_word().get_word();
    }
    Arrays.sort(array);
    ArrayList<MyButton2> sorted_list = new ArrayList<MyButton2>();
    for(int i=0; i<size; i++){
        for(int j=0; j<list.size(); j++){
            if(list.get(j).get_word().get_word().equals(array[i])){
                sorted_list.add(list.get(j));
            }
        }
    }
    return sorted_list;
 }

 public void reload_translations_synonyms(int x){

    ArrayList<String> trans = new ArrayList<String>();
    ArrayList<String> syns = new ArrayList<String>();

    for(int i=0; i<database_words.get(x).size(); i++){

        trans = new ArrayList<String>(database_words.get(x).get(i).get_word().get_translations());
        syns = new ArrayList<String>(database_words.get(x).get(i).get_word().get_synonyms());

        if(trans.size()>0){
            database_translations.get(x).get(i).setText(trans.get(0));
        }
        else{
            database_translations.get(x).get(i).setText("");
        }
        if(syns.size()>0){
            database_synonyms.get(x).get(i).setText(syns.get(0));
        }
        else{
            database_synonyms.get(x).get(i).setText("");
        }
        
    }
 }

 public void expand(){
    
    if(getWidth()==main_width){

        expand_button.set_expanded(true);
        for(int i=main_width; i<=1080-10; i=i+2){
            setSize(i,getHeight());
        }
        setSize(1080-10,getHeight());
        expanded = true;
        evaluate();

        
    }
    else if(getWidth()>main_width){

        expand_button.set_expanded(false);
        int width = getWidth();
        for(int i=width; i>=main_width; i=i-2){
            setSize(i,getHeight());
        }
        setSize(main_width,getHeight());
        expanded = false;

        restart_gui_charts();
        restart_charts();
        
    }

    
    

    main.revalidate();
    main.repaint();
 }


 public void evaluate(){ //poses lekseis yparxoun se ka9e gramma *pososta
    for(int i=0; i<database.size(); i++){
        chart_array[i].setToolTipText(database.get(i).size()+" words of total "+total_words);
        double q = (double)database.get(i).size()/total_words;
        q = q*242.0;
        int q0 = (int)q;
        xi_max[i] = q0;
    }
    timer.start();
 }


 public void search_graph(){
    int total_x=search_database.size();
    for(int i=0; i<search_database.size(); i++){
        int x1 = search_database.get(i).getX();
        xi_max[x1]++;
    }
    for(int i=0; i<26; i++){
        chart_array[i].setToolTipText("Words containing <"+typing+"> : "+xi_max[i]);
        double q = (double)xi_max[i]/total_x;
        q = q*242.0;
        int q0 = (int)q;
        xi_max[i] = q0;
    }
    timer.start();

 }


 public void restart_charts(){
    for(int i=0; i<26; i++){
        xi_max[i] = 0;
        
    }
 }

 public void restart_gui_charts(){ /* restarting gui graph to starting point*/
    for(int i=0; i<26; i++){
        xi_max[i] = 0;
        timer.start();
        chart_array[i].setToolTipText("");
    }
 }

    


 

  






}