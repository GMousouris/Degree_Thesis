import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.io.*;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;

public class Lesson extends JTextField implements MouseListener{

    private String[] files = {"tasks.txt","projects.txt","exams.txt"};
    private String[] data = {"","",""};

    private ArrayList<myButton> tasks;
    private ArrayList<myButton> projects;
    private ArrayList<myButton> exams;

    private String label;
    private boolean focused = false;

    private JPanel panel = null;





    public Lesson(String label,boolean create) {
        super(label);

        this.label = label;
        this.panel = panel;

        tasks = new ArrayList<myButton>();
        projects = new ArrayList<myButton>();
        exams = new ArrayList<myButton>();


        setSize(new Dimension(147,32));
        setBorder(BorderFactory.createBevelBorder(0));
        setBackground(new Color(64,64,64));
        setForeground(Color.lightGray);
        //setBorder(BorderFactory.createLineBorder(new Color(32,32,32)));
        setHorizontalAlignment(JTextField.CENTER);
        setEditable(false);
        setFocusable(true);

        addMouseListener(this);


        if(create){
            try{

                Path currentRelativePath = Paths.get("");
                String s = currentRelativePath.toAbsolutePath().toString();
                s += "/Lessons/"+label;
                new File(s).mkdirs();
                s += "/";

                BufferedWriter writer = new BufferedWriter(new FileWriter(s+"/tasks.txt"));
                writer = new BufferedWriter(new FileWriter(s+"/projects.txt"));
                writer = new BufferedWriter(new FileWriter(s+"/exams.txt"));

                writer.close();

            }
            catch(Exception e){}
        }
        //else{
          //  load();
        //}


    }



    public void rename(String newName){
        Path currentRelativePath = Paths.get("");
        String s = currentRelativePath.toAbsolutePath().toString();
        s += "/Lessons/"+newName;
        new File(s).mkdirs();
        s += "/";

        String oldLabel = label;

        label = newName;
        save();

    }

    void deleteDirectory(File file) throws IOException {
        if (file.isDirectory()) {
            File[] entries = file.listFiles();
            if (entries != null) {
                for (File entry : entries) {
                    deleteDirectory(entry);
                }
            }
        }
        if (!file.delete()) {
            throw new IOException("Failed to delete " + file);
        }
    }

    public void save(){


        ArrayList<ArrayList<myButton>> list = new ArrayList<ArrayList<myButton>>();
        list.add(tasks); list.add(projects); list.add(exams);

        for(int i = 0; i<3; i++){

            Path currentRelativePath = Paths.get("");
            String s = currentRelativePath.toAbsolutePath().toString();
            s += "/Lessons/"+label+"/";

            BufferedWriter bw = null;
            File file = new File(s+files[i]);


            try {
                bw = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(file,false),"UTF-8"));

                for(myButton entry : list.get(i)){

                    String title = entry.getTitle();
                    String desc = entry.getDesc();
                    String deadline = entry.getDeadline();

                    bw.write("{");
                    bw.newLine();
                    bw.write("<");bw.newLine();
                    bw.write(entry.getTitle());
                    bw.newLine();
                    bw.write(">");
                    bw.newLine();
                    bw.write("<");
                    bw.newLine();
                    bw.write(entry.getDeadline());
                    bw.newLine();
                    bw.write(">");
                    bw.newLine();
                    bw.write("<");
                    bw.newLine();
                    bw.write(entry.getDesc());
                    bw.newLine();
                    bw.write(">");
                    bw.newLine();
                    bw.write("}");
                    bw.newLine();

                    //bw.close();
                }


            }catch(Exception e){}

