import org.apache.commons.math3.linear.*;
import org.apache.commons.math3.random.JDKRandomGenerator;
import org.apache.commons.math3.random.RandomGenerator;

import javax.swing.*;
import java.io.*;
import java.net.ServerSocket;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Stack;


public class MasterNode {


    private JTextArea pipe;

    private ArrayList<Index> indices;
    private ArrayList<Double> values;

    private int sizeX; //size X of input_data Array
    private int sizeY; //size Y of input_data Array

    private RealMatrix r; //raw_data matrix
    private RealMatrix p; //prefference matrix
    private RealMatrix c; //confidence matrix


    /*state flags*/
    private boolean data_init = false;
    private boolean server_init = false;
    private String message = "";
    //

    private ServerSocket server_socket;
    private int port;

    private ArrayList<ClientWorker> Workers;

    //private Timer timer;
    private DataTrainer trainer;
    private Thread trainingThread;

    private Application GUI;
    private MyNode serverNode;


    /*Master Constructor*/
    public MasterNode(Application gui){

        indices = new ArrayList<Index>();
        values = new ArrayList<Double>();
        Workers = new ArrayList<ClientWorker>();

        this.GUI = gui;
        this.pipe = GUI.getPipe();
        trainer = new DataTrainer();

    }

    /*Default*/
    public MasterNode(){
        indices = new ArrayList<Index>();
        values = new ArrayList<Double>();
    }

    /*readFile Function
    / reads data from a file given.
     */
    public int readFile(String filepath){

        //filepath = "c:/java/dataset.csv";

        try{
            FileReader in = new FileReader(new File(filepath));
            BufferedReader br = new BufferedReader(in);

            String line;
            while((line = br.readLine()) != null) {

                if(parseLine(line.replaceAll("\\s", ""),filepath)!=0){
                    message = "\n-error while trying to parse data from file : "+filepath+" !";
                    return -1;
                }
            }
        }
        catch(IOException e){
            message = "\n-error while trying to read data from file : "+filepath+" !";
            return -1;
        }

        //message = "Data succesfully got read from file : "+filepath+" !";
        InitializeData();
        message = "\n-Data succesfully got initialized from file : "+filepath+" !";
        return 0;
    }
    /* */


    /*Parsing each line from File
    given to read data.
     */
    int parseLine(String line,String filepath){


        int counter = 0;
        int sep_counter = 0;
        String[] tokens = new String[3];

        for(int i=0; i<line.length(); i++){
            if(line.charAt(i)==','){

                tokens[counter] = line.substring(sep_counter,i);
                sep_counter = i+1;
                counter++;

                if(counter==2){
                    tokens[counter] = line.substring(sep_counter,line.length());
                    break;
                }
            }
        }
        try{
            indices.add(new Index(Integer.parseInt(tokens[0]),Integer.parseInt(tokens[1])));
            values.add(Double.parseDouble(tokens[2]));
        }
        catch(Exception ee){
            System.out.println("~error while trying to parse input data from "+filepath+" file!\nProcedure reseting!");
            indices.clear();
            values.clear();
            return -1;
        }

        return 0;
    }
    /* */

    /*Initializes the input data from file
    /to internal Arrays/Matrices.
     */
    public void InitializeData(){

        sizeX = 765;
        sizeY = 1964;


        /*constructing double[][] array from input data in order to initialize RealMatrix*/
        double[][] in_data = new double[sizeX][sizeY];
        for(int i = 0; i < indices.size(); i++){
            in_data[indices.get(i).i()][indices.get(i).j()] = values.get(i);
        }

        r = MatrixUtils.createRealMatrix(in_data);
        data_init = true;

    }
    /* */


    /*Method responsible for
    starting the Training Process with
    given Parameters.
     */
    public void trainData(double a_var,double l_var,int f_var,int iterations_var,double error_val){

        //trainer = new DataTrainer();
        trainer.setParams(a_var,l_var,f_var,iterations_var,error_val);
        trainingThread = new Thread(trainer);
        trainingThread.start();
    }
    /*------------------------------------------------------------------------------------------------------------------*/



    /*DataTrainer Class
    Thread Responsible for training our Data matrices[X,Y]
     */
    public class DataTrainer implements Runnable{

        /*Training parameters*/
        private double a;
        private double l;
        private int f;
        private int iterations;
        private double error;
        private int rec_list_size = 5;

        private ArrayList<Double> costs;
        private ArrayList<Double> errors;

