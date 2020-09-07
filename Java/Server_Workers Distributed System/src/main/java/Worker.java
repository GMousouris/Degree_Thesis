import java.net.*;
import java.io.*;
import java.util.ArrayList;
import java.util.Scanner;
import com.sun.management.OperatingSystemMXBean;
import java.lang.management.ManagementFactory;
import java.lang.Math;

import org.apache.commons.math3.linear.*;
import java.lang.Thread;
import java.util.Timer;
import java.util.TimerTask;


public class Worker{


    private Socket socket;

    //
    private double ram_usage;
    private double cpu;
    private double total;


    /*states*/
    private boolean IN = true;
    private boolean OUT = false;
    private boolean Alive = false;
    private boolean done = false;
    private boolean WORK_TO_DO = false;
    private String status = null;
    private DataMsg msgIN = null;
    /* */
    private ObjectInputStream in = null;
    private ObjectOutputStream out = null;
    //
    private commThread com_thread;
    private String calc = null;

    /* - - - */
    private RealMatrix X,Y;
    private RealMatrix rawData;
    private RealMatrix c;
    private RealMatrix p;

    private RealMatrix cu,ci;
    private RealMatrix pu,pi;

    private RealMatrix IY,IX,lI;

    private RealMatrix YTY;
    private RealMatrix XTX;

    private double alpha;
    private double lambda;
    private int f;
    /* - - - */

    //
    private int begin;
    private int end;
    private int atm_counter = -1;
    //
    private boolean INIT_P = true;
    private boolean PAUSE = false;
    private double currentload = 0.0;
    //

    private ArrayList<RealMatrix> result;

    private Timer ProgressTimer;
    private int timerDelay;
    private boolean timerEnabled;

    private int id;
    private boolean REASSIGNED_WORK = false;

    public Worker(int id,String address,int delay,boolean timerenabled){

        this.timerDelay = delay;
        this.timerEnabled = timerenabled;


        if(timerEnabled){
            ProgressTimer = new Timer();
        }

        result = new ArrayList<RealMatrix>();
        System.out.println("WORKER initialising . . .\n");

        try{
            System.out.println("Attempting Connection to server.\nAddress : "+address+" | Port : "+id);
            this.socket = new Socket(address,id);
            this.socket.setSoTimeout(3000);
            Alive = true;
            System.out.println("\nConnection to server established.");
        }
        catch(Exception e){
            e.printStackTrace();
        }

        try{
            in = new ObjectInputStream(this.socket.getInputStream());
            out = new ObjectOutputStream(this.socket.getOutputStream());
        }
        catch(IOException e){
            e.printStackTrace();
        }

        status = "WAITING_FOR_JOB";

        System.out.println("\nWaiting for Server to Initialize process . . .");
        com_thread = new commThread(this.socket,in,out);
        com_thread.start();

    }
    //


    public void sendInfoToServer(){

        while(calculateCPU_RAM()==-1){}

        System.out.println("Sending INFO back to server . . .");
        try{
            DataMsg msg_out = new DataMsg("INFO",new WorkerInfo(cpu,ram_usage,status));
            out.writeObject(msg_out);

        }
        catch(IOException e){
            e.printStackTrace();

        }

    }

     int calculateCPU_RAM(){
         /*calculating CPU/RAM usage*/
         OperatingSystemMXBean os = ManagementFactory.getPlatformMXBean(
                 OperatingSystemMXBean.class);
         double cpu_sum=0;

         while(true){

             double d = os.getSystemCpuLoad();
             if(d>0){

                 int counter = 0;
                 double[] cpu_sums = new double[10];

                 while(counter<10){

                     for(int i=0; i<100; i++){

                         cpu_sum+=os.getSystemCpuLoad();
                     }

                     cpu_sums[counter] = cpu_sum;
                     cpu_sum = 0;
                     counter++;
                 }
                 for(int i=0; i<cpu_sums.length; i++){
                     cpu_sum+=cpu_sums[i];
                 }
                 cpu_sum/=1000;
                 cpu = cpu_sum*100;
                 break;
             }
         }
         if(cpu<=2){
             return -1;
         }


         double ram = os.getFreePhysicalMemorySize();
         total = os.getTotalPhysicalMemorySize();

         this.cpu = (double)Math.round(cpu*1000d) / 1000d;
         ram_usage = 100 - (ram/total)*100;
         ram_usage = (double)Math.round(ram_usage*1000d) / 1000d;

         return 0;
     }

    public void processWork(){

        if(calc.equals("X")){
            calculateMatrixX();
        }
        else if(calc.equals("Y")){
            calculateMatrixY();
        }
    }


    /*-------------------*/
    class Progress extends TimerTask {


        public Progress(){

        }

        public void run(){

            int perc = (int)(atm_counter*100d/end);
            if(!PAUSE) System.out.println("~Progress : "+perc+"%");
        }
    }

