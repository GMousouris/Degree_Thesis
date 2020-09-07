public class Pair
{
    private Object X;
    private Object Y;

    public Pair(Object X, Object Y)
    {
        this.X = X;
        this.Y = Y;
    }

    public Object getIndex(int index)
    {
        if(index == 0)
            return X;
        else
            return Y;
    }
}
