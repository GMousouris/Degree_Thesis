import java.util.Scanner;

public class Main {


    // set to false for no user input
    static boolean INPUT = false;


    /*
      * Default values for our game variables
      * Max depth is set to [5] for each player
      * Starting player is set to BLACK
      * Player controls : BLACK
      * AB_pruning is set to [true]
      *
      * May change through user's input
    */
    static int BLACK_MAXDEPTH = 2;
    static int WHITE_MAXDEPTH = 2;
    static int BOARD_STARTING_PLAYER = Board.BLACK;
    static int BOARD_PLAYER = Board.BLACK;
    static boolean AB_PRUNING = true;




    //Reading inputs from user//
    public static void initVariables()
    {
        Scanner sc = new Scanner(System.in);
        int counter = 0;
        int fail = 0;

        do
        {

            try
            {
                switch(counter)
                {

                    case 0 : System.out.print("\nBoard.BLACK set MiniMax max_depth : ");
                        BLACK_MAXDEPTH = sc.nextInt();
                        counter++;
                        break;

                    case 1 : System.out.print("\nBoard.WHITE set MiniMax max_depth : ");
                        WHITE_MAXDEPTH = sc.nextInt();
                        counter++;
                        break;

                    case 2 :
                        System.out.print("\nBoard.STARTING_PLAYER set : ");
                        if(fail == 0)
                        {
                            sc.nextLine();
                            fail ++;
                        }
                        String input = sc.nextLine();

                        if(input.toLowerCase().equals("black"))
                        {
                            BOARD_STARTING_PLAYER = Board.BLACK;
                            counter++;
                        }
                        else if(input.toLowerCase().equals("white"))
                        {
                            BOARD_STARTING_PLAYER = Board.WHITE;
                            counter++;
                        }
                        else
                        {
                            System.out.println("\n  ~Invalid input. Board.STARTING_PLAYER = {BLACK , WHITE}");
                        }
                        break;

                    case 3 : System.out.print("\nPlayer controls : ");
                        input = sc.nextLine();
                        if(input.toLowerCase().equals("black"))
                        {
                            BOARD_PLAYER = Board.BLACK;
                            counter++;
                        }
                        else if(input.toLowerCase().equals("white"))
                        {
                            BOARD_PLAYER = Board.WHITE;
                            counter++;
                        }
                        else if(input.toLowerCase().equals("none") || input.equals("0") || input.toLowerCase().equals("empty"))
                        {
                            BOARD_PLAYER = Board.EMPTY;
                            counter++;
                        }
                        else
                        {
                            System.out.println("  ~Invalid input. Player can control = {BLACK , WHITE , EMPTY}");
                        }
                        break;

                    case 4 : System.out.print("\nAB_PRUNING set : ");
                        input = sc.nextLine();
                        if(input.toLowerCase().equals("true"))
                        {
                            AB_PRUNING = true;
                            counter++;
                        }
                        else if(input.toLowerCase().equals("false"))
                        {
                            AB_PRUNING = false;
                            counter++;
                        }
                        else{
                            System.out.println("  ~Invalid input. AB_PRUNING = {true , false}");
                        }
                        break;

                }


            }
            catch (Exception e)
            {
                System.out.println("  ~Invalid input.");
                sc.next();
            }

        }while(counter <5);

    }

    //printing game variables
    public static void printGameInfo()
    {

        String start_player = "";
        if(BOARD_STARTING_PLAYER == Board.BLACK)
        {
            start_player = "BLACK";
        }
        else
        {
            start_player = "WHITE";
        }
        String player = "";
        if(BOARD_PLAYER == Board.BLACK)
        {
            player = "BLACK";
        }
        else if(BOARD_PLAYER == Board.WHITE)
        {
            player = "WHITE";
        }
        else{
            player = "NONE";
        }

        System.out.println(
                "\n"+
                        "               GAME_INFO"+"\n"+
                        "- - - - - - - - - - - - - - - - - - - - - - - - - - -"+"\n"+
                        "         o BLACK_MAXDEPTH : "+BLACK_MAXDEPTH+"\n"+
                        "         o WHITE_MAXDEPTH : "+WHITE_MAXDEPTH+"\n"+
                        "         o AB_PRUNING : "+AB_PRUNING+"\n"+
                        "         o STARTING_PLAYER : "+start_player+"\n"+
                        "         o PLAYER CONTROLS : "+player+"\n"+
                        "- - - - - - - - - - - - - - - - - - - - - - - - - - -"
        );

    }