    void timerPause(){

        ProgressTimer.cancel();
        ProgressTimer.purge();
    }

    void timerStart(){

        ProgressTimer = new Timer();
        ProgressTimer.schedule(new Progress(),0,timerDelay);
    }
    /*------------------------------------------------------------------------------------------------------------------*/
    /*Calculating Methods*/

    public void calculateMatrixX(){

        System.out.println("_Calculating : Xu_");
        result.clear();

        if(!REASSIGNED_WORK){

            YTY = calculateMatrixYTY();
            IY = MatrixUtils.createRealIdentityMatrix(rawData.getColumnDimension());
            lI = MatrixUtils.createRealIdentityMatrix(f).scalarMultiply(lambda);
        }

        RealMatrix Xu = null;

        if(timerEnabled) timerStart();


        long start = System.nanoTime();
        for(int u = begin; u<end; u++){


            atm_counter = u;

            cu = calculateMatrixCu(u);

            if(INIT_P){
                pu = p.getRowMatrix(u);
            }
            else{
                pu = calculatePu(u);
            }

            RealMatrix cu_i = cu.subtract(IY);

            Xu = Y.transpose().multiply(cu_i);
            Xu = Xu.multiply(Y);
            Xu = Xu.add(YTY);
            Xu = Xu.add(lI);
            Xu = invertMatrix(Xu);

            Xu = Xu.multiply(Y.transpose());
            Xu = Xu.multiply(cu);
            Xu = Xu.multiply(pu.transpose());


            result.add(Xu);

            while(PAUSE){
                try{
                    Thread.sleep(10);
                }
                catch(InterruptedException e){}
            }
        }

        long endd = System.nanoTime();
        if(timerEnabled) timerPause();

        System.out.flush();
        double time_elapsed = (endd-start)/1000000000;
        time_elapsed = (double)Math.round(time_elapsed*1000d)/1000d;

        System.out.println("DONE. ("+time_elapsed+"s)");
        done = true;
        WORK_TO_DO = false;


        RealMatrix matrix = MatrixUtils.createRealMatrix(f,result.size());
        for(int i=0; i<matrix.getColumnDimension(); i++){
            matrix.setColumnMatrix(i,result.get(i));
        }

        System.out.println("\nRESULT = ["+calc+"] : {"+matrix.getRowDimension()+" , "+matrix.getColumnDimension()+"}");
        System.out.println("~Sending results to master . . .");


        DataObject obj = new DataObject("RESULT",matrix,begin,end);
        sendData(new DataMsg("DATA",obj));

    }

    public void calculateMatrixY(){

        System.out.println("_Calculating : Yi_");
        result.clear();


        if(!REASSIGNED_WORK){
            XTX = calculateMatrixXTX();
            IX = MatrixUtils.createRealIdentityMatrix(rawData.getRowDimension());
            lI = MatrixUtils.createRealIdentityMatrix(f).scalarMultiply(lambda);
        }

        RealMatrix Yi = null;
        if(timerEnabled) timerStart();

        long start = System.nanoTime();
        for(int i = begin; i<end; i++){

            atm_counter = i;

            ci = calculateMatrixCi(i);
            if(INIT_P){
                pi = p.getColumnMatrix(i);
            }
            else{
                pi = calculatePi(i);
            }

            RealMatrix ci_i = ci.subtract(IX);

            Yi = X.transpose().multiply(ci_i);
            Yi = Yi.multiply(X);
            Yi = Yi.add(XTX);
            Yi = Yi.add(lI);
            Yi = invertMatrix(Yi);

            Yi = Yi.multiply(X.transpose());
            Yi = Yi.multiply(ci);
            Yi = Yi.multiply(pi);

            result.add(Yi);

            while(PAUSE){
                try{
                    Thread.sleep(10);
                }
                catch(InterruptedException e){}
            }

        }
        //

        if(timerEnabled) timerPause();
        long endd = System.nanoTime();

        double time_elapsed = (endd-start)/1000000000;
        time_elapsed = (double)Math.round(time_elapsed*1000d)/1000d;

        System.out.flush();
        System.out.println("DONE. ("+time_elapsed+"s)");

        done = true;
        WORK_TO_DO = false;

        RealMatrix matrix = MatrixUtils.createRealMatrix(f,result.size());
        for(int i=0; i<matrix.getColumnDimension(); i++){
            matrix.setColumnMatrix(i,result.get(i));
        }

        System.out.println("\nRESULT = ["+calc+"] : {"+matrix.getRowDimension()+" , "+matrix.getColumnDimension()+"}");
        System.out.println("~Sending results to master . . .");


        DataObject obj = new DataObject("RESULT",matrix,begin,end);
        sendData(new DataMsg("DATA",obj));

    }



