import java.util.ArrayList;
import java.util.List;

public class Tree<T> {

    private Node<T> root;

    public Tree(T rootData) {

        root = new Node<T>();
        root.value = rootData;
        root.children = new ArrayList<>();
    }

    public Node<T> getRoot()
    {
        return root;
    }


    public static class Node<T> {

        private ArrayList<String> closed_set =  new ArrayList<>();
        private int depth = 0;
        private T value;
        private Node<T> parent = null;
        private List<Node<T>> children = new ArrayList<>();

        public Node(){}

        public Node(T value)
        {
            this.value = value;
        }

        public Node(T value , int depth)
        {
            this.value = value;
            this.depth = depth;
        }

        public List<Node<T>> getChildren()
        {
            return children;
        }

        public Node<T> getChild(int index)
        {
            if(index < children.size())
                return children.get(index);
            else
                return null;
        }

        public Node<T> getParent()
        {
            return parent;
        }

        public void addChild(Node<T> child)
        {
            child.parent = this;
            children.add(child);
        }

        public void setValue(T value)
        {
            this.value = value;
        }

        public T getValue()
        {
            return value;
        }

        public int getDepth() {return  depth;}

        public void addToClosedSet(String S)
        {
            closed_set.add(S);
        }

        public void setClosed_set(ArrayList<String> list)
        {
            closed_set = new ArrayList<>(list);
        }

        public ArrayList<String> getClosed_set() {
            return closed_set;
        }
    }


}