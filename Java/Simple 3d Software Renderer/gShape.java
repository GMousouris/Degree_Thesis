import java.util.ArrayList;

public class gShape {

    protected ArrayList<Vec3> vertices;
    protected ArrayList<Vec4> faces;
    protected double size;

    public gShape(){

    }

    public gShape(double size){
        this.size = size;
    }

    public ArrayList<Vec3> get_Vertices(){
        return this.vertices;
    }

    public IndexedLinesList get_Lines(){
        return null;
    }

    public IndexedTriangleList get_Triangles(){
        return null;
    }
}
