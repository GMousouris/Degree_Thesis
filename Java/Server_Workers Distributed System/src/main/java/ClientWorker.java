
import javax.swing.*;
import java.awt.*;
import java.io.*;
import java.net.Socket;
import java.net.SocketException;

public class ClientWorker extends Thread {




    private static int sum = 0;
    private int id;
    private Socket socket;

    boolean Alive = true;
    boolean info = false;
    boolean OUT = false;
    boolean READING = false;
    boolean IN = true;
    boolean done = false;
    boolean resumed = true;
    boolean paused = false;

    private JTextArea pipe;

    private ObjectInputStream in= null;
    private ObjectOutputStream out= null;

    private WorkerInfo worker_info;
    private DataMsg msgIN;
    private DataObject workerData = null;
    private double workload = 0.0;
    private double workDone = 0.0;
    private boolean alive = false;
    private boolean WORK_PROGRESS = false;

    private Line line;
    private MyNode node;
    private MyNode serverNode;

    private int extra_work = 0;
    private int splitted_work = 0;

    private boolean selectable = true;
    private boolean distributed = false;

    private int begin = -1;
    private int end = -1;


    public ClientWorker(Socket socket,JTextArea pipe,MyNode serverNode){

        line = new Line();
        alive = true;
        this.pipe = pipe;
        this.id = sum++;
        this.serverNode = serverNode;

        this.socket = socket;
        try {
            this.socket.setSoTimeout(2000);
        } catch (SocketException e) {
            e.printStackTrace();
        }


        try {

            out = new ObjectOutputStream(this.socket.getOutputStream());
            in = new ObjectInputStream(this.socket.getInputStream());

        }catch(IOException e){}

        pipe.append("\n\nWorker connected to Server.");
        pipe.append("\nAwaiting for Worker's response . . .");

        sendData(new DataMsg("ID",null,getID()));


    }


    public void run(){

        Object obj;

        while(alive){

            obj = null;

            try{
                Thread.sleep(30);
            }
            catch (Exception e){}

            if(IN){
                try{
                    obj = in.readObject();
                }
                catch(SocketException e){continue;}
                catch(IOException e){continue;}
                catch(Exception e){continue;}

                if(obj!=null){

                    msgIN = (DataMsg)obj;
                    if(msgIN.getType().equals("DATA")){
                        DataObject data = msgIN.getData();
                        if(data.getType().equals("RESULT")){

                            done = true;
                            selectable = true;
                            node.setProcessing(false);
                            line.done = true;
                            this.workerData = data;
                            if(workerData==null){
                                pipe.append("\n\n-Worker results : CORRUPTED.");
                            }
                            pipe.append("\n\n-Got Results from Worker : "+getID()+"\n-Worker : "+getID()+" : DONE.  ");
                        }
                    }
                    else if(msgIN.getType().equals("INFO")){
                        info = true;
                    }
                    else if(msgIN.getType().equals("WORK_PROGRESS")){
                        WORK_PROGRESS = true;
                    }
                    else if(msgIN.getType().equals("HEARTBEAT")){
                        alive = true;
                    }

                }
            }
        }
        //

    }
    /*-----------------------------------------*/

    public boolean getWorkerSignal(){

        line.heartBeat();
        line.setOUT();
        try{
            out.writeObject(new DataMsg("HEARTBEAT",null));
            out.flush();
        }
        catch(Exception e){
            alive = false;
            return false;
        }

        int counter = 0;

        line.setIN(serverNode);
        while(true){
            if(counter>100){line.setDEF(); return false;}
            try{
                Thread.sleep(10);
                counter++;
            }
            catch(Exception e){}
            if(msgIN.getType().equals("HEARTBEAT")){
                line.setDEF();
                return true;
            }

        }


    }

    public void getWorkerInfo(boolean request,boolean print){
        info = false;

        line.setOUT();
        if(request){
            try {
                out.writeObject(new DataMsg("INFO",null));
            } catch (IOException e) { }
        }

        line.setIN(serverNode);
        while(!info) {
            try {
                Thread.sleep(10);
            } catch (Exception e) { }
        }
        if(msgIN.getType().equals("INFO")){

            this.worker_info = msgIN.getInfo();
            if(print){
                pipe.append("\n\n          _Worker ID : <"+this.id+">_");
                pipe.append("\n    Cpu Usage : "+(worker_info.cpuUsage)+"%");
                pipe.append("\n    Ram usage : "+worker_info.ramUsage+"%");
                pipe.append("\n    Worker state : "+worker_info.status+"\n");
            }
        }

        line.setDEF();
    }

