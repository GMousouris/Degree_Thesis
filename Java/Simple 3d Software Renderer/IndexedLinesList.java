import java.util.ArrayList;

public class IndexedLinesList {

    ArrayList<Vec3> vertices;
    ArrayList<Integer> indices;

    public IndexedLinesList(ArrayList<Vec3> vs,ArrayList<Integer> ics){

        this.vertices = new ArrayList<Vec3>(vs);
        this.indices = new ArrayList<Integer>(ics);

    }
}
