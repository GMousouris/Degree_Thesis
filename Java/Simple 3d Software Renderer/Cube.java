import java.util.ArrayList;

public class Cube extends gShape {

    private double side;
    private double var = 0.2;

    public Cube(double size){
        super(size);

        vertices = new ArrayList<Vec3>();
        faces = new ArrayList<Vec4>();

        side = size/2.0;
        construct();
    }

    public void construct(){

        side+=var;

        vertices.add(new Vec3(-side,-side,-side));
        vertices.add(new Vec3(side,-side,-side));
        vertices.add(new Vec3(-side,side,-side));
        vertices.add(new Vec3(side,side,-side));

        vertices.add(new Vec3(-side,-side,side));
        vertices.add(new Vec3(side,-side,side));
        vertices.add(new Vec3(-side,side,side));
        vertices.add(new Vec3(side,side,side));

        faces.add(new Vec4(0,1,3,2)); // mprosta
        faces.add(new Vec4(1,5,7,3)); //plagia de3ia
        faces.add(new Vec4(6,7,3,2)); //katw plevra
        faces.add(new Vec4(4,5,7,6)); // pisw
        faces.add(new Vec4(4,0,2,6)); //plagia aristera
        faces.add(new Vec4(4,5,1,0)); //panw plevra
    }

    public IndexedLinesList get_Lines(){

        ArrayList<Integer> indices = new ArrayList<Integer>();

        indices.add(0); indices.add(1);
        indices.add(1); indices.add(3);
        indices.add(3); indices.add(2);
        indices.add(2); indices.add(0);
        indices.add(0); indices.add(4);
        indices.add(1); indices.add(5);
        indices.add(3); indices.add(7);
        indices.add(2); indices.add(6);
        indices.add(4); indices.add(5);
        indices.add(5); indices.add(7);
        indices.add(7); indices.add(6);
        indices.add(6); indices.add(4);

        return new IndexedLinesList(this.vertices,indices);
    }

    public IndexedTriangleList get_Triangles(){

        ArrayList<Integer> indices = new ArrayList<Integer>();

        indices.add(0); indices.add(2); indices.add(1); //021
        indices.add(2); indices.add(3); indices.add(1); //231

        indices.add(1); indices.add(3); indices.add(5); //135
        indices.add(3); indices.add(7); indices.add(5); //375

        indices.add(2); indices.add(6); indices.add(3); //263
        indices.add(3); indices.add(6); indices.add(7); //367

        indices.add(4); indices.add(5); indices.add(7); //457
        indices.add(4); indices.add(7); indices.add(6); //476

        indices.add(0); indices.add(4); indices.add(2); //042
        indices.add(2); indices.add(4); indices.add(6); //246

        indices.add(0); indices.add(1); indices.add(4); //014
        indices.add(1); indices.add(5); indices.add(4); //154

        return new IndexedTriangleList(this.vertices,indices);

    }




}