    public static void PAUSE(int WAIT_TIME)
    {
        try
        {
            Thread.sleep(WAIT_TIME);
        }
        catch(Exception e)
        {

        }
    }


    public static void main(String[] args)
    {


        if(INPUT)
        {
            initVariables();
        }


        //Setting up our board Players
        GamePlayer BLACK = new GamePlayer( BLACK_MAXDEPTH , Board.BLACK);
        GamePlayer WHITE = new GamePlayer( WHITE_MAXDEPTH , Board.WHITE);
        //setting AB_PRUNING
        BLACK.setAB_PRUNING(AB_PRUNING);
        WHITE.setAB_PRUNING(AB_PRUNING);

        //Setting up our board
        Board board = new Board();
        board.setLastPlayerPlayed(BOARD_STARTING_PLAYER*(-1));
        board.setPlayer(BOARD_PLAYER);



        //adjusting helper's max depth of MiniMax
        if(BOARD_PLAYER != Board.EMPTY)
        {
            if(BOARD_PLAYER == Board.BLACK)
            {
                board.setHelpersDepth(WHITE.getMaxDepth()+5);
            }
            else
            {
                board.setHelpersDepth(BLACK.getMaxDepth()+5);
            }
        }

        //Giving some pause time between each turn for the GUI updates
        int WAIT_TIME;
        if(BOARD_PLAYER != Board.EMPTY)
        {
            WAIT_TIME = 600;
        }
        else
        {
            WAIT_TIME = 10;
        }

        //printing Game Info (variables etc.)
        printGameInfo();


        //initialize GUI
        board.initGUI();


        //Waiting for user to start the Game
        board.waitForUserToStartGame();


        double start_time = System.currentTimeMillis()/1000.0;
            //initialize game loop
        while(!board.isTerminal())
        {

            //which player played last turn -> it's the others turn to play
            switch (board.getLastPlayerPlayed())
            {


                //if WHITE played last , then BLACK plays now
                case Board.WHITE :

                    //if BLACK has no available moves he passed the round.
                    if(!board.hasMoves())
                    {
                        board.passTurn();
                        board.updateGUI();
                        break;
                    }
                    // if PLAYER controls BLACK
                    if(Board.BLACK == BOARD_PLAYER)
                    {
                        board.waitForUserInput();
                    }
                    // if AI controls BLACK
                    else {
                        Move Bmove = BLACK.MiniMax(board);
                        board.makeMove(Bmove.getRow() , Bmove.getCol() , Board.BLACK , true);
                    }

                    board.updateGUI();
                    if(BOARD_PLAYER == Board.EMPTY || BOARD_PLAYER == Board.BLACK)
                    {
                        PAUSE(WAIT_TIME);
                    }
                    break;//

                //if BLACK played last  , then WHITE plays now
                case Board.BLACK :

                    //if WHITE has no available moves he passed the round.
                    if(!board.hasMoves())
                    {
                        board.passTurn();
                        board.updateGUI();
                        break;
                    }
                    //if PLAYER controls WHITE
                    if(Board.WHITE == BOARD_PLAYER)
                    {
                        board.waitForUserInput();
                    }
                    //if AI controls WHITE
                    else {
                        Move Wmove = WHITE.MiniMax(board);
                        board.makeMove(Wmove.getRow() , Wmove.getCol() , Board.WHITE , true);
                    }

                    board.updateGUI();
                    if(BOARD_PLAYER == Board.EMPTY || BOARD_PLAYER == Board.WHITE)
                    {
                        PAUSE(WAIT_TIME);
                    }
                    break;//

                default:
                    break;

            }//switch


        }//while (Exits when board is terminal -> Game ends)

        double end_time = System.currentTimeMillis()/1000.0;
        board.endGame();

        //calculating total time passed to end the game minus PAUSE_TIME for each turn
        if(BOARD_PLAYER == Board.EMPTY)
        {
            double total_turns = board.getTotalTurns();
            double tota_PAUSE_TIME = total_turns*WAIT_TIME;
            tota_PAUSE_TIME *= 0.001;
            System.out.println("\n-Game duration : "+(end_time - start_time - tota_PAUSE_TIME ));
        }

    }//main


}