        /*Distribution parameters*/
        private int d_priority = 0;
        private double d_treshold = 0.7;
        private int sleepTime = 100;

        /*Matrices*/
        private  RealMatrix X;
        private RealMatrix Y;
        private RealMatrix activeMatrix = null;

        /*Worker lists*/
        private ArrayList<DataObject> workers_results;
        private ArrayList<ClientWorker> inactive_workers;
        private ArrayList<ClientWorker> active_workers;
        private ArrayList<ClientWorker> available_workers;

        /*Matrix Updater Thread*/
        private ArrayList<Thread> updatingThreads;

        /*flags*/
        private boolean synch = true;
        private boolean redistribute = true;
        private boolean RUNNING = false;
        private boolean W_PROCESSING = false;

        /*DataTrainer Constructor*/
        public DataTrainer(){

            workers_results = new ArrayList<DataObject>();
            inactive_workers = new ArrayList<ClientWorker>();
            active_workers = new ArrayList<ClientWorker>();
            available_workers = new ArrayList<ClientWorker>();
            updatingThreads = new ArrayList<Thread>();

            costs = new ArrayList<Double>();
            errors = new ArrayList<Double>();


        }

        /*Setting Parameters for Data Training*/
        public void setParams(double a_var,double l_var,int f_var,int iterations_var,double error_val){

            //if(f>=5 && f<=10)d_treshold = 0.7;
            if(f<10)d_treshold = 0.80;
            else if(f>=10 && f<=15)d_treshold = 0.90;
            else if(f>15 && f<=20)d_treshold = 0.95;
            //else if(f>20 && f<=30)d_treshold = 0.95;
            else d_treshold = 0.95;

            this.a = a_var;
            this.l = l_var;
            this.f = f_var;
            this.iterations = iterations_var;
            this.error = error_val;

        }