    public RealMatrix calculateMatrixC(){


        double[][] c_temp = new double[rawData.getRowDimension()][rawData.getColumnDimension()];
        for(int i=0; i<rawData.getRowDimension(); i++){
            for(int j=0; j<rawData.getColumnDimension(); j++){
                c_temp[i][j] = 1 + alpha*rawData.getEntry(i,j);
            }
        }
        return MatrixUtils.createRealMatrix(c_temp);
    }

    public RealMatrix calculateMatrixP(){

        if(rawData == null){
            return null;
        }
        double[][] p = new double[rawData.getRowDimension()][rawData.getColumnDimension()];
        for(int i=0; i<rawData.getRowDimension(); i++){
            for(int j=0; j<rawData.getColumnDimension(); j++){
                if(rawData.getEntry(i,j)>0){
                    p[i][j] = 1;
                }
                else{
                    p[i][j] = 0;
                }
            }
        }
        return MatrixUtils.createRealMatrix(p);

    }

    public RealMatrix calculateMatrixCu(int u){

        RealMatrix u_row = c.getRowMatrix(u); // getting : u row from c Matrix ~ c[u,:] //

        double[] u_row_data = new double[u_row.getColumnDimension()];
        for(int i = 0; i<u_row_data.length; i++){
            u_row_data[i] = u_row.getEntry(0,i);
        }
        return MatrixUtils.createRealDiagonalMatrix(u_row_data);
    }

    public RealMatrix calculateMatrixCi(int i){

        RealMatrix i_column = c.getColumnMatrix(i);

        double[] i_column_data = new double[i_column.getRowDimension()];
        for(int q = 0; q<i_column_data.length; q++){
            i_column_data[q] = i_column.getEntry(q,0);
        }
        return MatrixUtils.createRealDiagonalMatrix(i_column_data);
    }

    public RealMatrix calculatePu(int u){

        RealMatrix pu = c.getRowMatrix(u);

        for(int i = 0; i<c.getColumnDimension(); i++){
            if(pu.getEntry(0,i)!=0){
                pu.setEntry(0,i,1.0);
            }
        }

        return pu;
    }

    public RealMatrix calculatePi(int i){

        RealMatrix pi = c.getColumnMatrix(i);

        for(int j = 0; j<c.getRowDimension(); j++){
            if(pi.getEntry(j,0)!=0){
                pi.setEntry(j,0,1.0);
            }
        }

        return pi;
    }

    public RealMatrix calculateMatrixYTY(){

        RealMatrix YTY = Y.transpose().multiply(Y);
        return YTY;
    }

    public RealMatrix calculateMatrixXTX(){

        RealMatrix XTX = X.transpose().multiply(X);
        return XTX;

    }


    public RealMatrix invertMatrix(RealMatrix m){

        RealMatrix I = MatrixUtils.createRealIdentityMatrix(m.getRowDimension());
        DecompositionSolver solver = new LUDecomposition(m).getSolver();

        return solver.solve(I);

    }

    public void initialize_basic_matrices(){
        c = calculateMatrixC();
        if(INIT_P){
            p = calculateMatrixP();
        }
    }

    /*------------------------------------------------------------------------------------------------------------------*/
    /*Communications Thread*/
    class commThread extends Thread{

        private Socket socket;

        public commThread(Socket socket,ObjectInputStream in,ObjectOutputStream out){
            this.socket = socket;
        }


        public void run(){

            Object obj;

            while(Alive){

                obj = null;

                try{
                    Thread.sleep(5);
                }
                catch(Exception e){}

                /*Always Reading*/
                try{
                    obj = in.readObject();
                }
                catch(SocketException e){}
                catch(IOException e){}
                catch(Exception e){}

                /*processing server's response*/
                if(obj!=null){
                    //System.out.println("\nGot data from server.");
                    msgIN = (DataMsg)obj;
                    processMsg(msgIN);
                    //msgIN = null; obj = null;
                }
                /* */



            }//while

        }//run

    }//commThread

    void sendData(DataMsg msg){

        try{
            out.writeObject(msg);
            out.flush();
        }
        catch (Exception e){
            System.out.println("****failed to send data to server!");
        }

    }