            finally{
                try{
                    bw.close();
                }catch(Exception e){}
            }


        }
    }

    public void load(){

        Path currentRelativePath = Paths.get("");
        String s = currentRelativePath.toAbsolutePath().toString();
        s += "/Lessons/"+label+"/";

        BufferedReader br = null;
        File file = null;

        for(int i = 0; i<3; i++){

            file = new File(s+files[i]);

            try{

                br = new BufferedReader(
                        new InputStreamReader(
                                new FileInputStream(file), "UTF8"));

                String line = "";

                boolean reading = false;
                boolean reading_info = false;

                int symb1_count = 0;
                int symb2_count = 0;

                int counter = 0;

                String title = "";
                String deadline = "";
                String desc = "";

                while((line = br.readLine())!= null){


                    line.replace("\uFEFF", "");

                    if(line.equals("{") || line.equals("}")){
                        symb1_count++;
                        if(symb1_count%2 == 0){
                            counter = 0;
                            symb1_count = 0;
                            reading = false;
                            reading_info = false;
                        }
                        else{
                            reading = true;
                        }
                    }
                    else if(line.equals("<") || line.equals(">")){
                        symb2_count++;
                        if(symb2_count == 1){
                            counter++;
                            reading_info = true;
                        }
                        else if(symb2_count == 2){
                            reading_info = false;
                            symb2_count = 0;
                        }

                    }
                    else {
                        switch(counter){
                            case 1 : title = line; reading_info = false;  break;
                            case 2 : deadline = line; reading_info = false;  break;
                            case 3 : desc += line+"\n"; break;
                        }
                    }


                    if(counter == 3 && !reading_info){
                        counter = 0;

                        switch(i){
                            case 0 : tasks.add(new myButton(title,desc,deadline)); title = ""; desc =""; deadline=""; break;
                            case 1 : projects.add(new myButton(title,desc,deadline));title = ""; desc =""; deadline=""; break;
                            case 2 : exams.add(new myButton(title,desc,deadline));title = ""; desc =""; deadline=""; break;
                        }
                    }

                }
                br.close();

            }catch(Exception e){}

        }

    }

    public void setFocused(boolean state){

        focused = state;

        if(focused){
            setBackground(new Color(112, 111, 94));

            setForeground(Color.white);
            //setBorder(BorderFactory.createLineBorder(Color.lightGray));
        }
        else{
            setBackground(new Color(64,64,64));
            setForeground(Color.lightGray);
            setBorder(BorderFactory.createBevelBorder(0));
        }

    }

    public boolean isFocused(){
        return this.focused;
    }

    public void addEntry(String title,String desc,String deadline,int ch){


        if(ch == 1){
            tasks.add(new myButton(title,desc,deadline));
        }
        else if(ch == 2){
            projects.add(new myButton(title,desc,deadline));
        }
        else{
            exams.add(new myButton(title,desc,deadline));
        }
    }

    public ArrayList<myButton> getArray(int ch) {
        if(ch == 1){
            return new ArrayList<myButton>(tasks);
        }
        else if(ch == 2){
            return new ArrayList<myButton>(projects);
        }
        else{
            return new ArrayList<myButton>(exams);
        }
    }

    public ArrayList<ArrayList<myButton>> getData(){

        ArrayList<ArrayList<myButton>> list = new ArrayList<ArrayList<myButton>>();
        list.add(new ArrayList<myButton>(tasks));
        list.add(new ArrayList<myButton>(projects));
        list.add(new ArrayList<myButton>(exams));

        return list;
    }

    public void updateEntry(ArrayList<myButton> list,int pointer){
        if(pointer == 0){
            tasks = new ArrayList<myButton>(list);
        }
        else if(pointer == 1){
            projects = new ArrayList<myButton>(list);
        }
        else{
            exams = new ArrayList<myButton>(list);
        }
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
        if(focused){
            return;
        }
        setBackground(new Color(84,84,84));
        setBorder(BorderFactory.createBevelBorder(1));
        setForeground(Color.white);
        //setBorder(BorderFactory.createLineBorder(Color.lightGray));
    }

    @Override
    public void mouseExited(MouseEvent e) {
        if(!focused){
            setBackground(new Color(64,64,64));
            setForeground(Color.lightGray);

            //setBorder(null);
        }
        setBorder(BorderFactory.createBevelBorder(0));




    }
}