        /*TRAINING FUNCTION*/
        public void run(){

            RUNNING = true;
            GUI.activateStop();
            GUI.deactivateStart();

            pipe.append("\n__TrainingProcess__");
            pipe.append("\n- - - - - - - - - -");
            pipe.append("\nVARIABLES:");
            pipe.append("\n  _alpha : "+this.a);
            pipe.append("\n  _f[size] : "+this.f);
            pipe.append("\n  _lambda : "+this.l);
            pipe.append("\n  _iterations : "+this.iterations);
            pipe.append("\n  _maximum error : "+this.error);
            pipe.append("\n- - - - - - - - - -\n");


            p = calc_Matrix_p(); /*calculate P~prefference Matrix*/
            c = calc_Matrix_c(a); /*calculate C~confidence Matrix*/

            X = generateRandomMatrix(r.getRowDimension(),this.f,3);
            Y = generateRandomMatrix(r.getColumnDimension(),this.f,3);

            pipe.append("\n-Data : Ready.\n-Computed Matrices : c,p.\n-Generated random initiated Matrices : X,Y.\n");


            int leftovers = 0;
            double[] xi = null;/*used to store worker's weight for current job*/
            int iter = 0;

            long start;
            long end;

            while(iter < iterations && RUNNING){


                start = System.nanoTime();
                available_workers = new ArrayList<ClientWorker>(getAvailableWorkers()); /*storing available workers -> available_Workers<>*/

                if(iter == 0){
                    for(ClientWorker c : available_workers){
                        sendWorkerRawData(c,r,a,l,f);
                    }
                }

                /* - - - - - - <COMPUTING Xu> - - - - - - */
                pipe.append("\n\n_______COMPUTING : [Xu]_______");
                activeMatrix = X;
                workers_results.clear();

                sort_av_workers(available_workers); /*sorting available workers by cpu_usage > ram_usage*/
                xi = calc_work_perc_forWorkers(available_workers , r.getRowDimension()); /*calculating work percentage for each available worker*/

                int sum=0;
                for(int i = 0; i<xi.length; i++){
                    sum+=xi[i];
                }
                leftovers = r.getRowDimension() - sum;

                /*distributing %Y to workers ~ computing Xu*/
                distributeWork(available_workers,Y,xi,leftovers,r.getRowDimension(),"Y");

                active_workers = new ArrayList<ClientWorker>(available_workers);
                inactive_workers = new ArrayList<ClientWorker>();

                /*checking workers state*/
                W_PROCESSING = true;
                while(active_workers.size()>0){

                    checkWorkers(active_workers,inactive_workers,workers_results);

                    if(redistribute){
                        if(inactive_workers.size()>0 && active_workers.size()>0){
                            reDistributeWork(inactive_workers,active_workers,d_priority,d_treshold);
                        }
                    }
                    try{
                        Thread.sleep(sleepTime);
                    }
                    catch (Exception e){}
                }
                W_PROCESSING = false;
                //

                /*waiting updating threads to finish*/
                if(synch){
                    waitUpdatingThreads(updatingThreads);
                }
                else{
                    X = AssembleResults(workers_results,X,"X");
                }
                /* - - - - - - </COMPUTING Xu> - - - - - --------------------------------------------------- */



                /* - - - - - - <COMPUTING Yi>- - - - - ----------------------------------------------------- */
                pipe.append("\n\n_______COMPUTING : [Yi]_______");
                activeMatrix = Y;
                workers_results.clear();

                available_workers = new ArrayList<ClientWorker>(getAvailableWorkers());
                getWorkersPower(available_workers);

                sort_av_workers(available_workers); /*sorting available workers by cpu_usage > ram_usage*/
                xi = calc_work_perc_forWorkers(available_workers , r.getColumnDimension()); /*calculating work percentage for each available worker*/

                sum=0;
                for(int i = 0; i<xi.length; i++){
                    sum+=xi[i];
                }
                leftovers = r.getColumnDimension() - sum;
                /* */

                /*distributing %X to workers ~ computing Yi*/
                distributeWork(available_workers,X,xi,leftovers,r.getColumnDimension(),"X");

                active_workers = new ArrayList<ClientWorker>(available_workers);
                inactive_workers = new ArrayList<ClientWorker>();

                /*checking workers state*/
                W_PROCESSING = true;
                while(active_workers.size()>0){

                    checkWorkers(active_workers,inactive_workers,workers_results);

                    if(redistribute){
                        if(inactive_workers.size()>0 && active_workers.size()>0){
                            reDistributeWork(inactive_workers,active_workers,d_priority,d_treshold);
                        }
                    }
                    try{
                        Thread.sleep(sleepTime);
                    }
                    catch(Exception e){}
                }
                W_PROCESSING = false;
                //


                if(synch){
                    waitUpdatingThreads(updatingThreads);
                }
                else{
                    Y = AssembleResults(workers_results,Y,"Y");
                }
                /* - - - - - - </COMPUTING Yi> - - - - - - */

                end = System.nanoTime();
                long time = (end - start)/1000000000;
                time = (long)Math.round(time*1000l)/1000l;

                costs.add(calculateCostFunction(l,X,Y));
                pipe.append("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n          ITERATION : "+iter+" ("+time+"s)\n          COST_FUNCTION["+iter+"] = "+costs.get(iter));
                if(iter>0){
                    errors.add(Math.abs(costs.get(iter)-costs.get(iter-1)));
                    pipe.append("\n          ERROR = "+errors.get(iter-1));
                }
                pipe.append("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
                iter++;

            }
            //end of loop//
            RUNNING = false;

            pipe.append("\n- - - - - - - - - - - - - - - - - - - - - -\n~TRAINING COMPLETED~");
            pipe.append("\nITERATIONS : "+iter);
            pipe.append("\nCost : "+costs.get(costs.size()-1));
            pipe.append("\nERROR : "+errors.get(errors.size()-1));
            pipe.append("\nMax Error : "+error);
            pipe.append("\n- - - - - - - - - - - - - - - - - - - - - ");

            pipe.append("\n~Reccomendations are now enabled!");

            GUI.activateReccomendations();
            GUI.activateStart();
            GUI.deactivateStop();

            for(ClientWorker c : Workers){
                System.out.println("\n------------------------------");
                System.out.println("Worker : "+c.getID());
                System.out.println("Splitted work : "+c.getSplittedWork());
                System.out.println("Extra work : "+c.getExtraWork());
                System.out.println("");
            }


        }
        /* */


        /*- - - Data Trainer Methods - - - - - - - - - - - - - - - - - -*/

        /*Calculates Cost Function for current Iteration*/
        double calculateCostFunction(double lambda,RealMatrix X,RealMatrix Y){
            pipe.append("\n-Calculating cost function . . .");
            double sum1 = 0;
            double sum2 = 0;
            double sum3 = 0;

            double cost = 0;
            RealMatrix XuTYi;
            double pui = 0;
            double cui = 0;

            for(int u = 0; u<r.getRowDimension(); u++){

                for(int i = 0; i<r.getColumnDimension(); i++){

                    //XuTYi = X.getRowMatrix(u).transpose();
                    //XuTYi = XuTYi.multiply(Y.getRowMatrix(i));
                    RealVector XuT = MatrixUtils.createRealVector(X.getRow(u)); //new
                    RealVector Yi = MatrixUtils.createRealVector(Y.getRow(i)); //new

                    double XuYi = XuT.dotProduct(Yi);


                    cui = c.getEntry(u,i);
                    pui = p.getEntry(u,i);

                    pui = pui - XuYi; //new
                    //pui = pui - XuTYi.getEntry(0,0);
                    pui = Math.pow(pui,2);
                    pui*=cui;

                    sum1+=pui;

                }
            }

            for(int u = 0; u<X.getRowDimension(); u++){

                RealVector Vu = MatrixUtils.createRealVector(X.getRow(u)); //new
                sum2+= Math.pow(Vu.getNorm(),2); //new
                //RealMatrix Xu = X.getRowMatrix(u);
                /*for(int i = 0; i<Xu.getColumnDimension(); i++){
                    sum2+=Math.pow(Xu.getEntry(0,i),2);
                }*/
            }

            for(int i = 0; i<Y.getRowDimension(); i++){
                RealVector Vi = MatrixUtils.createRealVector(Y.getRow(i)); //new
                sum3+= Math.pow(Vi.getNorm(),2); //new
                //RealMatrix Yi = Y.getRowMatrix(i);
                /*for(int j = 0; j<Yi.getColumnDimension(); j++){
                    sum3+=Math.pow(Yi.getEntry(0,j),2);
                }*/
            }

            return sum1 + l*(sum2+sum3);

        }

        double calculateScore(int u ,int i){
            RealVector XuT = MatrixUtils.createRealVector(X.getRow(u));
            RealVector Yi = MatrixUtils.createRealVector(Y.getRow(i));
            return XuT.dotProduct(Yi);
        }

        void ReccomendBestLocalPois(int user){

            ArrayList<Index> list = new ArrayList<Index>();
            ArrayList<Integer> recList = new ArrayList<Integer>();

            for(int i = 0; i<r.getColumnDimension(); i++){
                list.add(new Index(calculateScore(user,i),i));
            }


            SelectionSort(list);
            int counter = 0;
            int i = 0;

            while(recList.size()<rec_list_size){

                int poi = list.get(i).j();

                if(r.getEntry(user,poi)<=0) {
                    recList.add(poi);
                }

                i++;
            }

            pipe.append("\n - - - - - - - - - - - - - - - - - -");
            pipe.append("\n-Top "+rec_list_size+" Reccomendations for User : "+user);
            for(int q =0; q<recList.size(); q++){
                pipe.append("\n     | POI["+q+"] : "+recList.get(q)+" |");
            }
            pipe.append("\n - - - - - - - - - - - - - - - - - -");

        }

        /*Adds an updatingThread to current updatingThreads List*/
        public void addUpdatingThread(Thread updaterThread){
            updatingThreads.add(updaterThread);
        }

        /*if synchronized_update = true ,
        Waiting for each updatingThread to finish ,before
        continuing Training process.
         */
        public void waitUpdatingThreads(ArrayList<Thread> updatingThreads){
            for(Thread updaterThread : updatingThreads){
                try{
                    updaterThread.join();
                }catch(Exception e){}
            }
            updatingThreads.clear();
        }

        /*Reassembles results from workers, once they are all done*/
        RealMatrix AssembleResults(ArrayList<DataObject> obs,RealMatrix m,String name){

            pipe.append("\n\n-Getting results from workers.");
            pipe.append("\n-Updating Matrix : ["+name+"]");

            RealMatrix XY = MatrixUtils.createRealMatrix(m.getRowDimension(),m.getColumnDimension());

            for(DataObject d : obs){

                RealMatrix matrix = d.getMatrix();
                matrix = matrix.transpose();

                int a = d.Begin();
                int b = d.End();

                for(int i = a; i<b; i++){
                    XY.setRowMatrix(i,matrix.getRowMatrix(i-a));
                }
            }

            return XY;

        }

        /*Checks if worker(s) done work.
        if a worker is done , worker's results get immediatelly processed , or stored
        depending on , if synchronized_update = true | false.
         */
        void checkWorkers(ArrayList<ClientWorker> workers,ArrayList<ClientWorker> inactive, ArrayList<DataObject> results){


            ArrayList<ClientWorker> removable = new ArrayList<ClientWorker>();

            for(ClientWorker c : workers){
                if(c.isDone()){

                    if(synch){
                        DataUpdater updater = new DataUpdater(c.getWorkerData(),activeMatrix);
                        Thread updatingThread = new Thread(updater);
                        trainer.addUpdatingThread(updatingThread);
                        updatingThread.start();
                    }
                    else{
                        results.add(c.getWorkerData()); // synchronized update ston X,Y.
                    }
                    removable.add(c);
                }
            }

            for(ClientWorker c : removable){
                workers.remove(c);
                inactive.add(c);
            }
        }

        /*Distributes Workload% to Workers*/
        void distributeWork(ArrayList<ClientWorker> workers,RealMatrix matrix,double[] xi,int leftovers,int size,String matrix_name){
            serverNode.setProcessing(true);

            pipe.append("\n\n-Distributing Matrix : "+matrix_name+" to "+workers.size()+" workers . . .");
            int begin = 0;
            int end = 0;
            int counter = 0;

            for(ClientWorker c : workers){

                end+= (int)xi[counter];
                if(counter == 0){
                    end+=leftovers;
                }

                double perc = (end-begin);
                perc = (double)perc/size;
                perc*=100d;
                perc = (double)Math.round(perc*1000d)/1000d;
                c.setWorkload(perc);

                RealMatrix m = MatrixUtils.createRealMatrix(matrix.getData());

                DataObject obj = new DataObject(matrix_name,m,begin,end,perc);
                DataMsg data = new DataMsg("DATA",obj);
                c.distributeWork(data);

                pipe.append("\n     Worker "+c.getID()+" assigned : "+perc+"% of total work.");

                begin = end;
                counter++;
            }
            serverNode.setProcessing(false);
        }

        /*Function responsible for handling overall
        workload redistribution amongst workers.
         */
        void reDistributeWork(ArrayList<ClientWorker> inactive,ArrayList<ClientWorker> active,int priority,double treshold) {

            serverNode.setProcessing(true);
            ArrayList<ClientWorker> removable = new ArrayList<ClientWorker>();
            for (ClientWorker c : inactive) {
                if(!manageInactiveWorker(c, active, priority, treshold)){break;}
                else{
                    removable.add(c);
                }
            }

            for(ClientWorker c : removable){
                c.setSelectable(true);
                inactive.remove(c);
            }
            serverNode.setProcessing(false);
        }

        /*Handles inactive worker by distributing Workload from an active overloaded worker*/
        boolean manageInactiveWorker(ClientWorker worker,ArrayList<ClientWorker> active_workers,int priority,double treshold){


            ArrayList<ClientWorker> removable = new ArrayList<ClientWorker>();

            int PRIORITY = priority; // (0 for Work_done , 1 for CPU usage)
            ClientWorker selected = null;
            DataMsg datamsg = null;
            double max = -10;
            double cpu;
            double work_done;
            double f = -10;
            double distribution_treshold = treshold;


            for(ClientWorker c : active_workers) {
               if(c.isDone()){
                   continue;
               }
               else{
                   if(c.isSelectable()){
                       c.Pause();

                       c.getWorkProgress();
                       c.getWorkerData();
                       cpu = c.getCpuUsage()/100d;
                       work_done = c.getWorkDone()/100d;

                       if(work_done>distribution_treshold) {
                           c.setSelectable(false);
                           if(c.isPaused()){
                               c.Resume();
                           }
                           continue;
                       }

                       if(PRIORITY == 0){
                           f = 0.8*(1-work_done) + 0.2*(cpu -1);
                       }
                       else {
                           f = 0.3*(1-work_done) + 0.7*(cpu -1);
                       }
                       if(f > max){

                           max = f;
                           selected = c;
                       }
                   }
               }
            }
            if(selected == null){
                return false;
            }

            DataObject worker_data = selected.getWorkerData();
            int begin = worker_data.Begin();
            int end = worker_data.End();
            int current = worker_data.Current();
            int new_end = current + (end-current)/2;

            if(new_end<=0 || (end-current)<=4){
                selected.setSelectable(false);
                for(ClientWorker c : active_workers){
                    if(c.isPaused()){
                        c.Resume(); // selected.Resume();
                    }
                }
                return false;
            }

            String acMatrix; // = trainer.getActiveMatrix();

            double perc = 0;
            double size = 0;


            RealMatrix matrix;
            size = activeMatrix.getRowDimension();

            if(size == r.getRowDimension()){
                acMatrix = "Y";
                matrix = MatrixUtils.createRealMatrix(Y.getData());
            }
            else{
                acMatrix = "X";
                matrix = MatrixUtils.createRealMatrix(X.getData());
            }

            perc = ((new_end-current)*100d)/size;
            perc = (double)Math.round(perc*1000d)/1000d;

            DataObject ob = new DataObject(acMatrix,null,current,new_end,perc);
            selected.splitWorkLoad(new DataMsg("SPLITED_WORK",ob));


            begin = new_end;
            double perc2 = ((end-begin)*100d)/size;
            perc2 = (double)Math.round(perc2*1000d)/1000d;


            if(worker.isDistributed()){
                ob = new DataObject(acMatrix,null,begin,end,perc2);
                worker.distributeExtraWork(new DataMsg("EXTRA_WORK",ob));
            }
            else{
                ob = new DataObject(acMatrix,matrix,begin,end,perc2);
                worker.distributeWork(new DataMsg("DATA",ob));
            }

            active_workers.add(worker);
            selected.Resume();

            pipe.append("\n    - - REDISTRIBUTED : Workload from (Worker "+selected.getID()+") --> to (Worker "+worker.getID()+")");
            pipe.append("\n   (<--) Worker <"+selected.getID()+"> Re-Assigned Workload : "+perc+"%");
            pipe.append("\n   (-->) Worker <"+worker.getID()+"> Re-Assigned Workload : "+perc2+"%");

            for(ClientWorker c : active_workers){
                if(c.isPaused()){
                    c.Resume();
                }
            }
            return true;

        }

        /*Gets Available~Alive workers*/
        ArrayList<ClientWorker> getAvailableWorkers(){

            ArrayList<ClientWorker> removable = new ArrayList<ClientWorker>();
            ArrayList<ClientWorker> av_workers = new ArrayList<ClientWorker>();

            pipe.append("\n-Getting available workers . . .");
            for(ClientWorker worker : Workers){
                if(worker.getWorkerSignal()){
                    worker.reset();
                    av_workers.add(worker);
                }
                else{
                    pipe.append("\n - - Worker "+worker.getID()+" has disconnected. - - \nRemoving Worker "+worker.getID()+" from workers list.");
                    removable.add(worker);
                }
            }


            if(removable.size()>0){
                removeWorker(removable);
            }

            return av_workers;
        }

        /*sorting available workers by cpu > ram usage */
        void sort_av_workers(ArrayList<ClientWorker> list){
            int size = list.size();

            ClientWorker[] temp = new ClientWorker[size];
            for(int i=0; i<size; i++){
                temp[i] = list.get(i);
            }

            int min;

            int p = 0;
            for(int i = p; i<size; i++){
                min = p;
                for(int j = i; j<size; j++){
                    if(Math.abs(temp[j].getCpuUsage() - temp[min].getCpuUsage())>=3d){
                        if(temp[j].getCpuUsage() < temp[min].getCpuUsage()){
                            min = j;
                        }
                    }
                    else{
                        if(temp[j].getRamUsage() < temp[min].getRamUsage()){
                            min = j;
                        }
                    }

                }
                if(min!=p){
                    ClientWorker t = temp[min];
                    temp[min] = temp[p];
                    temp[p] = t;
                }

                p++;

            }


            for(int i = 0; i<size; i++){
                list.set(i,temp[i]);
            }

        }

        /*calculating work percentage for each available worker */
        double[] calc_work_perc_forWorkers(ArrayList<ClientWorker> list , int var){
            pipe.append("\n-Calculating workers weight of work . . .");
            int size = list.size()-1;
            double[] lambas = new double[size];

            for(int i = 1; i<list.size(); i++){
                if(list.get(i-1).getCpuUsage() > list.get(i).getCpuUsage()){
                    lambas[i-1] = list.get(i).getRamUsage()/list.get(i-1).getRamUsage();
                }
                else{
                    lambas[i-1] = list.get(i).getCpuUsage()/list.get(i-1).getCpuUsage();
                }

                lambas[i-1] = (double)Math.round(lambas[i-1]*1000d)/1000d;
                //System.out.println(" l["+i+"] : "+lambas[i-1]);
            }

            double sum = 0;
            double p;
            double[] pi = new double[lambas.length+1];

            for(int i = 0; i<lambas.length+1; i++){
                p = 1;
                for(int j = 0; j<i; j++){
                    p = p*lambas[j];
                }
                pi[i] = 1/p;
                sum+=pi[i];
                //System.out.println("p["+i+"] : "+pi[i]);
            }

            double x = var/sum;
            x = (double)Math.round(x*1000d) / 1000d;

            double[] xi = new double[list.size()];
            for(int i = 0; i<xi.length; i++){
                xi[i] = x*pi[i];
                //System.out.println("xi["+i+"] : "+x*pi[i]);
            }

            return xi;

        }

        /*Requests from Workers Cpu/Ram INFO*/
        void getWorkersPower(ArrayList<ClientWorker>workers){
            pipe.append("\n-Getting workers info . . .");
            for(ClientWorker c : workers){
                c.getWorkerInfo(true,false);
            }
        }

        /*Removes Workers from Clients*/
        void removeWorker(ArrayList<ClientWorker> list){
            for(ClientWorker c : list){
                c.end_connection();
                Workers.remove(c);
            }
            GUI.paintPanel(1);
        }

        void manageNewWorker(ClientWorker worker){

            sendWorkerRawData(worker,r,a,l,f);
            if(worker.getWorkerSignal() && W_PROCESSING){
                worker.setDistributed(false);
                inactive_workers.add(worker);
            }

        }

        /*Returns active training Matrix*/
        public RealMatrix getActiveMatrix(){
            return this.activeMatrix;
        }

        public void SelectionSort(ArrayList<Index> array) {


            double max_v;
            Index max;

            int pivot = 0;
            int pointer = 0;


            for (int i = pivot; i<array.size(); i++) {

                max_v = array.get(i).V();
                max = array.get(i);
                pointer = i;
                for (int j = i; j<array.size(); j++) {
                    if (array.get(j).V() > max_v) {
                        max_v = array.get(j).V();
                        max = array.get(j);
                        pointer = j;
                    }
                }
                Index temp = array.get(pointer);
                array.set(pointer,array.get(pivot));
                array.set(pivot,temp);
                pivot++;
            }
            //return array;

        }

    }
    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

