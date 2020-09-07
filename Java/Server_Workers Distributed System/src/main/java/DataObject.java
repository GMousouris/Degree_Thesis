import org.apache.commons.math3.linear.MatrixUtils;
import org.apache.commons.math3.linear.RealMatrix;
import java.io.Serializable;

public class DataObject implements Serializable {


    RealMatrix Matrix;

    private double alpha = -1;
    private double lambda = -1;
    private double workload = 0.0;

    private int f =-1;

    private int begin = -1;
    private int end = -1;
    private int current = -1;
    private String type;


    public DataObject(String type,RealMatrix matrix,double a,double l,int f){

        this.type = type;

        Matrix = matrix;
        this.alpha = a;
        this.lambda = l;
        this.f = f;


    }

    public DataObject(String type, RealMatrix matrix,int a ,int b){

        this.type = type;
        Matrix = matrix;
        begin = a;
        end = b;

    }

    public DataObject(String type, RealMatrix matrix,int a ,int b,double load){

        this.type = type;
        this.Matrix = matrix;
        this.begin = a;
        this.end = b;
        this.workload = load;
    }

    public DataObject(String type, RealMatrix matrix,int a ,int b,int current,double load){
        this.type = type;

        Matrix = matrix;
        begin = a;
        end = b;
        this.current = current;
        workload = load;
    }




    RealMatrix getMatrix(){
        return MatrixUtils.createRealMatrix(this.Matrix.getData());
        //return Matrix;
    }

    public String getType(){
        return this.type;
    }

    public double getAlpha(){
        return this.alpha;
    }

    public double getLambda(){
        return this.lambda;
    }

    public int getf(){
        return this.f;
    }

    public int[] getIters(){
        return new int[]{begin,end};
    }

    public int Begin(){
        return begin;
    }

    public int End(){
        return end;
    }

    public int Current(){
        return current;
    }

    public double getWorkload(){
        return  this.workload;
    }
}
