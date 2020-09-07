
import java.io.Serializable;

public class DataMsg implements Serializable {


    private String msg;
    private String type;
    private WorkerInfo info;
    private DataObject data;
    private int value;
    private int id;

    public DataMsg(String type , Object data){

        this.type = type;

        if(type.equals("INFO")){
            info = (WorkerInfo) data;
        }
        else if(type.equals("DATA")){
            this.data = (DataObject) data;
        }
        else if(type.equals("MSG")){
            this.msg = String.valueOf(data);
            //this.data = null;
        }
        else if(type.equals("STATE")){
            this.msg = String.valueOf(data);
        }
        else if(type.equals("SPLITED_WORK") || type.equals("EXTRA_WORK")){
            this.data = (DataObject)data;
        }

    }

    public DataMsg(String type , Object data,int id){

        this.type = type;
        this.id = id;

        if(type.equals("INFO")){
            info = (WorkerInfo) data;
        }
        else if(type.equals("DATA")){
            this.data = (DataObject) data;
        }
        else if(type.equals("MSG") || type.equals("STATE")){
            this.msg = String.valueOf(data);

        }
        else if(type.equals("SPLITED_WORK") || type.equals("EXTRA_WORK")){
            this.data = (DataObject)data;
        }

    }



    public DataMsg(String type, DataObject data, WorkerInfo info){

        this.type =type;
        this.data = data;
        this.info = info;
    }


    public DataObject getData(){
        return this.data;
    }

    public WorkerInfo getInfo(){
        return this.info;
    }

    public String getType(){
        return this.type;
    }

    public String getMsg(){
        return this.msg;
    }

    public int getValue(){
        return this.value;
    }

    public int getID(){
        return  this.id;
    }



}
