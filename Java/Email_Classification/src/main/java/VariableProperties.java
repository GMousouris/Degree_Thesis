class VariableProperties
{


    // Real frequencies of variable X
    private int freq_0 = 0; // frequency on class ACCEPT (ham)
    private int freq_1 = 0; // frequency on class REJECT (spam)
    private int freq = 0; // frequency

    // boolean frequencies of variable X ( only care if X : exists in email Y )
    private int bool_freq_0 = 0; // (ham)
    private int bool_freq_1 = 0; // (spam)
    private int bool_freq = 0; // bool frequency

    private float P_wi_0 = 0.0f; // P(wi| ACCEPT) (ham)
    private float P_wi_1 = 0.0f; // P(wi| REJECT) (spam)

    private float P_X_1 = 0.0f; // P(X = 1) : P( variable x occurs in an email)
    private float P_X_0 = 0.0f; // P(X = 0) : P( variable x not occur in an email) = !P(X = 1)

    public VariableProperties()
    {

    }

    // sets the Frequency of variable on given category { 0 , 1 }
    public void setFrequency(int category , int number)
    {
        switch(category)
        {
            case DataTrainer.C_ACCEPT : freq_0 = number;
                bool_freq_0++;
                break;

            case DataTrainer.C_REJECT : freq_1 = number;
                bool_freq_1++;
                break;

            case -1 : freq = number;
            bool_freq++;
            break;

            default : //printError("VariableProperties" , "setFrequency(int,int)" , "Invalid category");
                break;
        }
    }

    // Increments the Frequency of variable on given category { 0 , 1 }
    public void incFrequency(int category , int count)
    {

        switch(category)
        {
            case DataTrainer.C_ACCEPT : freq_0 += count;
                break;

            case DataTrainer.C_REJECT : freq_1 += count;
                break;

            case -1 : freq += count;
            break;

            default : //printError("VariableProperties" , "incFrequency(int,int)" , "Invalid category");
                break;
        }
    }

    // Returns the Frequency of variable on given category { 0 , 1 }
    public int getFrequency(int category , boolean bool)
    {
        switch(category)
        {
            case DataTrainer.C_ACCEPT : return (bool? bool_freq_0 : freq_0);

            case DataTrainer.C_REJECT : return (bool? bool_freq_1 : freq_1);

            case -1 : return (bool? bool_freq : freq);

            default : //printError("VariableProperties" , "getFrequency(int)" , "Invalid category");
                return 0;
        }

    }

    public void setP_wi_0(float p) { P_wi_0 = p;}
    public void setP_wi_1(float p) { P_wi_1 = p;}
    public void setP_X_1(float p) {P_X_1 = p;}
    public void setP_X_0(float p){P_X_0 = p;}

    public float getP_wi_0(){ return P_wi_0;}
    public float getP_wi_1(){ return  P_wi_1;}
    public float getP_X_1(){ return P_X_1;}
    public float getP_X_0(){ return P_X_0;}


    // Calculate Probabilities P_wi_0 , P_wi_1
    public void computeProbabilities()
    {
            /*
            float freq_0;
            float freq_1;

            // case : NaiveBayes_Version = BERNOULLI -> we use bool_frequencies of variable
            if(VERSION == BERNOULLI)
            {
                freq_0 = bool_freq_0;
                freq_1 = bool_freq_1;

                // calculate probabilities P(wi|classX) and apply Laplace estimator to avoid zero probability
                P_wi_0 = (freq_0+1.0f)/((float)HamEmailCount+2.0f); // P(wi|H) = (hamFrequency+1)/(hamTotalCount + 2)
                P_wi_1 = (freq_1+1.0f)/((float)SpamEmailCount+2.0f); // P(wi|S) = (spamFrequency+1)/(SpamTotalCount + 2)
            }

            // case : NaiveBayes_Version = MULTINOMIAL -> we use real frequencies of variable
            else if(VERSION == MULTINOMIAL)
            {
                freq_0 = this.freq_0;
                freq_1 = this.freq_1;

                P_wi_0 = (freq_0+1.0f)/((float)totalVariableFreq_0 + (float)totalVariableCount); // P(wi|H) = (hamFrequency+1)/(hamTotalCount + 2)
                P_wi_1 = (freq_1+1.0f)/((float)totalVariableFreq_1 + (float)totalVariableCount); // P(wi|S) = (spamFrequency+1)/(SpamTotalCount + 2)
            }


            // calculate probabilities P(X = 0) and P(X = 1) where X : this.variable
            // we use the boolean frequencies of variable cause we only want to know if variableX exists or not
            P_X_1 = ((bool_freq_0 + bool_freq_1)+1.0f)/(totalEmailCount+2.0f);
            P_X_0 = 1.0f - P_X_1;
            */
    }


}