    /*Data Updater Thread*/
    public class DataUpdater implements Runnable{

        private DataObject data;
        private RealMatrix m;

        public DataUpdater(DataObject data,RealMatrix m){
            this.data = data;
            this.m = m;
        }

        public void run(){
            updateMatrixXY(data,m);
        }

    }
    /*- - - - - - - - - - -*/

    /*Synchronized updating method for matrices X,Y*/
    public synchronized void updateMatrixXY(DataObject data,RealMatrix m){

        serverNode.setProcessing(true);


        RealMatrix matrix = data.getMatrix();
        matrix = matrix.transpose();

        int a = data.Begin();
        int b = data.End();

        for(int i = a; i<b; i++){
           m.setRow(i,matrix.getRow(i-a));
        }
        serverNode.setProcessing(false);
        //trainer.updateActiveMatrix(XY);
    }
    /*- - - - - - - - - - -*/


    /*Send workers rawData : r & parameters [alpha,lambda,f]*/
    public void sendWorkerRawData(ClientWorker c,RealMatrix r,double a,double l,int f) {

        DataObject obj = new DataObject("RAW_DATA",r,a,l,f);
        DataMsg data = new DataMsg("DATA",obj);
        c.sendData(data);

    }
    /*------------------------------------------------------------------------------------------------------------------*/


