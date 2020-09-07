

import java.io.File;
import java.io.PrintWriter;
import java.util.*;

public class DataTrainer implements ErrorHandler

{
    static final String[] stopwords = {"a", "about", "above", "after", "again", "against", "all", "am", "an", "and", "any", "are", "as", "at", "be", "because", "been", "before", "being", "below", "between", "both", "but", "by", "could", "did", "do", "does", "doing", "down", "during", "each", "few", "for", "from", "further", "had", "has", "have", "having", "he", "he'd", "he'll", "he's", "her", "here", "here's", "hers", "herself", "him", "himself", "his", "how", "how's", "i", "i'd", "i'll", "i'm", "i've", "if", "in", "into", "is", "it", "it's", "its", "itself", "let's", "me", "more", "most", "my", "myself", "nor", "of", "on", "once", "only", "or", "other", "ought", "our", "ours", "ourselves", "out", "over", "own", "same", "she", "she'd", "she'll", "she's", "should", "so", "some", "such", "than", "that", "that's", "the", "their", "theirs", "them", "themselves", "then", "there", "there's", "these", "they", "they'd", "they'll", "they're", "they've", "this", "those", "through", "to", "too", "under", "until", "up", "very", "was", "we", "we'd", "we'll", "we're", "we've", "were", "what", "what's", "when", "when's", "where", "where's", "which", "while", "who", "who's", "whom", "why", "why's", "with", "would", "you", "you'd", "you'll", "you're", "you've", "your", "yours", "yourself", "yourselves"};

    static final int NAIVE_BAYES = 0;
    static final int ID_3 = 1;
    static final int AdaBOOST = 2;
    static final int LOGISTIC_REGRESSION = 3;

    static final int C_ACCEPT = 0;
    static final int C_REJECT = 1;

    protected int TRAINER_ID;
    protected int VERSION;

    protected boolean TRAINING = false;
    protected boolean TESTING = false;

    // input data path used for training and testing
    protected String INPUT_DATA;
    protected String INPUT_DATA_ID;
    protected double train_data_factor = 0.9; // part of data used for training
    protected double test_data_factor = 0.1; // part of data used for testing

    // spam , ham and total email counts
    protected int HamEmailCount = 0;
    protected int SpamEmailCount = 0;
    protected int totalEmailCount = 0;

    protected int testEmailHamCount = 0;
    protected int testEmailSpamCount = 0;
    protected int testEmailTotalCount = 0;
    //

    // contains <Word , Word_properties> of the training data
    // Key represents the word , while VariableProperties represent each word's frequencies , probabilities , etc . . .
    protected HashMap<String , VariableProperties> variables = new HashMap<>();
    // contains each email's data ( from training data )
    protected HashMap<Integer , HashMap<String, VariableProperties>> Emails = new HashMap<>();

    // stores the testing results  , for each prediction X we have real value Y thus <X,Y>
    protected List<Pair> predictions = new ArrayList<Pair>();
    protected int correct_predictions = 0;
    protected int false_predictions = 0;

    protected float accuracy;
    protected float error;

    protected float FP = 0;
    protected float FN = 0;
    protected float TP = 0;
    protected float TN = 0;


    protected float[] precision;
    protected float[] recall;
    protected float[] f1;

    //
    public DataTrainer()
    {

    }

    //
    public DataTrainer(int TRAINER_ID)
    {
        this.TRAINER_ID = TRAINER_ID;

        int classes;

        switch (TRAINER_ID)
        {
            case NAIVE_BAYES : classes = 2;
            break;

            case ID_3 : classes = 2;
            break;

            case LOGISTIC_REGRESSION : classes = 2;
            break;

            default: classes = 2;
            break;
        }

        precision = new float[classes];
        recall = new float[classes];
        f1 = new float[classes];
    }

    //
    public boolean InitTrainingProcess()
    {
        //processRawData();
        return processRawData(INPUT_DATA , true);
    }

