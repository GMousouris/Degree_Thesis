import javax.swing.JFrame;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JTextField;
import java.awt.Color;
import java.awt.Insets;
import javax.swing.JComboBox;
import java.util.Vector;
import javax.swing.JColorChooser;
import java.awt.event.ActionListener;
import java.awt.event.*;

/*
Base class : <light_choose_Frame>
    /*
    Holding data {
         - GUI elements inside the JFrame window
    }
*/
public class light_choose_Frame extends JFrame implements ActionListener,KeyListener{

    private JButton add,show_color;
    private JTextField light_name,light_type;
    private JLabel light_name_label ,light_type_label,color_choose;
    private JComboBox types;

    private Vector<String> types_name;
    private Color chosen_color = Color.white;

    private int state = -1;
    private Light light;


    public light_choose_Frame(){

        super("Choose Light");

		/*Setting the GUI*/
        setGui();

		/*Setting the window*/
        this.setSize(292,150);
        this.setDefaultCloseOperation(DISPOSE_ON_CLOSE);
        this.setResizable(false);
        this.setLocationRelativeTo(null);
        this.setVisible(true);

    }

    public light_choose_Frame(Light light){
        super("Edit Light");

        this.light = light;
        setGui();

        this.setSize(292,150);
        this.setDefaultCloseOperation(DISPOSE_ON_CLOSE);
        this.setResizable(false);
        this.setLocationRelativeTo(null);
        this.setVisible(true);
    }

    public void setGui(){

        this.setLayout(null);
        this.getContentPane().setBackground(Color.darkGray);

        light_name_label = new JLabel("Light Label : ");
        light_name_label.setForeground(Color.lightGray);
        light_name_label.setBounds(this.getInsets().left+10,this.getInsets().top+20,90,20);
        this.add(light_name_label);

        if(this.light==null){
            light_name = new JTextField("");
        }
        else{
            light_name = new JTextField(this.light.get_Label());
        }
        light_name.setBounds(this.getInsets().left+10+100+10,this.getInsets().top+20,150,24);
        //light_name.setBorder(null);
        light_name.addKeyListener(this);
        this.add(light_name);


        light_type_label = new JLabel("Light type : ");
        light_type_label.setForeground(Color.lightGray);
        light_type_label.setBounds(this.getInsets().left+10,this.getInsets().top+20+25+3,100,20);
        this.add(light_type_label);

        types_name = new Vector<String>(); types_name.add("Regular light"); types_name.add("RGB light");
        types = new JComboBox(types_name);
        types.setBounds(this.getInsets().left+10+100+10,this.getInsets().top+20+25+3,150,23);
        types.setSelectedIndex(0);
        types.addActionListener(this);
        this.add(types);
        if(this.light!=null){
            if(this.light.getClass().getName().equals("normal_Light")){
                types.setSelectedIndex(0);
            }
            else{
                types.setSelectedIndex(1);
            }
            types.setEnabled(false);
        }

        color_choose = new JLabel("Color");
        color_choose.setForeground(Color.lightGray);
        color_choose.setBounds(this.getInsets().left+10,this.getInsets().top+20+25+25+3,70,20);

        this.add(color_choose);

        show_color = new JButton();
        show_color.setBounds(this.getInsets().left+10+110,this.getInsets().top+20+25+25+3,22,22);

        show_color.setBackground(Color.white);

        if(light!=null){
            show_color.setBackground(light.get_Color());
        }

        show_color.addActionListener(this);

        this.add(show_color);
       /* if(this.light!=null){

        	if(String.valueOf(types.getSelectedItem()).equals("normal_Light")){
			show_color.setEnabled(false);
		    }
		    else{
			    show_color.setEnabled(true);
		     }

        }
		*/

        add = new JButton("Complete");
        add.setBounds(this.getInsets().left+10+110+60,this.getInsets().top+20+25+25+20,90,25);
        add.addActionListener(this);
        this.add(add);

    }




    /*ACTION LISTENER*/
    public void actionPerformed(ActionEvent e){

        if(e.getSource().equals(types) && this.light==null){
            if(String.valueOf(types.getSelectedItem()).equals("Regular light")){
                chosen_color = Color.white;
                show_color.setBackground(chosen_color);
            }
        }

        if(e.getSource().equals(show_color) && String.valueOf(types.getSelectedItem()).equals("RGB light")) {
            chosen_color = JColorChooser.showDialog(null, "JColorChooser Sample",Color.white);
            show_color.setBackground(chosen_color);
        }

        if(e.getSource().equals(add)){
            if(light!=null){
                state = 1;
            }
            else{
                state = 0;
            }
            this.dispatchEvent(new WindowEvent(this, WindowEvent.WINDOW_CLOSING));
        }
    }
	/* */

    /*KEY LISTENER*/
    public void keyTyped(KeyEvent e) {

    }


    public void keyPressed(KeyEvent e) {
        if(e.getKeyCode()==KeyEvent.VK_ENTER){

            if(e.getSource().equals(light_name)){
                if(light!=null){
                    state = 1;
                }
                else{
                    state = 0;
                }

                this.dispatchEvent(new WindowEvent(this, WindowEvent.WINDOW_CLOSING));
            }
        }
    }


    public void keyReleased(KeyEvent e) {

    }
    /* */



    String getLight_label(){
        return this.light_name.getText();
    }

    String getLight_type(){
        return String.valueOf(this.types.getSelectedItem());
    }

    Color getLight_color(){
        return this.chosen_color;
    }

    int get_State(){
        return this.state;
    }


}



