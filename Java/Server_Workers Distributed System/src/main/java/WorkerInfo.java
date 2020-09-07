import java.io.Serializable;

public class WorkerInfo implements Serializable {

    double cpuUsage;
    double ramUsage;
    double ramTotal;
    String status = null;
    boolean done = false;
    double perc = 0;

    public WorkerInfo(double c , double r){
        this.cpuUsage = c;
        this.ramUsage = r;

    }

    public WorkerInfo(double c , double r,String status){
        this.cpuUsage = c;
        this.ramUsage = r;
        this.status = status;
    }


}
