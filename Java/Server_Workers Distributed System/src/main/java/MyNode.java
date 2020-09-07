import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;

/*MyNode class*/
class MyNode extends JButton implements MouseListener {

    private BufferedImage drawimg;
    private BufferedImage img;
    private BufferedImage hover;
    private BufferedImage processing;
    private boolean processingState = false;
    private infoArea info;

    public MyNode(){

        super();
        this.addMouseListener(this);
        setOpaque(false);
        setContentAreaFilled(false);
        setBorderPainted(false);
        setFocusPainted(false);

        try{
            img = ImageIO.read(new File("worker1.png"));
            hover = ImageIO.read(new File("worker2.png"));
            processing = ImageIO.read(new File("worker3.png"));
            drawimg = img;
        } catch (IOException e) {
            e.printStackTrace();
        }

        info = new infoArea(this);


    }
    public MyNode(String title){
        super();

        this.addMouseListener(this);
        setOpaque(false);
        setContentAreaFilled(false);
        setBorderPainted(false);
        setFocusPainted(false);

        try{
            img = ImageIO.read(new File("server1.png"));
            hover = ImageIO.read(new File("server1.png"));
            processing = ImageIO.read(new File("server3.png"));
            drawimg = img;
        } catch (IOException e) {
            e.printStackTrace();
        }

        info = new infoArea(this);

    }


    public void changeImg(){
        if(drawimg.equals(img)){
            drawimg = processing;
        }
        else{
            drawimg = img;
        }
    }

    public void setImage(String path){
        try{
            img = ImageIO.read(new File(path));
        } catch (IOException e) {
            e.printStackTrace();
        }
        repaint();
    }

    public void paintComponent(Graphics g){
        super.paintComponent(g);
        g.drawImage(drawimg,0,0,null);
    }

    public Dimension getPreferredSize(){
        return new Dimension(img.getWidth(),img.getHeight());
    }
    public void mouseClicked(MouseEvent e)
    {

    }

    public void mouseEntered(MouseEvent e)
    {

        info.setVisible(true);
        drawimg = hover;
    }

    public void mouseExited(MouseEvent e)

    {
        if(processingState){
            drawimg = processing;
        }
        else{
            drawimg = img;
        }

        info.setVisible(false);
    }

    public void mousePressed(MouseEvent e) {
    }
    public void mouseReleased(MouseEvent e) {}




    public void setProcessing(boolean state){
        if(state){
            drawimg = processing;
        }
        else{
            drawimg = img;
        }
        this.processingState = state;
    }
    public void setProcessing(){
        new Thread(new ProcessAnimation(this)).start();
    }
    public boolean isProcessing(){
        return this.processingState;
    }

    public class ProcessAnimation implements Runnable{

        private MyNode node;
        private boolean state;


        public ProcessAnimation(MyNode node){
            this.node = node;
            this.state = true;
        }

        public void run(){

            if(state){

                    node.changeImg();
                    try{
                        Thread.sleep(400);
                    }catch (Exception e){}

                    node.changeImg();
                    try{
                        Thread.sleep(100);
                    }catch (Exception e){}

                node.setProcessing(false);
                return;
            }

        }

    }

    public infoArea getInfoArea(){
        return this.info;
    }


}
/* */