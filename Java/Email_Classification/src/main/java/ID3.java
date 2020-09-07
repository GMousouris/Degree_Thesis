import org.apache.commons.math3.*;
import org.apache.commons.math3.linear.MatrixUtils;
import org.apache.commons.math3.linear.RealMatrix;


import javax.print.DocFlavor;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.*;


public class ID3 extends DataTrainer
{

    // 2 Classes of emails ( Ham - Spam or ACCEPT - REJECT )
    static final int C_ACCEPT = 0;
    static final int C_REJECT = 1;

    // ID3 versions
    static final int MULTINOMIAL = 0;
    static final int BERNOULLI = 1;

    // Parameter k : minimum total frequency of word X in order to make it as an attribute
    private int k = 5;
    // Parameter m : maximum number of attributes with max IG value
    private int m = 500;


    // Spam and Ham email counts from train data
    private int totalVariableFreq_0 = 0;
    private int totalVariableFreq_1 = 0;
    private int totalVariableCount = 0;

    private float H_C = 0.0f; // H(C)
    private float P_C_0 = 0.0f; // P(C=0) .. P(ACCEPT) (ham)
    private float P_C_1 = 0.0f; // P(C=1) .. P(REJECT) (spam)

    //
    //HashMap<String , HashMap<String , VariableProperties>> emailVariables;
    ArrayList<String> VOCABULARY = new ArrayList<>();
    HashMap<Email , HashMap<String,VariableProperties>> emailVariables = new HashMap<>();

    Tree.Node<String> DT_root = null;
    int tree_nodes_counter = 0;
    ArrayList<String> closed_set = new ArrayList<>();

    Stack<Tree.Node<String>> stack = new Stack<>();


    //Default Constructor
    public ID3()
    {
        super(ID_3);
    }

    //
    public ID3(int version , int k)
    {
        super(ID_3);
        setVersion(version);
        this.k = k;
    }

    public ID3(int version , int k , int m)
    {
        super(ID_3);
        setVersion(version);
        this.k = k;
        this.m = m;
    }

    //Copy constructor
    public ID3(ID3 source)
    {
        super(ID_3);

        setVersion(source.VERSION);
        k = source.k;
        m = source.m;
        setTrainingDataFactor(source.train_data_factor);

    }


    public void setParameter_k(int k)
    {
        this.k = k;

    }
    public void setParameter_Depth(int m)
    {
        this.m = m;

    }

    public int getParameter_k(){return k;}
    public int getParameter_m(){return m;}
    public int getParameter_Depth(){return m;}


