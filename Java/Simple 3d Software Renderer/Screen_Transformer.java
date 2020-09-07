import javax.swing.*;

public class Screen_Transformer {

    private double xF,yF;
    private JFrame frame;

    public Screen_Transformer(JFrame frame){
        this.frame = frame;
        xF = frame.getWidth()/2.0;
        yF = frame.getHeight()/2.0;
    }

    /*trasnforming vec3{x,y,z} -> to XY screen co-ordinates*/
    public Vec3 transform(Vec3 vec){

        double invZ = 1.0/vec.z;
        vec.x = (vec.x*invZ + 1.0)*xF;
        vec.y = (-vec.y*invZ + 1.0)*yF;
        return vec;
    }

    public void update(){
        xF = frame.getWidth()/2.0;
        yF = frame.getHeight()/2.0;
    }
}
