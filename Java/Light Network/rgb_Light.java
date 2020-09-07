import java.awt.Color;

/*
Derived class : <rgb_light> / Base class : Light
    /*
    Holding data {
         -
    }
*/
public class rgb_Light extends Light{

    /*Default Constructor*/
    public rgb_Light(){
        super("RGB");

    }


    /*Constructor #1*/
    public rgb_Light(String label,Color color){
        super(label,color,"RGB");

    }
}