    // Initialize Training process on training data
    public boolean InitTrainingProcess()
    {

        System.out.println("-Initializing Training process.");


        System.out.println("-Processing raw data : "+INPUT_DATA+"\n" +
                "  o processing each email's data..");
        boolean process_raw_data = super.InitTrainingProcess();


        System.out.println("-Discarding variables with frequency less than : "+k);
        emailVariables = new HashMap<>(discardVariablesBasedOn_k(emailVariables)); // try discarding some variables (?)


        System.out.println("-Constructing Decision tree . . . ");
        //DT_root = new Tree.Node<>();
        DT_root = constructDecisionTreeBasedOnData( createArray2DBasedOnData(emailVariables) , 0 );


        // Discard variables with freq < k
        //variables = new HashMap<>(discardVariablesBasedOn_k(variables));

        // Compute probabilities based on trained data
        //computeProbabilitiesBasedOnTrainedData(variables);


        // Compute information gain based on current variables Xi
        //HashMap<String,Float> IG_map = new LinkedHashMap<>(computeInformationGainBasedOnTrainedData(variables));

        // Keep m attributes with the maximum IG value
        //variables = new HashMap<>(getMaximum_m_variables_basedOnIG(variables , IG_map));


        //System.out.println("Variables before : "+variables.size());
        //System.out.println("Variables after : "+variables.size());
        //testData();

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


    // xreiazomai vector gia ka9e email variables
    // wste na mporw na ftia3w enan pinaka en telei
    // k epeita otan 9a kanw construct to decision tree
    // na mporw na dhmiourgw ypopinakes tou arxikou pinaka (subsets tou S)
    // kai na ypologizw tis nees pi9anothtes me auton ton tropo
    // ara 9elw apache.math.common klp

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
                    String token = Tokenizer.nextToken().replaceAll("\\s","");
                    token = token.replaceAll("\\p{Punct}", "").replaceAll("[0-9]","").toLowerCase();
                    //String token = Tokenizer.nextToken().replaceAll("[0-9]","").replaceAll("\\s+","").toLowerCase();

                    if(token.equals(null) || token.equals("") || token.equals(" "))
                        continue;

                    if(Arrays.asList(stopwords).contains(token))
                        continue;
                    // process Token . . .
                    if(!VOCABULARY.contains(token))
                        VOCABULARY.add(token);



                    // case : word is already stored in email's HashMap
                    if(email_variables.containsKey(token))
                        email_variables.get(token).incFrequency(-1 , 1); // Increase its frequency by 1

                        // case : word isn't in email's HashMap
                        // store new word in email's HashMap
                    else
                    {
                        VariableProperties properties = new VariableProperties();
                        properties.incFrequency(-1 , 1);
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
            updateVariablesHashMap(email_variables , email.getName() ,  email_class);
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

        int predicted_class = -1;

        boolean state = true;
        Tree.Node<String> node = DT_root;

        while(state)
        {
            if(node.getValue().equals("ACCEPT"))
            {
                predicted_class = C_ACCEPT;
                break;
            }
            else if(node.getValue().equals("REJECT"))
            {
                predicted_class = C_REJECT;
                break;
            }
            else if(email_variables.containsKey(node.getValue()))
                node = node.getChild(1);
            else
                node = node.getChild(0);
        }

        /* */


        // correct prediction
        if(predicted_class == email_class)
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
            if(predicted_class == C_REJECT) // predict : spam , email = ham -> false positive
                FP++;
            else                            // predict : ham , email = spam -> false negative
                FN++;
        }

        // store prediction
        predictions.add(new Pair(predicted_class,email_class));
    }


    // Update main Variables HashMap with each processed email
    private void updateVariablesHashMap(HashMap<String , VariableProperties> email_variables ,String email_id, int email_class)
    {

        Email email = new Email(email_id , email_class);
        emailVariables.put(email , email_variables);
    }


    // Discard variables(words) with total frequency less than parameter k
    private HashMap<Email,HashMap<String, VariableProperties>> discardVariablesBasedOn_k(HashMap<Email , HashMap<String,VariableProperties>> map)
    {

        HashMap<Email , HashMap<String,VariableProperties>> discardedMap = new HashMap<>(map);
        ArrayList<String> to_remove = new ArrayList<>();

        for ( String variable : VOCABULARY)
        {
            int total_freq = 0;

            for ( Email email : emailVariables.keySet())
            {
                if(emailVariables.get(email).containsKey(variable))
                    total_freq++;
            }

            if(total_freq < k )
                to_remove.add(variable);
        }

       for (String var : to_remove)
       {

           for(Email email : emailVariables.keySet())
           {
               if(emailVariables.get(email).containsKey(var))
                   discardedMap.get(email).remove(var);
           }

           VOCABULARY.remove(var);
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
        H_C = -P_C_1 * (float)Math.log((double)P_C_1) - P_C_0*(float)Math.log((double)P_C_0);

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
            // case : ID3_Version = BERNOULLI -> we use bool_frequencies of variable

            bool = true;
            freq_0 = variableX.getFrequency(0 ,bool);
            freq_1 = variableX.getFrequency(1 , bool);

            // calculate probabilities P(wi|classX) and apply Laplace estimator to avoid zero probability
            variableX.setP_wi_0 ( (freq_0+1.0f)/((float)HamEmailCount+2.0f) ); // P(wi|H) = (hamFrequency+1)/(hamTotalCount + 2)
            variableX.setP_wi_1(  (freq_1+1.0f)/((float)SpamEmailCount+2.0f) ); // P(wi|S) = (spamFrequency+1)/(SpamTotalCount + 2)



            // calculate probabilities P(X = 0) and P(X = 1) where X : this.variable
            // we use the boolean frequencies of variable cause we only want to know if variableX exists or not
            variableX.setP_X_1 ( ((variableX.getFrequency(0,true) + variableX.getFrequency(1,true))+1.0f)/(totalEmailCount+2.0f));
            variableX.setP_X_0 ( 1.0f - variableX.getP_X_1() );

        }



    }


