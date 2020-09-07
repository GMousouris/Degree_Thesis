

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;

import java.util.*;


public class NaiveBayes extends DataTrainer
{

    // 2 Classes of emails ( Ham - Spam or ACCEPT - REJECT )
    static final int C_ACCEPT = 0;
    static final int C_REJECT = 1;

    // NaiveBayes versions
    static final int MULTINOMIAL = 0;
    static final int BERNOULLI = 1;

    private boolean IG_FEATURE_SELECTION = true;

    // Parameter k : minimum total frequency of word X in order to make it as an attribute
    private int k = 5;
    // Parameter m : maximum number of attributes with max IG value
    private int m = 3000;


    // Spam and Ham email counts from train data
    private int totalVariableFreq_0 = 0;
    private int totalVariableFreq_1 = 0;
    private int totalVariableCount = 0;

    private float H_C = 0.0f; // H(C)
    private float P_C_0 = 0.0f; // P(C=0) .. P(ACCEPT) (ham)
    private float P_C_1 = 0.0f; // P(C=1) .. P(REJECT) (spam)


    //Default Constructor
    public NaiveBayes()
    {
        super(NAIVE_BAYES);
    }

    //
    public NaiveBayes(int version , int k)
    {
        super(NAIVE_BAYES);
        setVersion(version);
        this.k = k;
    }

    //
    public NaiveBayes(int version , int k , int m)
    {
        super(NAIVE_BAYES);
        setVersion(version);
        this.k = k;
        this.m = m;
    }

    //Copy constructor
    public NaiveBayes(NaiveBayes source)
    {
        super(NAIVE_BAYES);
        setVersion(source.VERSION);
        k = source.k;
        m = source.m;
        IG_FEATURE_SELECTION = source.IG_FEATURE_SELECTION;
        setTrainingDataFactor(source.train_data_factor);
    }


    public void setParameter_k(int k)
    {
        this.k = k;
    }
    public void setParameter_m(int m){ this.m = m;}
    public void setIG_FEATURE_SELECTION(boolean state) { IG_FEATURE_SELECTION = state;}

    public int getParameter_k(){return k;}
    public int getParameter_m(){return m;}
    public boolean getIG_FEATURE_SELECTION(){return IG_FEATURE_SELECTION;}


    // Initialize Training process on training data
    public boolean InitTrainingProcess()
    {
        System.out.println("-Initializing Training process.");

        // calling the Parent.function which processRawData()..
        System.out.println("-Processing raw data : "+INPUT_DATA+"\n" +
                "  o processing each email's data..");
        boolean process_raw_data = super.InitTrainingProcess();

        // Discard variables with freq < k
        System.out.println("-Discarding variables with frequency less than : "+k);
        variables = new HashMap<>(discardVariablesBasedOn_k(variables));

        // Compute probabilities based on trained data
        System.out.println("-Computing probabilities : P(Xi|C) , P(C) , P(Xi) , etc . . .");
        computeProbabilitiesBasedOnTrainedData(variables);


        if(IG_FEATURE_SELECTION)
        {
            System.out.println("-Initializing feature selection based on Information Gain.");
            // Compute information gain based on current variables Xi
            HashMap<String,Float> IG_map = new LinkedHashMap<>(computeInformationGainBasedOnTrainedData(variables));
            System.out.println("  o Picking the "+m+" variables with maximum IG value.");
            // Keep m attributes with the maximum IG value
            variables = new HashMap<>(getMaximum_m_variables_basedOnIG(variables , IG_map));
        }


        System.out.println("-Training process finished.");
        return process_raw_data;
    }


    //
    public boolean InitTestingProcess()
    {
        System.out.println("-Initializing Testing process..");
        boolean test = super.InitTestingProcess();
        System.out.println("-Testing process finished!");
        return test;
    }


