import java.lang.Math;

public class Vec3 {

    double x, y,z;

    public Vec3(double x,double y){
        this.x = x;
        this.y = y;
    }

    public Vec3(double x,double y,double z){
        this.x = x;
        this.y = y;
        this.z = z;
    }

    public Vec3(Vec3 src){
        this.x = src.x;
        this.y = src.y;
        this.z = src.z;
    }


    public void Clone(Vec3 src){
        this.x = src.x;
        this.y = src.y;
        this.z = src.z;
    }



    /*Inner (/dot) Product */
    public double dot_mult(Vec3 vec){
        return (x*vec.x+y*vec.y+z*vec.z);
    }

    public Vec3 cross_prodcut(Vec3 vec){

        double x1 = y*vec.z - z*vec.y;
        double y1 = z*vec.x - x*vec.z;
        double z1 = x*vec.y - y*vec.x;

        return new Vec3(x1,y1,z1);
    }


    /*Regular Multiplication*/
    public Vec3 mult(double l){
        this.x = l*x;
        this.y = l*y;
        this.z = l*z;

        return this;
    }

    /*Measurement of a vector*/
    public double get_measure(){
        double a = Math.pow(x,2);
        double b = Math.pow(y,2);
        double c = Math.pow(z,2);

        return Math.pow(a+b+c,0.5);
    }

    /*Addition*/
    public Vec3 add(Vec3 vec){
        this.x = x + vec.x;
        this.y = y + vec.y;
        this.z = z + vec.z;

        return this;
    }

    /*subtraction*/
    public Vec3 sub(Vec3 vec){
        this.x = x - vec.x;
        this.y = y - vec.y;
        this.z = z - vec.z;

        return this;
    }

    /*Normalization*/
    public void normalize(){
        double m = this.get_measure();
        this.x = x/m;
        this.y = y/m;
        this.z = z/m;
    }

    /*equals*/
    public boolean isEqual(Vec3 vec){
        if(x == vec.x && y == vec.y && z == vec.z){
            return true;
        }
        return false;

    }


}
