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
public class Light_Info2 extends JTextArea{


    //JTextArea data_area;



    public Light_Info2(){

		/*initializing data*/
        super("__INFO__\nPower : \nHours : \n");
        this.setBorder(null);
        //this.setVisible(false);
        //this.setBackground(Color.darkGray);
        this.setBackground(new Color(10,0,0,95));
        this.setForeground(new Color(0,255,100));
        //this.setBorder(null);
        //data_area = new JTextArea("__INFO__\nPower : \nHours : \n");
        //data_area.setBorder(null);
        this.setEditable(false);
        this.setVisible(false);






        /*setting the GUI*/
        //setGui();
    }

    public void setGui(){



    }







}