    private Tree.Node<String> constructDecisionTreeBasedOnData(RealMatrix SET , int depth  )
    {
        tree_nodes_counter++;
        //System.out.println("\n-----------------------\nDepth : "+depth);
        //System.out.println("Tree nodes : "+tree_nodes_counter);
        // SET size = 0
        //System.out.println("\n");



        String attribute = getMaxIG(SET);
        Tree.Node<String> node = new Tree.Node<>(attribute);

        if(attribute.equals("ACCEPT") || attribute.equals("REJECT"))
        {
            //System.out.println(" - Attribute : "+attribute);
            return node;
        }
        else if( depth >= m )
        {
            node.setValue(getMostFrequentValueOfSet(SET));
            //System.out.println(" - node reached max depth : got value : "+node.getValue());
            return node;
        }
        else
        {

            RealMatrix subset_1 = splitDataBasedOnAttribute(SET , attribute , 0.0d);
            RealMatrix subset_0 = splitDataBasedOnAttribute(SET , attribute , 1.0d);

            if(subset_0 == null && subset_1 == null)
                node.setValue(getMostFrequentValueOfSet(SET));
            else
            {
                RealMatrix subsets[] = { subset_1 , subset_0 };
                //int prun_state = getPruningState(SET , subsets);
                //System.out.println(prun_state);

                for(int i = 0; i<subsets.length; i++)
                {
                    if(subsets[i] == null)
                        node.addChild(new Tree.Node<String>(getMostFrequentValueOfSet(SET)));
                    else
                        node.addChild( constructDecisionTreeBasedOnData(subsets[i] , depth+1));
                }
            }

            /*
            if(prun_state == 0)
            {

            }
            else if(prun_state == 1)
            {
                node.setValue(getMostFrequentValueOfSet(SET));
            }
            else
            {
                node.setValue(getMostFrequentValueOfSet(SET));
            }
            */


        }

        //System.out.println("Tree nodes : "+tree_nodes_counter);
        return node;
    }

