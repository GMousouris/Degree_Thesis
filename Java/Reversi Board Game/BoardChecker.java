import javax.imageio.ImageIO;
import javax.swing.*;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.File;

public class BoardChecker extends JButton {

    private BufferedImage BLACK = null;
    private BufferedImage WHITE = null;
    private BufferedImage img = null;


    static final Color  default_color = new Color(39,163,26);
    private int i = -1;
    private int j = -1;

    public BoardChecker()
    {
        super();

        try
        {
            BLACK = ImageIO.read(new File("images/BLACK.PNG"));
            WHITE = ImageIO.read(new File("images/WHITE.PNG"));
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }

        setForeground(Color.red);
        setBackground(default_color);
        setBorder(BorderFactory.createLineBorder(Color.darkGray));
        setFocusPainted(false);
        setOpaque(false);

        setSize(new Dimension(50 , 50));


    }


    protected void paintComponent(Graphics g)
    {
        Graphics2D g2d = (Graphics2D)g;
        if(img == BLACK || img == WHITE)
        {
            g2d.drawImage(img , 0 , 0, null);
        }
        else{
            g2d.setColor(getBackground());
            g2d.fillRect(0,0 , getWidth(), getHeight() );
        }

        if(i >=0 && j >=0)
        {
            g2d.setColor(new Color(0,255,0,170));
            g2d.drawString("["+i+","+j+"]" , 12,30);

        }
    }


    public void setIMG(int IMG)
    {
        if(IMG == Board.BLACK)
        {
            img = BLACK;
        }
        else if(IMG == Board.WHITE)
        {
            img = WHITE;
        }
        else{
            img = null;
        }
    }

    public int getColor()
    {
        if(img == null)
        {
            return Board.EMPTY;
        }
        else if(img == BLACK)
        {
            return Board.BLACK;
        }
        else
        {
            return Board.WHITE;
        }
    }

    public void drawText(int i , int j)
    {
        this.i = i;
        this.j = j;
    }


    public static Color getDefaultColor()
    {
        return default_color;
    }


}