    public void getWorkProgress(){


        sendData(new DataMsg("WORK_PROGRESS",null));

        line.setIN(serverNode);
        while(!WORK_PROGRESS && !done){try{
        Thread.sleep(10);}
        catch(Exception e){}

        }

        if(msgIN.getType().equals("WORK_PROGRESS")){
            this.workerData = msgIN.getData();
            this.worker_info = msgIN.getInfo();
            this.workDone = msgIN.getData().getWorkload();
        }
        WORK_PROGRESS = false;
        line.setDEF();

    }

    public void distributeExtraWork(DataMsg data){

        getIters(data.getData());
        selectable = true;
        extra_work++;
        done = false;
        node.setProcessing(true);
        line.done = false;
        sendData(data);
    }

    public void distributeWork(DataMsg data){

        getIters(data.getData());
        selectable = true;
        distributed = true;
        done = false;
        node.setProcessing(true);
        line.done = false;
        sendData(data);
    }

    public void splitWorkLoad(DataMsg data){

        getIters(data.getData());
        splitted_work++;
        node.setProcessing(true);
        line.done = false;
        sendData(data);
    }

    public void Pause(){
        paused = true;
        node.setProcessing(false);
        sendData(new DataMsg("PAUSE",null));
    }

    public void Resume(){

        paused = false;
        sendData(new DataMsg("RESUME",null));
        if(!done){
            node.setProcessing(true);
        }
        else{
            node.setProcessing(false);
        }
    }
    // - - - - - - - - - - - - - - - - - - - - //

    public void sendData(DataMsg data){

        line.setOUT();
        int counter = 0;

        while(counter<2){
            try{
                out.writeObject(data);
                out.flush();
                break;
            }
            catch(Exception e){
                pipe.append("\n - - Failed to send data to Worker "+getID());
                counter++;
                continue;
            }
        }

        line.setDEF();

    }
    /*------------------------------------------*/

    public void setTimeOut(int x){
        try{
            this.socket.setSoTimeout(x);
        }
        catch(SocketException e){
            e.printStackTrace();
        }
    }

    public void end_connection(){

        line.stroke = true;
        line.color = Color.red;
        try{
            in.reset(); out.reset();
            in.close(); out.close();
            //this.socket.close();
        }
        catch (Exception e){}
    }

    public boolean Alive(){
        return this.Alive;
    }

    public int getID(){
        return this.id;
    }

    public double getRamUsage(){
        return this.worker_info.ramUsage;
    }

    public double getCpuUsage(){
        return this.worker_info.cpuUsage;
    }

    public void Listen(boolean b) {

        if(b){
            IN = true;
        }
        else{
            IN = false;
        }

    }


    public boolean isDone(){
        /*
        msgIN = readFromWorker();
        if(msgIN==null)return false;
        if(msgIN.getType().equals("DATA")){
            if(msgIN.getData().getType().equals("RESULT")) state = true;

        }
        */
        return this.done;
    }

    public DataObject getWorkerData() {
        return this.workerData;
    }

    public void setWorkload(double w){
        this.workload = w;
    }

    public double getWorkload(){
        return this.workload;
    }

    public double getWorkDone(){
        return this.workDone;
    }

    public void setLine(Line line){
        this.line = line;
    }

    public void setNode(MyNode node){
        this.node = node;
    }

    public boolean isPaused(){
        return paused;
    }

    public int getExtraWork(){
        return extra_work;
    }

    public int getSplittedWork(){
        return splitted_work;
    }

    public void setSelectable(boolean s){
        this.selectable = s;
    }
    public boolean isSelectable(){
        return this.selectable;
    }

    public void setDistributed(boolean s){
        distributed = s;
    }

    public boolean isDistributed(){
        return this.distributed;
    }

    public void getIters(DataObject obj){
        begin = obj.Begin();
        end = obj.End();
    }

    public void reset(){
        selectable = true;
    }
}