    private int getPruningState(RealMatrix parentSET , RealMatrix[] childs)
    {
        //float Class_count[] = {0.0f , 0.0f};
        //float Class_count[][] = { {0.0f, 0.0f} , {0.0f , 0.0f} , {0.0f , 0.0f}};
        //float Errors[] = new float[3];

        //RealMatrix nodes[] = { parentSET , childsSet[0] , childsSet[1]};
        ArrayList<RealMatrix> childsSet = new ArrayList<>();

        for(int i = 0; i<childs.length; i++)
        {
            if(childs[i] != null)
                childsSet.add(childs[i]);

        }

        if(childsSet.size() == 0) // childSet = {null}
            return -1;

        RealMatrix nodes[] = new RealMatrix[1 + childsSet.size()];
        nodes[0] = parentSET;
        int counter = 1;
        for(RealMatrix set : childsSet)
        {
            nodes[counter] = set;
            counter++;
        }

        float Class_count[][] = new float[nodes.length][2];
        float Errors[] = new float[nodes.length];

        for(int i = 0; i<nodes.length; i++)
        {
            for(int j = 0; j<nodes[i].getRowDimension(); j++)
            {

                if(nodes[i].getEntry(j , nodes[i].getColumnDimension()-1) == 1.0d)
                    Class_count[i][1] += 1.0f;
                else
                    Class_count[i][0] += 1.0f;

            }
        }

        for(int i = 0; i<nodes.length; i++)
        {
            float N =  Class_count[i][0] + Class_count[i][1];
            float n = max(Class_count[i][0] , Class_count[i][1]);
            float k = 2.0f;

            Errors[i] = (N-n+k-1.0f)/(N+k);
        }

        //float total_p =  Class_count[1][0]+Class_count[1][1] + Class_count[2][0] + Class_count[2][1];
        //float p_1 = Class_count[1][0]+Class_count[1][1];
        //float p_2 = Class_count[2][0]+Class_count[2][1];

        float p_i[] = new float[nodes.length-1];

        float total_p = 0.0f;
        for ( int i = 0; i<p_i.length; i++)
        {
            p_i[i] = Class_count[i+1][0] + Class_count[i+1][1];
            total_p += p_i[i];
        }

        //float Backed_up_Error = (p_1/total_p) * Errors[1] + (p_2/total_p) * Errors[2];
        float Backed_up_Error = 0.0f;
        for(int i = 0; i<p_i.length; i++)
        {
            Backed_up_Error +=  ( p_i[i]/total_p ) * Errors[i+1];
        }


        /*
        System.out.println("parent set : class[0] : "+Class_count[0][0]+"\n"+
        "parent set : class[1] : "+Class_count[0][1]+"\n"+
        "parent set : Error : "+Errors[0]+"\n"+
        "child set[0] : class[0] : "+Class_count[1][0]+"\n"+
                        "child set[0] : class[1] : "+Class_count[1][1]+"\n"+
                        "child set[0] : Error : "+Errors[1]+"\n"+
                        "child set[1] : class[0] : "+Class_count[2][0]+"\n"+
                        "child set[1] : class[1] : "+Class_count[2][1]+"\n"+
                        "child set[1] : Error : "+Errors[2]+"\n"+
                        "p_i[0] : "+p_i[0] + "\np_i[1] : "+p_i[1]+"\n"+
                        "total_p : "+total_p+"\n"+
                        "Backedup_Error : "+Backed_up_Error+"\n"+
        " ");

        Scanner sc = new Scanner(System.in);
        sc.nextInt();
        sc.nextLine();
        */

        if(Backed_up_Error > Errors[0])
            return 1;
        else
            return 0;

    }

    private String getMostFrequentValueOfSet(RealMatrix SET)
    {
        int ACCEPT = 0;
        int REJECT = 0;
        for(int i = 0; i<SET.getRowDimension(); i++)
        {
            if(SET.getEntry(i,SET.getColumnDimension()-1) == 0.0d)
                ACCEPT++;
            else
                REJECT++;
        }

        return (ACCEPT>REJECT)? "ACCEPT" : "REJECT";
    }


    private RealMatrix splitDataBasedOnAttribute(RealMatrix data , String attribute , double value)
    {
        int column = 0;
        for(int i = 0; i<VOCABULARY.size(); i++)
        {
            if(VOCABULARY.get(i).equals(attribute))
            {
                column = i;
                break;
            }
        }

        ArrayList<double[]> temp = new ArrayList<>();

        for(int i = 0; i<data.getRowDimension(); i++)
        {
            if(data.getEntry(i,column) == value)
            {
                double[] row = new double[data.getColumnDimension()-1];
                int counter = 0;
                for ( int j = 0; j<data.getColumnDimension(); j++)
                {

                    if(j!=column)
                    {
                        row[counter] = data.getEntry(i,j);
                        counter++;
                    }

                }

                temp.add(row);
            }
        }

        if(temp.size() == 0)
        {
            //System.out.println(" -- splitting data by attribute : {"+attribute+"} = null");
            return null;
        }

        RealMatrix splitData = MatrixUtils.createRealMatrix(new double[temp.size()][data.getColumnDimension()-1]);
        for(int i = 0; i<temp.size(); i++)
        {
            splitData.setRow(i , temp.get(i));
        }



        return splitData;
    }