    //
    public boolean InitTestingProcess()
    {
        /*
           read each testing data part
           process each email
           predict for each email if email class = spam or ham
           keep track of each prediction and each correct answer
           calculate accuracy , error , f1
           do multiple tests with various learning curves ( different training_data_factor )
           and create a graph with the learning curve of the algorithm based on trained data

           P(C|X) = P(X|C)*P(C)/P(X)  ****if there is a word that is not on the vocabulary we give it the def P : 0.5
           if( P(C=1|X) > P(C=0|X) ) : email = spam
        */

        boolean test = processRawData(INPUT_DATA , false);
        extractInformationBasedOnTestedData();
        exportDataTrainerResultsToFile();
        return test;
    }

    //
    protected void extractInformationBasedOnTestedData()
    {

        //System.out.println("\nTP : "+TP+"\nFP : "+FP+"\nTN : "+TN+" \nFN : "+FN);
        precision[NaiveBayes.C_REJECT] = TP/(TP+FP); // spam precision
        precision[NaiveBayes.C_ACCEPT] = TN/(TN+FN); // ham precision

        recall[NaiveBayes.C_REJECT] = TP/(TP+FN); // spam recall
        recall[NaiveBayes.C_ACCEPT] = TN/(TN+FP); // ham recall

        accuracy = (TP+TN)/(TP+TN+FP+FN);

        error = 1.0f - accuracy;

        f1[NaiveBayes.C_REJECT] = 2 * (precision[NaiveBayes.C_REJECT] * recall[NaiveBayes.C_REJECT])
                /(precision[NaiveBayes.C_REJECT] + recall[NaiveBayes.C_REJECT]);
        f1[NaiveBayes.C_ACCEPT] = 2 * (precision[NaiveBayes.C_ACCEPT] * recall[NaiveBayes.C_ACCEPT])
                /(precision[NaiveBayes.C_ACCEPT] + recall[NaiveBayes.C_ACCEPT]);


    }

    //
    public void exportDataTrainerResultsToFile()
    {

        String path = "Results/";
        File results = new  File(path);
        int docs_size = results.listFiles().length;

        File exportResults = new File(path+getClass().getName()+"_"+docs_size+".txt");
        try
        {
            PrintWriter pw = new PrintWriter(exportResults, "UTF-8");

            // basic info
            pw.println("DataTrainer : "+getClass().getName());
            pw.println("Version : "+getVersionToString(VERSION));
            pw.println("Parameter_k : "+getParameter_k());

            if(TRAINER_ID == DataTrainer.NAIVE_BAYES)
            {
                pw.println("Parameter_m : "+getParameter_m());
                pw.println("IG_feature_selection : "+getIG_FEATURE_SELECTION());

            }
            else if(TRAINER_ID == DataTrainer.ID_3)
            {
                pw.println("Parameter_Depth : "+getParameter_Depth());
            }


            pw.println("Input_data : "+INPUT_DATA_ID);
            pw.println("Training_data_factor : "+train_data_factor);
            pw.println("Testing_data_factor : "+(1.0f-train_data_factor));

            // data info
            pw.println("Training_total_email_count : "+totalEmailCount);
            pw.println("Training_spam_email_count : "+SpamEmailCount);
            pw.println("Training_ham_email_count : "+HamEmailCount);

            pw.println("Testing_total_email_count : "+testEmailTotalCount);
            pw.println("Testing_spam_email_count : "+testEmailSpamCount);
            pw.println("Testing_ham_email_count : "+testEmailHamCount);

            // testing results
            pw.println("Correct_predictions : "+correct_predictions);
            pw.println("False_predictions : "+false_predictions);
            pw.println("Precision[spam] : "+precision[NaiveBayes.C_REJECT]);
            pw.println("Precision[ham] : "+precision[NaiveBayes.C_ACCEPT]);
            pw.println("Recall[spam] : "+recall[NaiveBayes.C_REJECT]);
            pw.println("Recall[ham] : "+recall[NaiveBayes.C_ACCEPT]);
            pw.println("Accuracy : "+accuracy);
            pw.println("Error : "+error);
            pw.println("F1[spam] : "+f1[NaiveBayes.C_REJECT]);
            pw.println("F1[ham] : "+f1[NaiveBayes.C_ACCEPT]);

            pw.close();
        }
        catch(Exception e) {e.printStackTrace(); }



    }