    /*Master's Methods
    Calculating Basic Matrices
     */
    RealMatrix generateRandomMatrix(int rows,int columns,int seed){

        RandomGenerator randomGenerator = new JDKRandomGenerator();
        RealMatrix matrix = MatrixUtils.createRealMatrix(rows,columns);
        randomGenerator.setSeed(seed);
        for(int i=0; i<matrix.getRowDimension(); i++){
            for(int j=0; j<matrix.getColumnDimension(); j++){
                matrix.setEntry(i,j,randomGenerator.nextDouble());
            }
        }

        return matrix;
    }
    RealMatrix calc_Matrix_p(){
        double[][] p_data = new double[r.getRowDimension()][r.getColumnDimension()];
        for(int i=0; i<r.getRowDimension(); i++){
            for(int j=0; j<r.getColumnDimension(); j++){
                if(r.getEntry(i,j)>0){
                    p_data[i][j] = 1;
                }
                else{
                    p_data[i][j] = 0;
                }
            }
        }
        return MatrixUtils.createRealMatrix(p_data);
    }
    RealMatrix calc_Matrix_c(double a){

        double[][] c_data = new double[r.getRowDimension()][r.getColumnDimension()];
        for(int i=0; i<r.getRowDimension(); i++){
            for(int j=0; j<r.getColumnDimension(); j++){
                c_data[i][j] = 1 + a*r.getEntry(i,j);
            }
        }
        return MatrixUtils.createRealMatrix(c_data);
    }
    /* */