    private RealMatrix createArray2DBasedOnData(HashMap<Email , HashMap<String,VariableProperties>> data)
    {
        double[][] DATA_2d = new double[data.size()][VOCABULARY.size()+1];
        RealMatrix Matrix2D = MatrixUtils.createRealMatrix(DATA_2d);

        int counter = 0;
        for( Email email : data.keySet())
        {
            Matrix2D.setRow(counter , getVectorFromEmail(data.get(email) , email.getCLASS()));
            counter++;
        }

        return Matrix2D;
    }


    private double[] getVectorFromEmail(HashMap<String, VariableProperties> emailVariables , int email_class)
    {
        double[] vector = new double[VOCABULARY.size()+1];

        for(int i = 0; i<VOCABULARY.size(); i++)
        {
            if(emailVariables.containsKey(VOCABULARY.get(i)))
                vector[i] = 1.0d;
            else
                vector[i] = 0.0d;

        }

        vector[vector.length-1] = email_class;
        return vector;
    }


    private String getMaxIG(RealMatrix SET)
    {
        //Entropy(SET)
        float Entropy_SET;
        float CLASS_ACCEPT = 0;
        float CLASS_REJECT = 0;
        float SET_SIZE = SET.getRowDimension();

        for(int i = 0; i<SET.getRowDimension(); i++)
        {

            if(SET.getEntry(i , SET.getColumnDimension()-1) == 1)
                CLASS_REJECT++;
            else
                CLASS_ACCEPT++;
        }


        //System.out.println("\n ACCEPT : "+CLASS_ACCEPT);
        //System.out.println(" REJECT : "+CLASS_REJECT);

        if(CLASS_ACCEPT == 0)
            return "REJECT";
        else if(CLASS_REJECT == 0)
            return "ACCEPT";

        Entropy_SET = - ( (CLASS_ACCEPT/SET_SIZE)*log2(CLASS_ACCEPT/SET_SIZE)  +  (CLASS_REJECT/SET_SIZE)*log2(CLASS_REJECT/SET_SIZE) );

        //System.out.println("Entropy(SET) : "+Entropy_SET);

        // compute information Gain for each variable in set
        float max_ig = -100.0f;
        String max_key = null;

        for(int j = 0; j<SET.getColumnDimension()-1; j++)
        {

            float attrib_IG = getAttributeIG(SET , j , Entropy_SET);

            if(attrib_IG > max_ig)
            {
                max_ig = attrib_IG;
                max_key = VOCABULARY.get(j);
            }


        }

        //System.out.println("SET : ["+(SET.getRowDimension())+"]["+SET.getColumnDimension()+"]");
        //System.out.println("Max key : "+max_key +" | IG : "+max_ig);
        return  max_key;
    }


