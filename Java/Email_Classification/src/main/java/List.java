import javax.swing.*;
import java.awt.*;

public class List extends JPanel
{



    class ListNode
    {
        ListNode next;
        int      value;

        public ListNode(int value)
        {
            this.value = value;
        }


    }

    public int getListSum(ListNode rootNode)
    {
        int sum = 0;
        ListNode node = rootNode;

        return sum;
    }


    protected void paintComponent(Graphics g)
    {
        super.paintComponent(g);
        Graphics2D g2 = (Graphics2D) g;
        g2.setStroke(new BasicStroke(2));
        g.drawLine();
    }



    public static void main(String[] args)
    {

    }
}
