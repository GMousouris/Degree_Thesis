import java.io.*;
public class Index {

    private int i;
    private int j;
    private double v;

    public Index(int i,int j){
        this.i = i;
        this.j = j;
    }

    public Index(double v,int j){
        this.v = v;
        this.j = j;
    }

    public int i(){
        return this.i;
    }

    public int j(){
        return this.j;
    }

    public double V(){
        return this.v;
    }
}