    private float getAttributeIG(RealMatrix SET , int column , float Entropy_SET)
    {

        float attrib_count_0 = 0.0f;
        float class_0[] = {0.0f,0.0f};

        float attrib_count_1 = 0.0f;
        float class_1[] = {0.0f,0.0f};


        float Entropy_0 = 0.0f;
        float Entropy_1 = 0.0f;

        for ( int row = 0; row<SET.getRowDimension(); row++)
        {
            if(SET.getEntry(row , column) == 0.0d) // attribute's value = 0.0d
            {
                attrib_count_0 += 1.0f;
                if(SET.getEntry(row,SET.getColumnDimension()-1) == 0.0d) // class of email is = 0 {ACCEPT}
                    class_0[0] += 1.0f;
                else                                                         // class of email is = 1 {REJECT}
                    class_0[1] += 1.0f;
            }
            else                                  // attribute's value = 1.0d
            {
                attrib_count_1 += 1.0f;
                if(SET.getEntry(row,SET.getColumnDimension()-1) == 0.0d) // class of email is = 0 {ACCEPT}
                    class_1[0] += 1.0f;
                else                                                         // class of email is = 1 {REJECT}
                    class_1[1] += 1.0f;
            }
        }

        if(attrib_count_0 == 0.0f) // case : attribute has no value 0 in this SET
        {
            Entropy_0 = 0.0f;
        }
        else
        {
            float p_0 = class_0[0]/attrib_count_0;
            float p_1 = class_0[1]/attrib_count_0;
            Entropy_0 = - (  p_0 * log2(p_0) +  p_1 * log2(p_1) );
        }
        if(attrib_count_1 == 0.0f) // case : attribute has no value 1 in this SET
        {
            Entropy_1 = 0.0f;
        }
        else
        {
            float p_0 = class_1[0]/attrib_count_1;
            float p_1 = class_1[1]/attrib_count_1;
            Entropy_1 = - (  p_0 * log2(p_0) +  p_1 * log2(p_1) );
        }

        float p_0 = attrib_count_0/(attrib_count_0+attrib_count_1);
        float p_1 = attrib_count_1/(attrib_count_0+attrib_count_1);

        float attrib_IG = Entropy_SET - (  p_0 * Entropy_0 + p_1 * Entropy_1 );
        float splitInfo =  (p_0 * log2(p_0) +  p_1 * log2(p_1));




        if(attrib_IG < 0.0f )
        {

            /*
            System.out.println("- - - - ~getAttributeIG()~ - - - - -"+"\n"+
            "attrib_0_count : "+attrib_count_0+"\n"+
                            "attrib_0 class[0] : "+class_0[0]+"\n"+
                    "attrib_0 class[1] : "+class_0[1]+"\n"+
                    "attrib_1 class[0] : "+class_1[0]+"\n"+
                    "attrib_1 class[1] : "+class_1[1]+"\n"+
            "attrib_1_count : "+attrib_count_1+"\n"+
            "Entropy_SET : "+Entropy_SET+"\n"+
            "Entropy_attrib_0 : "+Entropy_0+"\n"+
            "Entropy_attrib_1 : "+Entropy_1+"\n"+
            "Attrib_IG : "+attrib_IG+"\n");
            Scanner sc = new Scanner(System.in);
            sc.nextLine();
            sc.nextInt();
            */
            attrib_IG = 0.0f;
        }

        return attrib_IG;
    }


    // Compute the IG value of our current trained data
    private String getMaxInformationGainBasedOnData(HashMap<String,VariableProperties> map)
    {


        String maxKey = null;
        float maxIG = -10.0f;

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

            // H(C|X=0) && H(C|X=1)
            float sum_HC_X_0 = -( P_c1_x0*(float)Math.log((double)P_c1_x0) + P_c0_x0*(float)Math.log((double)P_c0_x0) );
            float sum_HC_X_1 = -( P_c1_x1*(float)Math.log((double)P_c1_x1) + P_c0_x1*(float)Math.log((double)P_c0_x1) );

            // finally compute IG[X,C]
            float IG_X = H_C - ( P_X_1*sum_HC_X_0 + P_X_0*sum_HC_X_1); // IG(X,C) = H(C) - SUM{ P(X=x)*H(C|X=x)}


            if(IG_X > maxIG)
            {
                maxIG = IG_X;
                maxKey = key;
            }
        }

        return maxKey;
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

            // H(C|X=0) && H(C|X=1)
            float sum_HC_X_0 = -( P_c1_x0*(float)Math.log((double)P_c1_x0) + P_c0_x0*(float)Math.log((double)P_c0_x0) );
            float sum_HC_X_1 = -( P_c1_x1*(float)Math.log((double)P_c1_x1) + P_c0_x1*(float)Math.log((double)P_c0_x1) );

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









}