    void processMsg(DataMsg msg){

        //System.out.println("Processing data . . .");
        String type = msg.getType();
        if(!type.equals("EXTRA_WORK")){REASSIGNED_WORK = false;}
        DataObject data = null;

        if(type.equals("DATA")){
            data = msg.getData();

            String data_type = data.getType();
            if(data_type.equals("X")){
                X = data.getMatrix();
                calc = "Y";
                begin = data.Begin();
                end = data.End();
                currentload = data.getWorkload();

                //System.out.println("\n---------------------------------------\nRECEIVED : X[0] : "+X.getRowMatrix(0));
                System.out.println("- - - - - - - - - - - -\n\nDATA RECEIVED : [X]\nIterations : {"+begin+" , "+end+"}\nWork assigned : "+currentload+"%");
                System.out.print("\n- - - - - - - - - - - -");
                WORK_TO_DO = true;
            }
            else if(data_type.equals("Y")){
                Y = data.getMatrix();
                calc = "X";
                begin = data.Begin();
                end = data.End();
                currentload = data.getWorkload();

                //System.out.println("\n---------------------------------------\nRECEIVED : Y[0] : "+Y.getRowMatrix(0));
                System.out.println("- - - - - - - - - - - -\n\nDATA RECEIVED : [Y]\nIterations : {"+begin+" , "+end+"}\nWork assigned : "+currentload+"%");
                System.out.print("\n- - - - - - - - - - - -");
                WORK_TO_DO = true;
            }
            else if(data_type.equals("RAW_DATA")){
                rawData = data.getMatrix();
                alpha = data.getAlpha();
                lambda = data.getLambda();
                f = data.getf();

                initialize_basic_matrices();
                System.out.println("- - - - - - - - - - - -\n\nDATA : [r]\na : "+alpha+"\nlambda : "+lambda+"\nf : "+f);
                System.out.print("\n- - - - - - - - - - - -");
                System.out.println("Data initialized, waiting for work . . .");


            }
        }
        else if(type.equals("INFO")){
            sendInfoToServer();
        }
        else if(type.equals("ID")){
            this.id = msg.getID();
            System.out.println("~ID assigned to this worker : "+this.id);
        }
        else if(type.equals("HEARTBEAT")){
            sendData(new DataMsg("HEARTBEAT",null));
            //System.out.println("\n[[ ...---__-^--_--^_---... ]]\n");
        }
        else if(type.equals("PAUSE")){
            PAUSE = true;
            System.out.println("~Main Processing Thread : PAUSED.");
        }
        else if(type.equals("RESUME")){
            PAUSE = false;
            System.out.println("~Main Processing Thread : RESUMED.");
        }
        else if(type.equals("WORK_PROGRESS")){
            calculateCPU_RAM();
            double overall_dist = end - begin;
            double current_dist = (double)(atm_counter - begin);
            double workprogress = (current_dist*100d)/overall_dist;

            DataObject dataobj = new DataObject("WORK_PROGRESS",null,begin,end,atm_counter,workprogress);
            WorkerInfo inf = new WorkerInfo(cpu,ram_usage);

            DataMsg msgout = new DataMsg("WORK_PROGRESS",dataobj,inf);
            sendData(msgout);
            //System.out.println("\n~Sent Work progress to Master : begin : ["+begin+"] | current : ["+atm_counter+"] | end : ["+end+"]\n");
        }
        else if(type.equals("SPLITED_WORK")){

            System.out.println("\n- - - - - - - - - - - - - - - - - - - - - - -\n~Splitting workload from this Worker.\n- - - - - - - - - - - - - - - - - - - - - - -");
            System.out.print("\n- - - - - - - - - - - -");
            data = msg.getData();
            end = data.End();
            currentload = data.getWorkload();
            System.out.println("\nIterations : {"+atm_counter+" , "+end+"}");
            System.out.println("NEW Work assigned : "+currentload+"%");

        }
        else if(type.equals("EXTRA_WORK")){

            REASSIGNED_WORK = true;
            data = msg.getData();

            begin = data.Begin();
            end = data.End();
            currentload = data.getWorkload();
            System.out.println("\n- - - - - - - - - - - - - - - - - - - - - - -\n~Re-assigned work to do , from another Worker.\n- - - - - - - - - - - - - - - - - - - - - - -");
            System.out.println("- - - - - - - - - - - -\n\nDATA : [NULL]\nIterations : {"+begin+" , "+end+"}\nWork assigned : "+currentload+"%");
            System.out.print("\n- - - - - - - - - - - -");
            WORK_TO_DO = true;
            //PAUSE = false;
        }


    }
    /*------------------------------------------------------------------------------------------------------------------*/
    public static void main(String[] args){

        int timerDelay = 7000;
        boolean timerEnabled = true;

        String address = "91.140.89.104";
        int port = 6060;

        boolean input = false;


        if(input){
            Scanner sc = new Scanner(System.in);
            System.out.print("\nIP address : ");
            address = sc.nextLine();

            System.out.print("\nServer port : ");
            port = sc.nextInt();
        }


        Worker worker = new Worker(port,address,timerDelay,timerEnabled);

        while(true){
            try{
                Thread.sleep(5);
            }
            catch(Exception e){}
            if(worker.WORK_TO_DO){
                System.out.println("\nProcessing work . . .");
                worker.processWork();
                System.out.println("\nWaiting for work . . .");
            }

        }
    }

}