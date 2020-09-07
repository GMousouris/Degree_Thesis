class Worker extends Thread
{

    private static boolean running = false;
    private static boolean equal = true;
    private boolean done = false;

    private int[] arrayA;
    private int[] arrayB;
    private int size;

    public Worker(int[] arrayA , int[] arrayB)
    {
        this.arrayA = arrayA;
        this.arrayB = arrayB;
        size = arrayA.length;
    }

    public void run()
    {

        int i = 0;
        while(running && i<size)
        {
            if(arrayA[i] != arrayB[i])
            {
                running = false;
                equal = false;
                break;
            }
            i++;
        }

    }


    public static void setRunning(boolean state)
    {
        running = state;
    }

    public static boolean get_State()
    {
        return equal;
    }

    public static boolean is_Running()
    {
        return running;
    }

    public static void reset()
    {
        equal = true;
        running = false;

    }
}