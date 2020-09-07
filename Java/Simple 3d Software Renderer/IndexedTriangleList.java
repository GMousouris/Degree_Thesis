import java.util.ArrayList;

public class IndexedTriangleList {


    ArrayList<Vec3> vertices;
    ArrayList<Integer> indices;
    ArrayList<Boolean> flags;


    public IndexedTriangleList(ArrayList<Vec3> vrts,ArrayList<Integer> indcs){

        this.vertices = new ArrayList<Vec3>(vrts);
        this.indices = new ArrayList<Integer>(indcs);
        this.flags = new ArrayList<Boolean>();

        /*initializing flags = false*/
        for(int i=0; i<indices.size()/3; i++){
            flags.add(false);
        }
    }


}
