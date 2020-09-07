
import java.awt.*;
import javax.swing.*;



public class infoArea extends JTextArea {


    private int x;
    private int y;



    public infoArea(MyNode node){

        /*initializing data*/
        super("Progress : \nCPU : \nRAM : \nSTATE : ");
        this.setBorder(null);
        this.setBackground(new Color(10,0,0,0));
        this.setForeground(new Color(0,255,100));
        this.setEditable(false);
        this.setVisible(false);

        this.x = node.getX();
        this.y = node.getY();

    }

    public void setGui(){

    }






}