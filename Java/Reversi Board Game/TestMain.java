
public class TestMain
{
    protected int a;
    protected int b;

    public TestMain(int a , int b)
    {
        this.a = a;
        this.b = b;
    }

    public TestMain(TestMain copy)
    {
        this.a = copy.a;
        this.b = copy.b;
    }

    public void print()
    {
        System.out.println(a);
        System.out.println(b);
    }





    public static void main(String[] args)
    {

    }


}