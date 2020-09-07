public class Main

{

    public static void main(String[] args)
    {

        // Initialize DataTrainingInterface
        DataTrainingInterface DT_interface = new DataTrainingInterface();

        // Create a trainer ...


        for(int i =1; i<=9; i++)
        {
            /* Creating a NaiveBayes */
            NaiveBayes TRAINER = new NaiveBayes();
            TRAINER.setVersion(NaiveBayes.BERNOULLI);
            TRAINER.setParameter_k(3);
            TRAINER.setParameter_m(100);
            TRAINER.setTrainingDataFactor(0.1*i);
            TRAINER.setIG_FEATURE_SELECTION(true);
            /* */


        /* Creating an ID3
        ID3 TRAINER = new ID3();
        TRAINER.setVersion(ID3.BERNOULLI);
        TRAINER.setParameter_k(5);
        TRAINER.setParameter_Depth(15);
        TRAINER.setTrainingDataFactor(0.1*i);
        /* */




            // pass trainer and data to DT_interface
            DT_interface.InitDataTrainer(TRAINER);
            //DT_interface.setTrainingDataFactor(0.9);
            DT_interface.InitData(DataTrainingInterface.LINGSPAM_BARE);

            // Init training process
            DT_interface.InitTrainingProcess();

            // Init testing process
            DT_interface.InitTestingProcess();

        }



    }
}
