import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;

public class myButton extends JButton implements MouseListener{

    private String title = null;
    private String desc = null;
    private String deadline = null;
    private JPanel panel = null;
    private JPanel infoPanel = null;

    private JLabel l1,l12,l2;
    private JTextField entryTitle,entryDeadline;
    private JTextArea entryDescription;
    private JScrollPane scroll;
    private JButton entryOK;
    private boolean pressed = false;
    private int x,y;
    private Rectangle bounds;
    private boolean adjusting = false;

    private boolean active = false;

    public myButton(myButton entry){

        super(entry.getTitle());
        setSize(new Dimension(145,50));
        setBackground(new Color(42,42,42));
        setBounds(entry.getBounds());
        //setBackground(new Color(51, 51, 45));
        //setBackground(null);
        setForeground(Color.white);
        setBorder(BorderFactory.createLineBorder(Color.gray));
        setFocusPainted(false);

        addMouseListener(this);

    }

    public myButton(String title,String desc,String deadline){
        super(title);

        this.title = title;
        this.desc = desc;
        this.deadline = deadline;


        setSize(new Dimension(145,50));
        setBackground(new Color(42,42,42));
        //setBackground(new Color(51, 51, 45));
        //setBackground(null);
        setForeground(Color.white);
        setBorder(BorderFactory.createLineBorder(Color.gray));
        setFocusPainted(false);
        addMouseListener(this);

        //createPanel();
    }

    public void setAdjusting(boolean a){
        adjusting = a;
    }
    public boolean isAdjusting(){
        return adjusting;
    }

    public void setActive(boolean state){
        active = state;
        if(active){
            //setBorder(BorderFactory.createLineBorder(Color.lightGray));
            setBorder(BorderFactory.createLineBorder(new Color(191, 252, 202)));
        }
        else{
            //setBackground(new Color(42,42,42));
            setBorder(BorderFactory.createLineBorder(Color.gray));
        }
    }

    public boolean isActive(){
        return this.active;
    }

    public boolean isPressed(){
        return pressed;
    }

    private void createPanel(){


        //infoPanel size = [400,379]
        infoPanel = new JPanel(null);
        infoPanel.setBackground(new Color(84,84,84));
        Insets ins = infoPanel.getInsets();

        l1 = new JLabel("Title : ");
        l1.setBounds(ins.left+5,ins.top+10,40,25);
        l1.setForeground(Color.lightGray);

        entryTitle = new JTextField();
        entryTitle.setBounds(ins.left+5+60+2-25,ins.top+10,200+137-2,25);
        entryTitle.setBackground(Color.darkGray);
        entryTitle.setForeground(Color.lightGray);
        entryTitle.setBorder(BorderFactory.createBevelBorder(1));
        entryTitle.setCaretColor(Color.lightGray);
        infoPanel.add(l1);
        infoPanel.add(entryTitle);

        l12 = new JLabel("DeadLine : ");
        l12.setBounds(ins.left+5,ins.top+10+25+10,70,25);
        l12.setForeground(Color.lightGray);

        entryDeadline = new JTextField();
        entryDeadline.setBounds(ins.left+5+60+8,ins.top+10+25+10,100,25);
        entryDeadline.setBackground(Color.darkGray);
        entryDeadline.setForeground(Color.lightGray);
        //entryDeadline.setBorder(BorderFactory.createLineBorder(Color.lightGray));
        entryDeadline.setBorder(BorderFactory.createBevelBorder(0));
        entryDeadline.setCaretColor(Color.lightGray);
        infoPanel.add(l12);
        infoPanel.add(entryDeadline);

        l2 = new JLabel("Description : ");
        l2.setBounds(ins.left+5,ins.top+10+25+10+30,100,25);
        l2.setForeground(Color.lightGray);

        entryDescription = new JTextArea();
        entryDescription.setBackground(new Color(244, 212, 137));
        entryDescription.setForeground(Color.black);
        scroll = new JScrollPane(entryDescription);
        scroll.setBorder(BorderFactory.createLineBorder(Color.lightGray));

        scroll.setBounds(ins.left+5,ins.top+10+25+10+25+2+30,200+174,200);
        infoPanel.add(l2);
        infoPanel.add(scroll);

        entryOK = new JButton("OK");
        entryOK.setBounds(ins.left+318,ins.top+310,60,25);
        infoPanel.add(entryOK);
    }

    public String getTitle(){
        return this.title;
    }

    public String getDesc(){
        return this.desc;
    }

    public String getDeadline(){
        return this.deadline;
    }

    public int getXX(){
        return x;
    }

    public int getYY(){
        return y;
    }

    public void setXY(int x, int y){
        this.x = x; this.y = y;
    }

    public void setBound(Rectangle bounds){
        this.bounds = bounds;
    }

    public Rectangle getBound(){
        return this.bounds;
    }





    public void save(){

    }

    @Override
    public void mouseClicked(MouseEvent e) {
        if(active){
            setBackground(new Color(80,80,80));
            setBorder(BorderFactory.createLineBorder(new Color(191, 252, 202)));
        }

    }

    @Override
    public void mousePressed(MouseEvent e) {
        pressed = true;
    }

    @Override
    public void mouseReleased(MouseEvent e) {
        pressed = false;
    }

    @Override
    public void mouseEntered(MouseEvent e) {
        if(active)return;
        setBackground(new Color(90, 90, 89));
        //setBackground(new Color(84,84,84));

        setBorder(BorderFactory.createLineBorder(Color.lightGray));
    }

    @Override
    public void mouseExited(MouseEvent e) {
        if(active)return;
        setBackground(new Color(42,42,42));

        if(active)return;
        setBorder(BorderFactory.createLineBorder(Color.gray));
    }

    public void updateData(String title,String deadline,String desc){
        this.title = title;
        setText(this.title);
        this.deadline = deadline;
        this.desc = desc;
    }
}
