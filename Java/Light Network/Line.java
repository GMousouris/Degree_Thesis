import java.awt.Color;


/*
Base class : <Line>
    Holding data {
         -int[] cords : holds the coordinates[x1,y1,x2,y2] of the Line
         -Color color : holds the Color data of the Line
         -boolean state : holds the state [on/off] of the Line
    }
*/
public class Line{



    int[] cords;
    Color color = Color.black;
    private boolean state = false;
    private boolean connected = true;
    private boolean powered = false;
    private int p = 0;
    double rotation = 0;
    int ell_xy = 200;
    float dash_p = 0.0f;


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

    /*Method that switches the state of the Line [on/off]*/
    public void swap(){
        state = !state;
    }

    /*Method that resets Line's color*/
    public void reset(){

         this.color = Color.black;
         //this.update();
         //connected = true;
        // powered = false;

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

    public void set_p(int a){
        this.p = a;
    }
    public int get_p(){
        return this.p;
    }

    public boolean isConnected(){
        return this.connected;
    }

    public void switch_con(boolean state){
        this.connected = state;
    }

    public void rotate(double a){
        rotation+=a;
    }

    public boolean isPowered(){
        return this.powered;
    }

    public void set_Power(boolean s){
        this.powered = s;
    }
}