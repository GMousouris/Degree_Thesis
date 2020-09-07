import javax.imageio.IIOImage;
import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.File;

public class myButton2 extends JButton implements MouseListener {

    private ImageIcon icon1,icon2,icon;
    private BufferedImage img1,img2,img;
    private boolean entered = false;

    public myButton2(){

        super();

        try{
            img1 = ImageIO.read(new File("Images/trash0.png"));
            img2 = ImageIO.read(new File("Images/trash1.png"));
        }catch(Exception e){}


        setSize(new Dimension(39,55));
        img = img1;
        setBackground(null);
        setFocusPainted(false);
        setBorderPainted(false);
        setContentAreaFilled(false);
        setOpaque(false);

        addMouseListener(this);
    }


    protected void paintComponent(Graphics g){
        super.paintComponent(g);
        Graphics2D g2d = (Graphics2D)g;

        float alpha = 1.0f;
        if(entered){
            alpha = 0.7f;
        }
        else{
            alpha = 1.0f;
        }
        AlphaComposite ac = AlphaComposite.getInstance(AlphaComposite.SRC_OVER,alpha);
        g2d.setComposite(ac);
        g2d.drawImage(img,0,0,null);
    }


    @Override
    public void mouseClicked(MouseEvent e) {

    }

    @Override
    public void mousePressed(MouseEvent e) {

    }

    @Override
    public void mouseReleased(MouseEvent e) {

    }

    @Override
    public void mouseEntered(MouseEvent e) {
        img = img2;
        entered = true;
    }

    public boolean isEntered(){
        return entered;
    }

    @Override
    public void mouseExited(MouseEvent e) {
        img = img1;
        entered = false;
    }
}
