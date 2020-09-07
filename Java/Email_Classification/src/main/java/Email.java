public class Email
{

    private  int CLASS;
    private String id;

    public Email()
    {

    }

    public Email(String id , int CLASS)
    {
        this.id = id;
        this.CLASS = CLASS;
    }


    public String getId()
    {
        return id;
    }

    public int getCLASS()
    {
        return  CLASS;
    }
}
