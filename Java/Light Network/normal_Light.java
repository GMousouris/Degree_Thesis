import java.awt.Color;


/*
Derived class : <normal_light> / Base class : Light
    /*
    Holding data {
         -
    }
*/
public class normal_Light extends Light{


    /*Default Constructor*/
    public normal_Light(){
        super("Regular");
    }

    /*Constructor #1*/
    public normal_Light(String label){
        super(label,"Regular");
    }
}