    /* - - - - - - - - SERVER - - - - - - - - */
    /*Opens server on giver port,
    then starts the listening thread.
     */
    public void openServer(int server_socket_id){

        this.port = server_socket_id;
        try{
            server_socket = new ServerSocket(port);
            server_init = true;
            serverNode = GUI.getServerNode();
            serverNode.setProcessing();
        }
        catch(Exception e){
            e.printStackTrace();
            message = "-Exception occured while trying to open ServerSocket on port : "+String.valueOf(port);
            server_init = false;
            return;
        }

        new serverListeningThread();

    }

    /*Thread keeping the server on Listening Mode */
    class serverListeningThread implements Runnable{

        int count = 0;

        public serverListeningThread(){
            (new Thread(this)).start();
        }

        public void run(){

            System.out.println("-Server is open. . .");
            while(true){
                try{

                    ClientWorker cli = new ClientWorker(server_socket.accept(),pipe,serverNode);
                    System.out.println("-Client Connected to server!");
                    serverNode.setProcessing();
                    Workers.add(cli);
                    GUI.paintPanel(1);


                    Workers.get(Workers.size()-1).start();
                    Workers.get(Workers.size()-1).getWorkerInfo(true,true);


                    if(trainer.RUNNING){
                        trainer.manageNewWorker(cli);
                    }

                }
                catch(Exception e){
                    e.printStackTrace();
                    message = "-Exception occured while trying to accept a Client !";

                }

            }
        }
    }
    /* */
    /* - - - - - - - - - - - - - - - - - - - */



