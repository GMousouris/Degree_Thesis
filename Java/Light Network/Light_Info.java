import javax.swing.JPanel;
import javax.swing.JLabel;
import javax.swing.JTextField;
import java.awt.*;
import javax.swing.BorderFactory;
import javax.swing.JTextArea;


/*
Base class : <Light_Info> /*Holds the info_panel for each Light instance*/
    /*
    Holding data {
         -JLabel name,type : Labels
         -JTextField light_name,state_name,type_name : info for each label
    }
*/
public class Light_Info extends JPanel{

    JLabel name,type;
    JTextField light_name,state_name,type_name;




    public Light_Info(String label,String light_type){

		/*initializing data*/
        super();
        this.setBackground(Color.darkGray);


        this.name = new JLabel("Label : ");
        this.name.setBackground(Color.darkGray);
        this.name.setForeground(Color.lightGray);


        this.light_name = new JTextField(label);
        this.light_name.setBorder(BorderFactory.createBevelBorder(1));
        this.light_name.setBackground(Color.darkGray);
        this.light_name.setForeground(Color.white);
        this.light_name.setEditable(false);

        this.type = new JLabel("Type : ");
        this.type.setBackground(Color.lightGray);
        this.type.setForeground(Color.lightGray);

        this.type_name = new JTextField(light_type);
        this.type_name.setBorder(BorderFactory.createBevelBorder(1));
        this.type_name.setBackground(Color.darkGray);
        this.type_name.setForeground(Color.white);
        this.type_name.setEditable(false);

        this.state_name = new JTextField("OFF");
        this.state_name.setEditable(false);

        this.state_name.setBorder(BorderFactory.createBevelBorder(0));
        this.state_name.setBackground(Color.darkGray);
        this.state_name.setForeground(Color.red);
        this.state_name.setFont(new Font(this.state_name.getFont().getName(),Font.BOLD,14));


        /*setting the GUI*/
        setGui();
    }

    public void setGui(){

        Insets insets = this.getInsets();
        this.setLayout(null);

        this.name.setBounds(insets.left+2,insets.top+2,40,20);
        this.add(name);

        this.light_name.setBounds(insets.left+2+45,insets.top+2,100,20);
        this.add(light_name);

        this.type.setBounds(insets.left+2,insets.top+2+20+2,40,20);
        this.add(type);

        this.type_name.setBounds(insets.left+2+50-5,insets.top+2+20+2,50,20);
        this.add(type_name);

        this.state_name.setBounds(insets.left+2+50-5+20+45,insets.top+2+20+2,36,22);
        this.add(state_name);






    }






    /*Method that sets the info_panel state label to : [on/off]*/
    public void set_State(boolean state){
        if(state){
            this.state_name.setText(" ON");
            this.state_name.setForeground(Color.green);
        }
        else{
            this.state_name.setText("OFF");
            this.state_name.setForeground(Color.red);
        }
    }

    public void set_Label(String label){
        this.light_name.setText(label);
    }

    public void setHover(){

        this.setBackground(new Color(64,64,64,60));

        this.name.setForeground(Color.gray);

        this.light_name.setBorder(BorderFactory.createBevelBorder(1));
        this.light_name.setBackground(new Color(64,64,64,60));
        this.light_name.setForeground(Color.gray);

        this.type.setBackground(Color.darkGray);
        this.type.setForeground(Color.gray);

        this.type_name.setBorder(BorderFactory.createBevelBorder(1));
        this.type_name.setBackground(new Color(64,64,64,60));
        this.type_name.setForeground(Color.gray);

        this.state_name.setBorder(BorderFactory.createBevelBorder(0));
        this.state_name.setBackground(new Color(64,64,64,60));
        this.setBorder(BorderFactory.createLineBorder(new Color(10,140,250,90),2));


    }
    public void reset(){
        this.setBorder(null);
        this.setBackground(Color.darkGray);

        this.name.setBackground(Color.darkGray);
        this.name.setForeground(Color.lightGray);

        this.light_name.setBorder(BorderFactory.createBevelBorder(1));
        this.light_name.setBackground(Color.darkGray);
        this.light_name.setForeground(Color.white);

        this.type.setBackground(Color.lightGray);
        this.type.setForeground(Color.lightGray);

        this.type_name.setBorder(BorderFactory.createBevelBorder(1));
        this.type_name.setBackground(Color.darkGray);
        this.type_name.setForeground(Color.white);

        this.state_name.setBorder(BorderFactory.createBevelBorder(0));
        this.state_name.setBackground(Color.darkGray);

    }

}