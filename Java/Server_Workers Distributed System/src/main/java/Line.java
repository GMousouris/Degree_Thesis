import java.awt.Color;



public class Line{



    boolean done = true;
    int[] cords;
    Color color = new Color(0,255,65);

    int r = color.getRed();
    int g = color.getGreen();
    int b = color.getBlue();

    boolean state = false;
    float d_phase = 100.0f;
    float d = 1;
    boolean stroke = false;


    /*Default Constructor*/
    public Line(){
        cords = new int[4];
    }


    /*Constructor*/
    public Line(int x1,int y1,int x2,int y2){
        cords = new int[]{x1,y1,x2,y2};
    }


    /*Method that sets the coordinates of the Line*/
    public void set_Cords(int x1,int y1,int x2,int y2){
        cords[0] = x1; cords[1] = y1; cords[2] = x2; cords[3] = y2;
    }

    public Color getColor(){
        return  this.color;
    }

    /*Method that switches the state of the Line [on/off]*/
    public void swap(){
        state = !state;
    }

    /*Method that resets Line's color*/
    public void reset(){
        this.color = Color.black;
    }

    /*Method that updates Line's color according to it's state*/
    public void update(){

        if(this.state){
            this.color = Color.green;
        }
        else{
            this.color = Color.black;
        }
    }

    public boolean get_State(){
        return this.state;
    }


    public void setIN(MyNode server){
        server.setProcessing();
        d = -1;
        stroke = true;
    }

    public void setOUT(){
        d = 1;
        stroke = true;
    }

    public void setDEF(){
        d = 1;
        stroke = false;
    }

    public void heartBeat(){
        new Thread(new heartBeat()).start();
    }


    public class heartBeat implements Runnable{

        public void run(){


            int rr,gg,bb;
            rr = r; gg = g; bb = b;

            int counter = 0;
            int z = 1;
            int d = 5;

            while(true){

                try{
                    Thread.sleep(5);
                }catch(Exception e){}

                if(z==1){
                    if(rr+d<=255)rr+=d;
                    if(gg+d<=255)gg+=d;
                    if(bb+d<=255)bb+=d;
                }
                else{
                    if(rr-d>=r)rr-=d;
                    if(gg-d>=g)gg-=d;
                    if(bb-d>=b)bb-=d;
                }

                if(rr==255 && gg == 255 && bb == 255){
                    try{
                        Thread.sleep(45);
                    }catch (Exception e){}
                    z = -1; counter++;
                }
                if(rr == r && gg == g && bb == b){
                    z = 1; counter++;
                }
                color = new Color(rr,gg,bb);

                if(counter == 4)break;

            }
        }
    }
}