    /*- - - - - - - - - - - - - - GETTERS - - - - - - - - - - - - - - - - -*/
    //Returns ServerSocket
    public int getServerPort(){
        return this.port;
    }

    //Returns current Message
    public String getMessage(){
        String msg = message;
        message = null;
        return msg;
    }

    //Returns Clients/Workers number
    public int getClientsNumber(){
        return Workers.size();
    }

    //Returns an ArrayList of the current Clients
    public ArrayList<ClientWorker> getClients(){
        return Workers;
    }

    //Returns SizeX of matrix : r
    public int getRsizeX(){
        return r.getRowDimension();
    }

    //Returns SizeY of matrix : r
    public int getRsizeY(){
        return r.getColumnDimension();
    }

    public RealMatrix getData() {
        if(data_init){
            return MatrixUtils.createRealMatrix(r.getData());
        }
        else return null;
    }

    //Returns 'true' if server is open
    public boolean isServer(){
        return server_init;
    }

    //Returns 'true' if Data has been initialized succesfully
    public boolean isData(){
        return data_init;
    }
    /* */

    /*setters for Trainer*/
    public void setDistri_treshold(double t){
        trainer.d_treshold = t;
        pipe.append("\n\n - - Redistribution treshold set to : "+t+"\n");
    }
    public void setDistri_priority(int a){
        if(a == 0){
            pipe.append("\n\n - - Redistribution priority set to : Workload > CPU usage\n");
        }
        else if(a ==1 ){
            pipe.append("\n\n - - Redistribution priority set to : CPU usage > Workload\n");
        }

    }
    public void setDistri(int a){
        if(a>0){
            trainer.redistribute = true;
        }
        else{
            trainer.redistribute = false;
        }
        pipe.append("\n\n - - Redistribution set to : "+trainer.redistribute);
    }
    public void setSynch(int a){
        if(a>0){
            trainer.synch = true;
        }
        else{
            trainer.synch = false;
        }
    }
    public void setIterations(int it){
        trainer.iterations = it;
        pipe.append("\n\n - - Max Iterations set to : "+it);
    }
    public void setError(double e){
        trainer.error = e;
        pipe.append("\n\n - - Max Error set to : "+e);
    }
    public void setRecSize(int a){
        trainer.rec_list_size = a;
    }
    public void Reccomend(int user){
        trainer.ReccomendBestLocalPois(user);
    }
    /* - - - - */
    public void StopTraining(){
        trainer.RUNNING = false;
        GUI.activateStart();
    }

    //Ends connections with Clients , then closes ServerSocket
    public void exit(){

        for(ClientWorker c:Workers){
            c.end_connection();
        }
        try {
            server_socket.close();
        } catch (Exception e) {
            e.printStackTrace();

        }

    }


}
//