    //
    protected boolean processRawData(String INPUT_DATA , boolean TRAINING)
    {
        this.TRAINING = TRAINING;
        this.TESTING = !TRAINING;

        String data_path = INPUT_DATA;

        File data_folder = new File(data_path); // data_path : folder where the data_parts are [1,10]
        if(!data_folder.exists())
        {
            printError("DataTrainer" , "processData","Requested data_folder doesn't exist!");
            return false;
        }

        /* CASE : INPUT_DATA = lingspam_public */
        File[] data_parts = data_folder.listFiles();
        // Accessing each data part (containing the training data)

        // splitting and picking which files to process based on Training or Testing
        int begin;
        int end;
        if(TRAINING)
        {
            begin = 0;
            end  = (int)(data_parts.length * train_data_factor);
        }
        else
        {
            begin = data_parts.length-1;
            end = data_parts.length;
        }

        // iterate between data files from : begin to : end
        for (int i = begin; i < end; i++) // Reading each data part
        {
            File data_part_i = data_parts[i]; // data part[i] -> { N number of data files }

            /*
               Reading each file in data part[i] { datafile1 , datafile2 , ... , datafileN }
               Each file is an e-mail (.txt type) which can be either legit or spam
               Each e-mail is recognized as spam or legit according to its filename
               ( example : msg_1.txt -> legit , spam_023.txt -> spam , etc . . . )
               cases vary based on which data are being used as input for training-testing { enron , lingspam_public , pu_corpora_public }
            */

            File[] data_part_i_files = data_part_i.listFiles();

            for ( int j = 0; j<data_part_i_files.length; j++) // Reading each email
            {

                /*
                   read each data file in data_part[i]
                   get its variables and update them
                   and their frequencies in the hashMap
                   based on email type { spam , legit }
                */
                    processEmail(data_part_i_files[j]);

            }//

        }//

        return true;
    }

    //
    protected void processEmail(File email)
    {

    }


    //
    protected void predictEmailsClass(HashMap<String , VariableProperties> email_variables , int email_class)
    {

    }

    //
    public void setVersion(int version)
    {
        if(TRAINER_ID == NAIVE_BAYES || TRAINER_ID == ID_3)
        {

            if(version<0 || version>1)
            {
                printError(getVersionToString(TRAINER_ID),"setNB_Version(int)","Invalid parameter!");
                return;
            }

        }


        VERSION = version;
    }

    //
    public void setInputData(String input_data , String input_data_id)
    {
        INPUT_DATA = input_data;
        INPUT_DATA_ID = input_data_id;
    }

    //
    public void setTrainingDataFactor(double train_factor)
    {
        train_data_factor = train_factor;
        test_data_factor = 1 - train_data_factor;
    }

    //
    public int getTrainerID()
    {
        return TRAINER_ID;
    }

    //
    public int getVersion()
    {
        return VERSION;
    }

    //
    public String getVersionToString(int version)
    {

        if(TRAINER_ID == NAIVE_BAYES || TRAINER_ID == ID_3)
        {
            switch (version)
            {
                case 1 : return "BERNOULLI";
                case 0 : return "MULTINOMIAL";

            }

        }

        return "null";
    }

    //
    public int getParameter_k(){return -1;}
    public int getParameter_m(){return -1;}
    public int getParameter_Depth(){return-1;}
    public boolean getIG_FEATURE_SELECTION(){return false;}

    public void printError(String className, String functionName, String err) {
        System.out.println(" -Error : ("+className+"::"+functionName+" : "+err);
    }

    // ...
    protected static  <K, V extends Comparable<? super V>> Map<K, V> sortByValue(Map<K, V> map) {
        List<Map.Entry<K, V>> list = new ArrayList<>(map.entrySet());
        list.sort(Collections.reverseOrder(Map.Entry.comparingByValue()));

        Map<K, V> result = new LinkedHashMap<>();
        for (Map.Entry<K, V> entry : list) {
            result.put(entry.getKey(), entry.getValue());
        }

        return result;
    }


    protected static float log2(float x)
    {
        return (float)(Math.log(x)/Math.log(2));
    }


    protected static float max(float a, float b)
    {
        return (a>b)? a:b;
    }
}