    // Process each email's variables (words)
    protected void processEmail(File email )
    {

        totalEmailCount++;

        // Recognize current email's class { ACCEPT , REJECT }
        // Based on file's name
        int email_class;

        if(email.getName().contains("spmsg")) // case : spam
        {
            email_class = C_REJECT;
            SpamEmailCount++;
        }
        else
        {
            email_class = C_ACCEPT; // case : ham
            HamEmailCount++;
        }



        //Initiate temp HashMap where Xi (words) of current email are going to get stored
        HashMap<String , VariableProperties> email_variables = new HashMap<>();

        //Read each line of email and process it accordingly
        try (BufferedReader br = new BufferedReader(new FileReader(email))) {

            StringTokenizer Tokenizer;
            boolean subject = false;
            String line;

            while ((line = br.readLine()) != null) {

                Tokenizer = new StringTokenizer(line); // tokenize each line
                int counter = 0;

                while(Tokenizer.hasMoreTokens())
                {
                    // removing punctuation & numbers
                    //String token = Tokenizer.nextToken().replaceAll("\\p{Punct}", "").replaceAll("[0-9]","").replaceAll("\\s+","").toLowerCase();
                    String token = Tokenizer.nextToken().replaceAll("[0-9]","").replaceAll("\\s+","").toLowerCase();
                    // process Token . . .

                    if(token.equals("") || token.equals(" ") || token.equals(null))
                    {
                        continue;
                    }
                    // case : word is already stored in email's HashMap
                    if(email_variables.containsKey(token))
                        email_variables.get(token).incFrequency(email_class , 1); // Increase its frequency by 1

                    // case : word isn't in email's HashMap
                    // store new word in email's HashMap
                    else
                    {
                        VariableProperties properties = new VariableProperties();
                        properties.incFrequency(email_class , 1);
                        email_variables.put(token , properties);
                    }


                }

            }
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        if(TRAINING) // case : TRAINING -> update main variables HashMap with processed email's variables
            updateVariablesHashMap(email_variables , email_class);
        else if(TESTING) // case : TESTING -> predict email's class based on trained data
            predictEmailsClass(email_variables , email_class);


    }


    //
    protected void predictEmailsClass(HashMap<String , VariableProperties> email_variables , int email_class)
    {
        testEmailTotalCount++;
        if(email_class == C_REJECT)
            testEmailSpamCount++;
        else
            testEmailHamCount++;

        /* case NB_VERSION : BERNOULLI */

        float Πi_c0 = 1.0f; // Πi p(wi|C=0) ..
        float Πi_c1 = 1.0f; // Πi p(wi|C=1) ..

        for(String key : variables.keySet())
        {

            float P_wi_0;
            float P_wi_1;

            if(email_variables.containsKey(key))
            {
                P_wi_1 = variables.get(key).getP_wi_1();
                P_wi_0 = variables.get(key).getP_wi_0();
            }

            else
            {
                P_wi_1 = 1.0f - variables.get(key).getP_wi_1();
                P_wi_0 = 1.0f - variables.get(key).getP_wi_0();
            }

            Πi_c0 *= P_wi_0;
            Πi_c1 *= P_wi_1;
        }

        float P_Spam_X = (P_C_1 * Πi_c1) / ((P_C_0 * Πi_c0)+(P_C_1 * Πi_c1));
        float P_Ham_X = (P_C_0 * Πi_c0) / ((P_C_0 * Πi_c0)+(P_C_1 * Πi_c1));
        /* */


        /* case NB_VERSION : MULTINOMIAL */

        /* */

        // . . .
        // predict email's class and gather info based on prediction and email's true class
        int predicted_class;
        if(P_Spam_X > P_Ham_X)
            predicted_class = C_REJECT;
        else
            predicted_class = C_ACCEPT;

        // correct prediction
        if( predicted_class == email_class )
        {
            correct_predictions++;
            if ( email_class == C_REJECT ) // predict : spam , email = spam -> true positive
                TP++;
            else                           // predict : ham , email = ham -> true negative
                TN++;
        }
        // false prediction
        else
        {
            false_predictions++;
            if( predicted_class == C_REJECT ) // predict : spam , email = ham -> false positive
                FP++;
            else                            // predict : ham , email = spam -> false negative
                FN++;
        }

        // store prediction
        predictions.add(new Pair(predicted_class,email_class));
    }


    // Update main Variables HashMap with each processed email
    private void updateVariablesHashMap(HashMap<String , VariableProperties> email_variables , int email_class)
    {
        int count;
        /*
           for every variable(word) stored in processed email's variables
           if it's already stored : update variable's properties
           if it's not stored : create new word and store it in main HashMap with its properties

           CASE : NaiveBayes_Version = BERNOULLI : increase word's frequency by 1 if word's freq > 0
           CASE : NaiveBayes_Version = MULTINOMIAL : increase word's frequency by its real frequency
        */
        for(String key : email_variables.keySet()) {

            int variable_frequency = email_variables.get(key).getFrequency(email_class , false);

            if (variables.containsKey(key))
                variables.get(key).setFrequency(email_class , variables.get(key).getFrequency(email_class,false) + variable_frequency);

            else
            {
                VariableProperties properties = new VariableProperties();
                variables.put(key , properties);
                variables.get(key).setFrequency(email_class , variable_frequency);

            }
        }
    }


    // Discard variables(words) with total frequency less than parameter k
    private HashMap<String, VariableProperties> discardVariablesBasedOn_k(HashMap<String , VariableProperties> map)
    {

        HashMap<String , VariableProperties> discardedMap = new HashMap<>(map);

        boolean bool;
        if ( VERSION == BERNOULLI )
            bool = true;
        else
            bool = false;

        for(String key : map.keySet())
        {
            VariableProperties var_properties = map.get(key);
            // discard is being done by checking each variables total (boolean) frequency
            int variable_total_frequency = var_properties.getFrequency(0,bool) + var_properties.getFrequency(1,bool);

            if ( variable_total_frequency < k )
                discardedMap.remove(key);
        }

        return discardedMap;
    }


    // Compute probabilities of : H(C) , P(C) , P(Xi|..) based on current trained data
    private void computeProbabilitiesBasedOnTrainedData(HashMap<String,VariableProperties> map)
    {
        totalVariableCount = map.size();

        P_C_0 = (float)HamEmailCount / (float)totalEmailCount; // P(C = 0) .. P(ham)
        P_C_1 = (float)SpamEmailCount / (float)totalEmailCount; // P(C = 1) .. P(spam)


        //compute H(C) given that Ce{0,1}
        H_C = -P_C_1 * log2(P_C_1) - P_C_0*log2(P_C_0);

        // case : NB_VERSION = MULTINOMIAL -> compute total count (freq) for each word for each class
        if(VERSION == MULTINOMIAL)
        {
            for(String key : map.keySet())
            {
                totalVariableFreq_0 += map.get(key).getFrequency(0,false);
                totalVariableFreq_1 += map.get(key).getFrequency(1,false);
            }
        }

        for(String key : map.keySet())
        {
            VariableProperties variableX = map.get(key);

            float freq_0;
            float freq_1;

            boolean bool;
            // case : NaiveBayes_Version = BERNOULLI -> we use bool_frequencies of variable
            if(VERSION == BERNOULLI)
            {
                bool = true;
                freq_0 = variableX.getFrequency(0 ,bool);
                freq_1 = variableX.getFrequency(1 , bool);

                // calculate probabilities P(wi|classX) and apply Laplace estimator to avoid zero probability
                variableX.setP_wi_0 ( (freq_0+1.0f)/((float)HamEmailCount+2.0f) ); // P(wi|H) = (hamFrequency+1)/(hamTotalCount + 2)
                variableX.setP_wi_1(  (freq_1+1.0f)/((float)SpamEmailCount+2.0f) ); // P(wi|S) = (spamFrequency+1)/(SpamTotalCount + 2)
            }

            // case : NaiveBayes_Version = MULTINOMIAL -> we use real frequencies of variable
            else if(VERSION == MULTINOMIAL)
            {
                bool = false;
                freq_0 = variableX.getFrequency(0 , bool);
                freq_1 = variableX.getFrequency(1 , bool);

                variableX.setP_wi_0 ( (freq_0+1.0f)/((float)totalVariableFreq_0 + (float)totalVariableCount) ); // P(wi|H) = (hamFrequency+1)/(hamTotalCount + 2)
                variableX.setP_wi_1( (freq_1+1.0f)/((float)totalVariableFreq_1 + (float)totalVariableCount) ); // P(wi|S) = (spamFrequency+1)/(SpamTotalCount + 2)
            }


            // calculate probabilities P(X = 0) and P(X = 1) where X : this.variable
            // we use the boolean frequencies of variable cause we only want to know if variableX exists or not
            variableX.setP_X_1 ( ((variableX.getFrequency(0,true) + variableX.getFrequency(1,true))+1.0f)/(totalEmailCount+2.0f));
            variableX.setP_X_0 ( 1.0f - variableX.getP_X_1() );

        }

        //for(String key : map.keySet())
           // map.get(key).computeProbabilities(); // Compute P(Xi|C) for each Variable X , Ce{0,1}

    }


    // Compute the IG value of our current trained data
    private HashMap<String , Float> computeInformationGainBasedOnTrainedData(HashMap<String,VariableProperties> map)
    {

        // HashMap IG_map stored the Information Gain (IG) for every variableX (word) of our current trained data
        HashMap<String , Float> IG_map = new HashMap<>();

        //compute IG for every variableX (word)
        for(String key : map.keySet())
        {
            VariableProperties variableX = map.get(key);

            // computing IG[X] ...
            // terms we need to calculate first : H(C|X = 0) , H(C|X = 1) and P(C=c|X=0) , P(C=c|X=1) ce{0,1}

            // P(C=0|X=1) && P(C=0|X=0)
            float P_c0_x1 =  variableX.getP_wi_0() * P_C_0 / variableX.getP_X_1();
            float P_c0_x0 =  (1.0f - variableX.getP_wi_0())*P_C_0 / variableX.getP_X_0();

            // P(C=1|X=1) && P(C=1|X=0)
            float P_c1_x1 = variableX.getP_wi_1() * P_C_1 / variableX.getP_X_1();
            float P_c1_x0 = (1.0f - variableX.getP_wi_1())*P_C_1 / variableX.getP_X_0();

            // P(Xi=1) && P(Xi=0)
            float P_X_1 = variableX.getP_X_1();
            float P_X_0 = variableX.getP_X_0();

            // H(C|X=0) && H(C|X=1)`
            float sum_HC_X_0 = -( P_c1_x0*log2(P_c1_x0) + P_c0_x0* log2(P_c0_x0) );
            float sum_HC_X_1 = -( P_c1_x1*log2(P_c1_x1) + P_c0_x1* log2(P_c0_x1) );

            // finally compute IG[X,C]
            float IG_X = H_C - ( P_X_1*sum_HC_X_0 + P_X_0*sum_HC_X_1); // IG(X,C) = H(C) - SUM{ P(X=x)*H(C|X=x)}


            // put on IG_map the IG value of variableX
            IG_map.put(key ,IG_X);
        }

        // sort IG_map by descending order
        IG_map = new LinkedHashMap<>(sortByValue(IG_map));

        return IG_map;
    }


    // create and return the new variable HashMap size of m , with the maximum IG values variables
    private HashMap<String,VariableProperties> getMaximum_m_variables_basedOnIG(HashMap<String,VariableProperties>var_map , HashMap<String,Float>IG_map)
    {
        HashMap<String,VariableProperties> new_var_map = new HashMap<>();

        for (String key : IG_map.keySet())
        {
            if(new_var_map.size()<m)
                new_var_map.put(key , var_map.get(key));
            else
                break;
        }

        return new_var_map;
    }



    private void testData()
    {


        System.out.println("HamEmailCount : "+HamEmailCount);
        System.out.println("SpamEmailCount : "+SpamEmailCount);
        System.out.println("TotalEmailCount : "+totalEmailCount);
    }

    // Holds each variables (word) properties






}
