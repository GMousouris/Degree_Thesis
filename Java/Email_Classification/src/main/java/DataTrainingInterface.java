
public class DataTrainingInterface implements ErrorHandler
{



    private DataTrainer TRAINER;

    private double train_data_factor = 0.9; // [x%]size of data -> training
    private double test_data_factor = 1.0 - train_data_factor; // [y&]size of data -> testing

    //Training-Testing Dataset
    static final int ENRON = 0;
    static final int LINGSPAM_BARE = 1;
    static final int LINGSPAM_LEMM = 2;
    static final int LINGSPAM_LEMM_STOP = 3;
    static final int LINGSPAM_STOP = 4;
    static final int LINGSPAM_TEST = 5;

    private int INPUT_DATA;
    private String input_data_path = null;

    private boolean DATA_SET = false;
    private boolean TRAINER_SET = false;
    private boolean TRAINING_DONE = false;


    //Constructor
    public DataTrainingInterface()
    {

    }

    //set the input data for training-testing
    public boolean InitData(int data)
    {
        switch(data)
        {
            case ENRON : input_data_path = "data/enron1/";
            INPUT_DATA = ENRON;
            break;

            case LINGSPAM_BARE : input_data_path =  "data/lingspam_public/bare/";
            INPUT_DATA = LINGSPAM_BARE;
            break;

            case LINGSPAM_LEMM : input_data_path = "data/lingspam_public/lemm/";
            INPUT_DATA = LINGSPAM_LEMM;
            break;

            case LINGSPAM_LEMM_STOP : input_data_path = "data/lingspam_public/lemm_stop/";
            INPUT_DATA = LINGSPAM_LEMM_STOP;
            break;

            case LINGSPAM_STOP : input_data_path = "data/lingspam_public/stop/";
            INPUT_DATA = LINGSPAM_STOP;
            break;

            case LINGSPAM_TEST : input_data_path = "data/lingspam_public/MyPanel/";
            break;

            default : printError("DataTrainingInterface" , "setInputData(int data)" , "Invalid input_data");
            return false;
        }

        DATA_SET = true;
        return true;
    }

    //set the trainer
    public boolean InitDataTrainer(Object trainer)
    {

        int trainerID = ((DataTrainer)trainer).getTrainerID();
        switch (trainerID)
        {
            case DataTrainer.NAIVE_BAYES : TRAINER = new NaiveBayes((NaiveBayes)trainer);
            break;

            case DataTrainer.ID_3 : TRAINER = new ID3((ID3)trainer);
            break;


            //case DataTrainer.AdaBoost : ...

            //case DataTrainer.LOGISTIC_REGGRESION : ...

            default: printError("DataTrainingInterface" , "setTrainingMethod(object,int)","Invalid parameters!");
            return false;
        }

        TRAINER_SET = true;
        return true;
    }




    public boolean InitTrainingProcess()
    {
        if(!TRAINER_SET && !DATA_SET)
        {
            printError("DataTrainingInterface","InitializeDataTraining()","Trainer or Data not set!");
            return false;
        }

        TRAINER.setInputData(input_data_path , InputDataToString(INPUT_DATA));
        TRAINING_DONE = TRAINER.InitTrainingProcess();
        return TRAINING_DONE;
    }


    public boolean InitTestingProcess()
    {
        if(!TRAINING_DONE)
        {
            printError("DataTrainingInterface", "InitTestingProcess()", "input Data not trained!");
            return false;
        }

        return TRAINER.InitTestingProcess();
    }


    private String InputDataToString(int input_data)
    {

        switch (input_data)
        {
            case ENRON : return "ENRON";
            case LINGSPAM_BARE : return "LINGSPAM_BARE";
            case LINGSPAM_LEMM : return "LINGSPAM_LEMM";
            case LINGSPAM_LEMM_STOP : return "LINGSPAM_LEMM_STOP";
            case LINGSPAM_STOP : return "LINGSPAM_STOP";
        }

        return "null";
    }

    @Override
    public void printError(String className, String functionName, String err)
    {
        System.out.println(" -Error : ("+className+"::"+functionName+" : "+err);
    }
}
