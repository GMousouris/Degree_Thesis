import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.File;

public class myButton3 extends JButton implements MouseListener {

    private boolean expanded = false;
    private BufferedImage exp1,exp11,exp2,exp22,img;

    public myButton3(){
        super();


        try{
            exp1 = ImageIO.read(new File("Images/expand1.png"));
            exp11 = ImageIO.read(new File("Images/expand11.png"));
            exp2 = ImageIO.read(new File("Images/expand2.png"));
            exp22 = ImageIO.read(new File("Images/expand22.png"));

        }catch (Exception e){}

        img = exp1;

        setSize(new Dimension(23,22));
        setBackground(null);
        setFocusPainted(false);
        setBorderPainted(false);
        setContentAreaFilled(false);
        setOpaque(false);
        addMouseListener(this);

    }

    public boolean isExpanded(){
        return  expanded;
    }

    protected void paintComponent(Graphics g){
        super.paintComponent(g);
        Graphics2D g2d = (Graphics2D)g;
        g2d.drawImage(img,0,0,null);
    }

    @Override
    public void mouseClicked(MouseEvent e) {
        if(expanded){
            img = exp1;
        }
        else{
            img = exp2;
        }
        expanded = !expanded;
    }

    @Override
    public void mousePressed(MouseEvent e) {

    }

    @Override
    public void mouseReleased(MouseEvent e) {

    }

    @Override
    public void mouseEntered(MouseEvent e) {
        if(expanded){
            img = exp22;
        }
        else{
            img = exp11;
        }
    }

    @Override
    public void mouseExited(MouseEvent e) {
        if(!isEnabled()){
            return;
        }
        if(expanded){
            img = exp2;
        }
        else{
            img = exp1;
        }
    }
}
