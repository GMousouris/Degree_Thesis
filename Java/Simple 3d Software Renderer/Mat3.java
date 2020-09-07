public class Mat3 {


    double[][] elements;
    final double PI = 3.14159265359;

    public Mat3(){
        elements = new double[3][3];
    }

    public Mat3(double x1,double x2,double x3,
                double y1,double y2,double y3,
                double z1,double z2,double z3)
    {
        elements = new double[3][3];

        elements[0][0] = x1; elements[0][1] = x2; elements[0][2] = x3;
        elements[1][0] = y1; elements[1][1] = y2; elements[1][2] = y3;
        elements[2][0] = z1; elements[2][1] = z2; elements[2][2] = z3;
    }


    /*vector * Matrix ~ Multiplication*/
    public static Vec3 vector_mult(Vec3 vec,Mat3 mat){

        double x = vec.x*mat.elements[0][0] + vec.y*mat.elements[1][0] + vec.z*mat.elements[2][0];
        double y = vec.x*mat.elements[0][1] + vec.y*mat.elements[1][1] + vec.z*mat.elements[2][1];
        double z = vec.x*mat.elements[0][2] + vec.y*mat.elements[1][2] + vec.z*mat.elements[2][2];

        return new Vec3(x,y,z);
    }

    /*Matrix * l ~ Multiplication*/
    public void mult(double l){
        for(int i=0; i<3; i++){
            for(int j=0; j<3; j++){
                this.elements[i][j] = this.elements[i][j]*l;
            }
        }
    }

    /*Matrix * Matrix ~ Multiplication*/
    public Mat3 matrix_mult(Mat3 mat){
        Mat3 new_mat = new Mat3();

        double sum;
        for(int j=0; j<3; j++){
            for(int k =0; k<3; k++){
                sum = 0.0;

                for(int i=0; i<3; i++){
                    sum +=(this.elements[j][i]*mat.elements[i][k]);
                }
                new_mat.elements[j][k] = sum;
            }

        }
        return new_mat;
    }

    /*get Identity Matrix*/
    public Mat3 get_Identity_Matrix(){
        return new Mat3(1,0,0,
                        0,1,0,
                        0,0,1);
    }

    /*Matrix Scaling*/
    public Mat3 scaling(double f){
        return new Mat3(f,0,0,
                    0,    f,0,
                    0,0,    f);

    }

    /*Rotation on ~ Z axis*/
    public static Mat3 rotateZ(double t){

        Mat3 mat = new Mat3(
                Math.cos(t), -Math.sin(t),0,
                Math.sin(t),Math.cos(t),0,
                0,0,1);
        return mat;
    }

    /*Rotation on ~ X axis*/
    public static Mat3 rotateX(double t){

        Mat3 mat = new Mat3(
                1 ,0 ,0,
                0,Math.cos(t),-Math.sin(t),
                0,Math.sin(t),Math.cos(t));
        return mat;
    }

    /*Rotation on ~ Y axis*/
    public static Mat3 rotateY(double t){

        Mat3 mat = new Mat3(
                Math.cos(t) ,0 ,Math.sin(t),
                0,1 , 0,
                -Math.sin(t),0,Math.cos(t));
        return mat;
    }
}
