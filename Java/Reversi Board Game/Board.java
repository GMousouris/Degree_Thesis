import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Random;
import java.util.stream.Collectors;
import java.util.stream.IntStream;


public class Board implements ActionListener
{


    //static values for the board
    public static final int BLACK = 1;
    public static final int WHITE = -1;
    public static final int EMPTY = 0;

    public static final int WIDTH = 8;
    public static final int HEIGHT = 8;
    //


    /*
     * Variables that hold information about the flanks of a Move
     * ex. If a move(row , col) registers HORIZONTAL_LEFT = true that means
     * that this move of PlayerA flanks horizontally enemy checker(s).
     * The same goes with HORIZONTAL_RIGHT , VERTICAL_LEFT , etc.
     * The 'LEFT' , 'RIGHT' , 'UP' , 'DOWN' , holds the direction that
     * we need to move to , from Move's (row , col) so we will end
     * at a friendly checker.
    */
    private boolean HORIZONTAL_LEFT = false;
    private boolean HORIZONTAL_RIGHT = false;

    private boolean VERTICAL_UP = false;
    private boolean VERTICAL_DOWN = false;

    private boolean DIAGONAL_UPLEFT = false;
    private boolean DIAGONAL_UPRIGHT = false;
    private boolean DIAGONAL_DOWNLEFT = false;
    private boolean DIAGONAL_DOWNRIGHT = false;
    //


    //holds the SUM of black / white checker of the board
    private int sum_black = 0;
    private int sum_white = 0;

    //
    private int PLAYER = 0;
    private boolean userInput = false;
    private boolean MAKING_MOVE = false;

    //Last move that lead to current board
    private Move lastMove;


    //value that keeps the last player who played
    private int lastPlayerPlayed;


    // our data for the reversi board
    // 8 x 8 : board
    private int [][] gameBoard;

    // DEPTH of the helper in case a PLAYER is controlling BLACK / WHITE
    private int DEPTH = 2;

    //Keeping track of board's data for each turn
    private ArrayList<Integer> currentPlayer; // current player for each turn
    private ArrayList<int[][]> states; //gameBoard[][] data for each turn
    private ArrayList<BoardIndex> states_move; //What move happened each turn
    private int turn = 0;
    private int total_turns = 0;
    private int WINNER = 0;
    private int score = 0;
    private boolean END = false;
    private boolean START = false;




    //GUI elements
    private JFrame window;
    private JPanel panel;
    private JLabel state_label , turn_counter;
    private BoardChecker[][] board_pieces;
    private JButton left, right;
    private JButton help;
    private JButton start;

    private boolean capturing = false;
    private boolean WAITING_INPUT = false;

    private boolean EXPORT = true;

    //

    /*Constructors*/
    public Board()
    {


        currentPlayer = new ArrayList<Integer>();
        states = new ArrayList<int[][]>();
        states_move = new ArrayList<BoardIndex>();

        lastMove = new Move();
        //default First Player = BLACK
        lastPlayerPlayed = WHITE;
        currentPlayer.add(BLACK);
        //
        gameBoard = new int[HEIGHT][WIDTH];

        //initializing the gameBoard data
        init_gameBoard(null);
        states.add(getGameBoard());

    }


    public Board(Board board)
    {
        //Bplayer = new GamePlayer(2 , Board.BLACK);
        //Wplayer = new GamePlayer(2 , Board.WHITE);

        //states = new ArrayList<int[][]>();

        this.lastMove = board.lastMove;
        lastPlayerPlayed = board.lastPlayerPlayed;
        gameBoard = new int[HEIGHT][WIDTH];
        init_gameBoard(board.gameBoard);
        //initGUI();

    }
    /* */

    public void reset()
    {
        turn = 0;
        init_gameBoard(null);
        total_turns = 0;
        END = false;
        left.setEnabled(false);
        right.setEnabled(false);
    }

    //Initializes gameBoard[][] data
    private void init_gameBoard(int[][] gameBoard)
    {

        // initializing values for default starting board //
		/*

	      - - - - - [8] - - - - - -     
		'  E  E  E  E  E  E  E  E  '
		'  E  E  E  E  E  E  E  E  '
		'  E  E  E  E  E  E  E  E  '
		'  E  E  E [W][B] E  E  E [8]
		'  E  E  E [B][W] E  E  E  ' 
		'  E  E  E  E  E  E  E  E  '
		'  E  E  E  E  E  E  E  E  '
		'  E  E  E  E  E  E  E  E  ' 
		' _ _ _ _ _ _ _ _ _ _ _ _  ' 
      
		* where :
		* E = EMPTY
		* W = WHITE checker
		* B = BLACK checker
		* and [8] x [8] is the grid size of the board
		*/

        if(gameBoard == null)
        {
            for(int i = 0; i<HEIGHT; i++)
            {
                for(int j = 0; j<WIDTH; j++)
                {
                    this.gameBoard[i][j] = EMPTY;
                }
            }
            this.gameBoard[3][3] = WHITE;
            this.gameBoard[4][4] = WHITE;

            this.gameBoard[3][4] = BLACK;
            this.gameBoard[4][3] = BLACK;


        }
        else{

            //initializing values based on a given gameboard//
            for(int i = 0; i<HEIGHT; i++){
                for(int j = 0; j<WIDTH; j++){
                    this.gameBoard[i][j] =  gameBoard[i][j];
                }
            }

        }


    }
    //


    //Initializes GUI
    public void initGUI()
    {
        //setting up the window
        window = new JFrame("Reversi Board");
        window.setSize(new Dimension(600,605));
        window.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        window.setResizable(false);
        window.setLayout(null);

        Insets w_ins = window.getInsets();

        //setting up the label
        state_label = new JLabel("");
        state_label.setSize(new Dimension(300 , 40));
        state_label.setFont(new Font(state_label.getFont().getFontName() , Font.BOLD , 24));
        state_label.setForeground(Color.black);
        state_label.setBackground(null);
        state_label.setOpaque(false);
        state_label.setBorder(null);

        String info;
        if(lastPlayerPlayed == BLACK)
        {
            if(PLAYER == WHITE)
            {
                info = "YOU ARE PLAYING";
            }
            else{
                info = "WHITE IS PLAYING";
            }

        }
        else
        {
            if(PLAYER == BLACK)
            {
                info = "YOU ARE PLAYING";
            }
            else{
                info = "BLACK IS PLAYING";
            }

        }
        state_label.setText(info);
        state_label.setBounds(w_ins.left+180 , w_ins.top + 5 , state_label.getWidth(), state_label.getHeight() );


        //setting up the panel
        panel = new JPanel(null);
        panel.setSize(new Dimension(500,500));
        panel.setBounds(w_ins.left+40 ,  w_ins.top+40 ,panel.getWidth(),panel.getHeight() );
        panel.setBackground(new Color(39, 163, 26));

        Insets ins = panel.getInsets();


        //setting up the Board checkers
        board_pieces = new BoardChecker[HEIGHT][WIDTH];

        int x = 50;
        int y = 50;

        for(int i = 0; i < HEIGHT; i++)
        {
            for(int j = 0; j < WIDTH; j++)
            {
                board_pieces[i][j] = new BoardChecker();

                if(gameBoard[i][j] == BLACK)
                {
                    board_pieces[i][j].setIMG(BLACK);
                }
                else  if(gameBoard[i][j] == WHITE)
                {
                    board_pieces[i][j].setIMG(WHITE);
                }
                else{
                    board_pieces[i][j].setIMG(EMPTY);
                }

                board_pieces[i][j].addActionListener(this);
                board_pieces[i][j].setBounds(ins.left + (j*x)+ 1 + 50 , ins.top + (i*y) + 1 + 35 , x , y );
                panel.add(board_pieces[i][j]);
            }
        }

        //setting up start button
        start = new JButton("START");
        start.setSize(new Dimension(80,25));
        start.setFocusPainted(false);
        start.setEnabled(true);
        start.setBounds(ins.left + 50 , ins.top + (8*50) + 1 + 35 + 20 , 80 , 25);
        start.addActionListener(this);

        //setting up prev - next buttons
        left = new JButton("PREV");
        left.setSize(new Dimension(80,25));
        left.setFocusPainted(false);
        left.setBounds(ins.left + 167 , ins.top + (8*50) + 1 + 35 + 20 , 80 ,25 );
        left.setEnabled(false);
        left.addActionListener(this);

        right = new JButton("NEXT");
        right.setSize(new Dimension(100,25));
        right.setFocusPainted(false);
        right.setBounds(ins.left + 167 + 80 + 10 , ins.top + (8*50) + 1 + 35 + 20 ,80 ,25 );
        right.setEnabled(false);
        right.addActionListener(this);


        //setting up help button in case a PLAYER is controlling BLACK or WHITE
        help = new JButton("HELP");
        help.setSize(65,25);
        help.setFocusPainted(false);
        help.setBounds(ins.left + 167 + 80 + 10 + 130 , ins.top + (8*50) + 1 + 35 + 20 ,65 ,25 );
        help.setEnabled(false);
        help.addActionListener(this);
        //


        turn_counter = new JLabel();
        turn_counter.setSize(new Dimension(600 , 25));
        turn_counter.setFont(new Font(turn_counter.getFont().getFontName() ,  Font.BOLD , 15));
        turn_counter.setBackground(null);
        turn_counter.setForeground(Color.black);
        turn_counter.setOpaque(false);
        turn_counter.setBorder(null);

        turn_counter.setBounds(w_ins.left + 40 , w_ins.top + 540 , turn_counter.getWidth() ,  turn_counter.getHeight() );
        turn_counter.setText("TURN : "+turn+"                                                                  BLACK ["+0 +"] vs WHITE ["+0+"]");
        //


        panel.add(start);
        panel.add(left);
        panel.add(right);
        panel.add(help);

        window.add(state_label);
        window.add(turn_counter);
        window.add(panel);

        window.revalidate();
        window.repaint();
        window.setVisible(true);

    }


    //Updates GUI elements
    public void updateGUI()
    {

        String info = "";
        //Turn that game ends and there is a winner
        if(turn == total_turns && WINNER!= 0)
        {

            if(WINNER == BLACK)
            {
                if(BLACK == PLAYER)
                {
                    info = "YOU WON THE GAME!";
                }
                else
                {
                    info = "BLACK WON THE GAME!";
                }

            }
            else //if WINNER = WHITE
            {
                if(WHITE == PLAYER)
                {
                    info = "YOU WON THE GAME!";
                }
                else
                {
                    info = "WHITE WON THE GAME";
                }
            }

        }
        //Turn that game ends and there is no winner -> tie
        else if(turn == total_turns && WINNER == 0)
        {
            info = "GAME IS TIE!";
        }
        else{
            //updating which player is currently playing
            if(currentPlayer.get(turn) == WHITE )
            {
                if(PLAYER == WHITE)
                {
                    info = "YOU ARE PLAYING";
                }
                else{
                    info = "WHITE IS PLAYING";
                }

            }
            else
            {
                if(PLAYER == BLACK)
                {
                    info = "YOU ARE PLAYING";
                }
                else{
                    info = "BLACK IS PLAYING";
                }

            }

        }
        state_label.setText(info);


        //updating board_pieces according to the current board
        for(int i = 0; i<WIDTH; i++)
        {
            for(int j = 0; j<HEIGHT; j++)
            {

                    if(gameBoard[i][j] == WHITE)
                    {
                        board_pieces[i][j].setIMG(WHITE);
                    }
                    else if(gameBoard[i][j] == BLACK)
                    {
                        board_pieces[i][j].setIMG(BLACK);
                    }
                    else{
                        board_pieces[i][j].setIMG(EMPTY);
                    }


                if(turn > 0)
                {
                    if(states_move.get(turn-1).x == i && states_move.get(turn-1).y == j)
                    {
                        board_pieces[i][j].setText("["+i+","+j+"]");
                        board_pieces[i][j].drawText(i,j);
                    }
                    else{
                        board_pieces[i][j].setText("");
                        board_pieces[i][j].drawText(-1,-1);
                    }

                }
                else{
                    board_pieces[i][j].setText("");
                    board_pieces[i][j].drawText(-1,-1);
                }

            }
        }

        evaluate();
        turn_counter.setText("TURN : "+turn+"                                                                  BLACK ["+sum_black +"] vs WHITE ["+sum_white+"]");

        panel.revalidate();
        panel.repaint();

        window.revalidate();
        window.repaint();


    }



    /*
     * Make a move on the board. Places a BLACK/WHITE checker on the board.
     * Continues by capturing all enemy's checkers that are being flanked
     * horizontally , vertically or diagonally.
    */
    public void makeMove(int row, int col, int player , boolean state)
    {

        resetFlags();
        isValidMove(row,col);

        //In case an Exception occurs we catch it , and export the gameBoard[][]
        //for testing-debugging purposes.
        try
        {
            gameBoard[row][col] = player;
        }
        catch(ArrayIndexOutOfBoundsException e)
        {

            e.printStackTrace();
            if(EXPORT)
            {
                exportGameBoard();
            }
            System.exit(0);
        }


        // Finding enemy checkers that are going to get captured
        // by checking all possible directions : Horizontal / Vertical / Diagonal
        ArrayList<BoardIndex> captured_pieces = new ArrayList<BoardIndex>();
        captured_pieces.add(new BoardIndex(row,col));

        //Finding Horizontal enemy checkers that are flanked
        if(HORIZONTAL_LEFT)
        {
            int rrow = row;
            int ccol = col - 1;

            int last_piece_shown = gameBoard[rrow][ccol];

            while( last_piece_shown == lastPlayerPlayed )
            {
                captured_pieces.add(new BoardIndex(rrow,ccol));

                ccol += (-1);

                if (ccol < 0 || ccol >= WIDTH || rrow < 0 || rrow >= HEIGHT) {
                    break;
                }

                last_piece_shown = gameBoard[rrow][ccol];

            }
        }
        if(HORIZONTAL_RIGHT)
        {

            int rrow = row;
            int ccol = col + 1;

            int last_piece_shown = gameBoard[rrow][ccol];
            while( last_piece_shown == lastPlayerPlayed )
            {
                captured_pieces.add(new BoardIndex(rrow,ccol));

                ccol += (+1);

                if (ccol < 0 || ccol >= WIDTH || rrow < 0 || rrow >= HEIGHT) {
                    break;
                }

                last_piece_shown = gameBoard[rrow][ccol];

            }

        }
        //Finding vertical enemy checkers that are flanked
        if(VERTICAL_UP)
        {

            int rrow = row - 1;
            int ccol = col;

            int last_piece_shown = gameBoard[rrow][ccol];

            while( last_piece_shown == lastPlayerPlayed )
            {
                captured_pieces.add(new BoardIndex(rrow,ccol));

                rrow += (-1);

                if (ccol < 0 || ccol >= WIDTH || rrow < 0 || rrow >= HEIGHT) {
                    break;
                }

                last_piece_shown = gameBoard[rrow][ccol];

            }

        }
        if(VERTICAL_DOWN)
        {

            int rrow = row + 1;
            int ccol = col;

            int last_piece_shown = gameBoard[rrow][ccol];

            while( last_piece_shown == lastPlayerPlayed )
            {
                captured_pieces.add(new BoardIndex(rrow,ccol));

                rrow += (+1);

                if (ccol < 0 || ccol >= WIDTH || rrow < 0 || rrow >= HEIGHT) {
                    break;
                }

                last_piece_shown = gameBoard[rrow][ccol];

            }

        }
        //Finding diagonal enemy checkers that are flanked
        if(DIAGONAL_UPLEFT)
        {

            int rrow = row - 1;
            int ccol = col - 1;

            int last_piece_shown = gameBoard[rrow][ccol];

            while( last_piece_shown == lastPlayerPlayed )
            {
                captured_pieces.add(new BoardIndex(rrow,ccol));

                rrow += (-1);
                ccol += (-1);

                if (ccol < 0 || ccol >= WIDTH || rrow < 0 || rrow >= HEIGHT) {
                    break;
                }

                last_piece_shown = gameBoard[rrow][ccol];

            }

        }
        if(DIAGONAL_UPRIGHT)
        {

            int rrow = row - 1;
            int ccol = col + 1;

            int last_piece_shown = gameBoard[rrow][ccol];

            while( last_piece_shown == lastPlayerPlayed )
            {
                captured_pieces.add(new BoardIndex(rrow,ccol));

                rrow += (-1);
                ccol += (+1);

                if (ccol < 0 || ccol >= WIDTH || rrow < 0 || rrow >= HEIGHT) {
                    break;
                }

                last_piece_shown = gameBoard[rrow][ccol];

            }

        }
        if(DIAGONAL_DOWNLEFT)
        {

            int rrow = row + 1;
            int ccol = col - 1;

            int last_piece_shown = gameBoard[rrow][ccol];

            while( last_piece_shown == lastPlayerPlayed )
            {
                captured_pieces.add(new BoardIndex(rrow,ccol));

                rrow += (+1);
                ccol += (-1);

                if (ccol < 0 || ccol >= WIDTH || rrow < 0 || rrow >= HEIGHT) {
                    break;
                }

                last_piece_shown = gameBoard[rrow][ccol];

            }

        }
        if(DIAGONAL_DOWNRIGHT)
        {

            int rrow = row + 1;
            int ccol = col + 1;

            int last_piece_shown = gameBoard[rrow][ccol];

            while( last_piece_shown == lastPlayerPlayed )
            {
                captured_pieces.add(new BoardIndex(rrow,ccol));

                rrow += (+1);
                ccol += (+1);

                if (ccol < 0 || ccol >= WIDTH || rrow < 0 || rrow >= HEIGHT) {
                    break;
                }

                last_piece_shown = gameBoard[rrow][ccol];

            }

        }


        //Finally , capturing flanked enemy checkers
        for(BoardIndex capturedPiece : captured_pieces)
        {
            int x = capturedPiece.x;
            int y = capturedPiece.y;
            gameBoard[x][y] = player;
        }

        //capturing animation only if makeMove() is called for main Board
        if(PLAYER != EMPTY && PLAYER != player && state)
        {
            new Thread(new AnimationHandler(captured_pieces)).start();
        }



        //Only if : makeMove() is being called for main Board.
        if(state)
        {
            currentPlayer.add(lastPlayerPlayed);
            states.add(getGameBoard()); //adding new gameBoard[][] to states
            states_move.add(new BoardIndex(row,col)); //adding the given move to moves.
            turn++;
        }

        lastMove = new Move(row , col);
        lastPlayerPlayed = player;

        //waiting for capturing animation to end before continuing
        while(capturing){
            try{
                Thread.sleep(2);
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }


    }
    //


    //Player pass the turn cause available moves = 0
    public void passTurn()
    {

        currentPlayer.add(lastPlayerPlayed);
        lastPlayerPlayed = lastPlayerPlayed*(-1);
        states.add(getGameBoard());
        states_move.add(new BoardIndex(-1,-1));
        turn++;
    }


    /*
     * Checks if move is valid.
     * a move in reversi is considered valid as long as
     * the checker that will get placed on the board ,
     * flanks the other player's checker(s) horizontally , vertically
     * or diagonally.
     * It is possible to flank enemy's checker(s) on more than 1 directions ( vertical || horizontal || diagonal ).
    */
    public boolean isValidMove(int row, int col)
    {



        // check if move is out-of-bounds
        if(row >= HEIGHT || row < 0 || col >= WIDTH || col < 0 )
        {
            return false;
        }

        // check if there is already a piece at the given pos[row][col]
        if(gameBoard[row][col] != EMPTY)
        {
            return false;
        }

        // checking flanks on all possible directions
        boolean HORIZONTAL = checkHorizontalValidity(row , col);
        boolean VERTICAL = checkVerticalValidity(row , col);
        boolean DIAGONAL = checkDiagonalValidity(row , col);

        // a move is valid as long as it is flanking on 1 or more directions
        return ( HORIZONTAL || VERTICAL || DIAGONAL );
    }

    private boolean checkHorizontalValidity(int row, int col)
    {
        //checking left of the given pos [row][col]
        boolean HORIZONTAL =  false;

        if(col > 0)
        {
            int counter = 0;
            int ccol = col - 1;
            int last_piece_shown = gameBoard[row][ccol];
            while(last_piece_shown == lastPlayerPlayed && ccol > 0)
            {
                counter++;
                ccol--;
                last_piece_shown = gameBoard[row][ccol];
            }

            if(last_piece_shown ==  lastPlayerPlayed*(-1) && counter > 0)
            {

                HORIZONTAL_LEFT = true;
                HORIZONTAL = true;

            }

        }

        //checking right of the given pos
        if(col < WIDTH -1)
        {
            int counter = 0;
            int ccol = col + 1;
            int last_piece_shown = gameBoard[row][ccol];
            while(last_piece_shown == lastPlayerPlayed && ccol < WIDTH - 1)
            {
                counter++;
                ccol++;
                last_piece_shown = gameBoard[row][ccol];
            }

            if(last_piece_shown == lastPlayerPlayed*(-1) && counter > 0)
            {


                HORIZONTAL_RIGHT = true;
                HORIZONTAL = true;

            }
        }



        return HORIZONTAL;
    }

    private boolean checkVerticalValidity(int row, int col)
    {
        boolean VERTICAL = false;

        //checking up of the given pos [row][col]
        if(row > 0)
        {
            int counter = 0;
            int rrow = row - 1;
            int last_piece_shown = gameBoard[rrow][col];
            while(last_piece_shown == lastPlayerPlayed && rrow > 0)
            {
                counter++;
                rrow--;
                last_piece_shown = gameBoard[rrow][col];
            }

            if(last_piece_shown ==  lastPlayerPlayed*(-1) && counter > 0)
            {

                VERTICAL_UP = true;
                VERTICAL = true;

            }

        }

        //checking down of the given pos
        if(row < HEIGHT -1)
        {
            int counter = 0;
            int rrow = row + 1;
            int last_piece_shown = gameBoard[rrow][col];
            while(last_piece_shown == lastPlayerPlayed && rrow < HEIGHT - 1)
            {
                counter++;
                rrow++;
                last_piece_shown = gameBoard[rrow][col];
            }

            if(last_piece_shown ==  lastPlayerPlayed*(-1) && counter > 0)
            {

                VERTICAL_DOWN = true;
                VERTICAL = true;
            }
        }
        return VERTICAL;

    }

    private boolean checkDiagonalValidity(int row, int col)
    {

        boolean DIAGONAL = false;

        //checking the upper - left diagonal
        if( row > 0 )
        {
            if( col > 0 )
            {
                int counter = 0;
                int rrow = row - 1;
                int ccol = col -1;
                int last_piece_shown = gameBoard[rrow][ccol];
                while(last_piece_shown == lastPlayerPlayed && rrow > 0 && ccol > 0)
                {
                    counter++;
                    rrow--;
                    ccol--;
                    last_piece_shown = gameBoard[rrow][ccol];
                }

                if(last_piece_shown == lastPlayerPlayed*(-1) && counter > 0)
                {

                    DIAGONAL_UPLEFT = true;
                    DIAGONAL = true;
                    //return true;
                }
            }
        }

        //checking the upper-right diagonal
        if( row > 0 )
        {
            if( col < WIDTH -1 )
            {
                int counter = 0;
                int rrow = row - 1;
                int ccol = col +1;
                int last_piece_shown = gameBoard[rrow][ccol];
                while(last_piece_shown == lastPlayerPlayed && rrow > 0 && ccol < WIDTH -1)
                {
                    counter++;
                    rrow--;
                    ccol++;
                    last_piece_shown = gameBoard[rrow][ccol];
                }

                if(last_piece_shown == lastPlayerPlayed*(-1) && counter > 0)
                {

                    DIAGONAL_UPRIGHT = true;
                    DIAGONAL = true;

                }
            }
        }

        //checking the down-left diagonal
        if( row < HEIGHT -1 )
        {
            if( col > 0 )
            {
                int counter = 0;
                int rrow = row + 1;
                int ccol = col -1;
                int last_piece_shown = gameBoard[rrow][ccol];
                while(last_piece_shown == lastPlayerPlayed && rrow < HEIGHT-1 && ccol > 0)
                {
                    counter++;
                    rrow++;
                    ccol--;
                    last_piece_shown = gameBoard[rrow][ccol];
                }

                if(last_piece_shown == lastPlayerPlayed*(-1) && counter > 0)
                {

                    DIAGONAL_DOWNLEFT = true;
                    DIAGONAL = true;

                }
            }
        }
        //checking the down - right diagonal
        if( row < HEIGHT -1 )
        {
            if( col < WIDTH - 1 )
            {
                int counter = 0;
                int rrow = row + 1;
                int ccol = col + 1;
                int last_piece_shown = gameBoard[rrow][ccol];
                while(last_piece_shown == lastPlayerPlayed && rrow < HEIGHT-1 && ccol < WIDTH -1)
                {
                    counter++;
                    rrow++;
                    ccol++;
                    last_piece_shown = gameBoard[rrow][ccol];
                }

                if(last_piece_shown == lastPlayerPlayed*(-1) && counter > 0)
                {

                    DIAGONAL_DOWNRIGHT = true;
                    DIAGONAL = true;

                }
            }
        }

        return DIAGONAL;
    }


    /* Generates the children of the state
     * Any valid move on the board, results to a child
    */
    public ArrayList<Board> getChildren(int player)
    {


        ArrayList<Board> children = new ArrayList<Board>();

        for(int row = 0; row<WIDTH; row++){
            for(int col = 0; col<HEIGHT; col++){

                if(isValidMove(row , col))
                {

                    Board child = new Board(this);
                    child.makeMove(row , col ,player,false);
                    children.add(child);
                }
            }
        }


        return children;
    }



    /*
     * The heuristic we use to evaluate is
     * the total number of each player's pieces on the board vs the other's .
     * So for a given state N , if h(N) > h(Other possible states) , then we do prefer
     * the N state.
    */
    public int evaluate()
    {
        sum_black = 0;
        sum_white = 0;

        for(int i = 0; i < HEIGHT; i++)
        {
            for(int j = 0; j < WIDTH; j++)
            {
                if(gameBoard[i][j] == BLACK)
                {
                    sum_black++;
                }
                else if(gameBoard[i][j] == WHITE)
                {
                    sum_white++;
                }
            }
        }

        return sum_black - sum_white;

    }



    /*
     * A state in Reversi is terminal if there are no more
     * valid moves on the board , meaning that
     * neither of the 2 players , can make a valid
     * move on the board.
    */
    public boolean isTerminal()
    {

        boolean A = true;
        boolean B = true;

        //checking valid moves for playerA
        for( int row = 0; row<HEIGHT; row++)
        {
            for( int col = 0; col < WIDTH; col++)
            {
                if(isValidMove(row , col)){
                    return false;


                }
            }
        }

        lastPlayerPlayed *= (-1);

        //checking valid moves for playerB
        for( int row = 0; row<HEIGHT; row++)
        {
            for( int col = 0; col < WIDTH; col++)
            {
                if(isValidMove(row , col)){
                    lastPlayerPlayed *= (-1);
                    return false;


                }
            }
        }

        return true;
    }


    //checks if a player has valid available moves on the board
    public boolean hasMoves()
    {
        for( int row = 0; row < HEIGHT; row ++)
        {
            for( int col = 0; col < WIDTH; col ++)
            {
                if(isValidMove(row , col))
                {
                    return true;
                }
            }
        }

        return false;
    }

    //Returns TRUE if 2 boards have equal gameBoard[][] data
    public boolean isEqual(Board board)
    {

        return false;
    }


    //resets Directional flags
    private void resetFlags()
    {
        HORIZONTAL_RIGHT = false;
        HORIZONTAL_LEFT = false;
        VERTICAL_DOWN = false;
        VERTICAL_UP = false;
        DIAGONAL_DOWNRIGHT = false;
        DIAGONAL_DOWNLEFT = false;
        DIAGONAL_UPLEFT = false;
        DIAGONAL_UPRIGHT = false;
    }


    // Waiting for user Input if a PLAYER controls BLACK / WHITE
    public void waitForUserInput()
    {
        help.setEnabled(true);
        WAITING_INPUT = true;
        while(!userInput)
        {
            try
            {
                Thread.sleep(2);
            }
            catch(Exception e)
            {

            }
        }
        userInput = false;
        //WAITING_INPUT = false;
        help.setEnabled(false);
    }

    public void waitForUserToStartGame()
    {

        while(!START)
        {
            try
            {
             Thread.sleep(2);
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }
    }


    /*
     * Ending game
     * Enabling [PREV / NEXT] iteration through game states
    */
    public void endGame()
    {
        total_turns = turn;
        score = evaluate();
        END = true;
        left.setEnabled(true);
        right.setEnabled(true);

        if(score > 0)
        {
            WINNER = BLACK;
        }
        else if(score < 0)
        {
            WINNER = WHITE;
        }
        else
        {
            WINNER = 0;
        }
        updateGUI();
    }


    /* getters */
    public Move getLastMove()
    {
        return lastMove;
    }

    public int getWinner()
    {
        return WINNER;
    }

    public int getTotalTurns()
    {
        return total_turns;
    }

    public int getLastPlayerPlayed()
    {
        return lastPlayerPlayed;
    }


    public int[][] getGameBoard()
    {

        int[][] new_gameBoard = new int[HEIGHT][WIDTH];

        for( int i = 0; i < HEIGHT; i++)
        {
            for( int j = 0; j < WIDTH; j++)
            {
                new_gameBoard[i][j] = gameBoard[i][j];
            }
        }
        return new_gameBoard;
    }


    /* setters */
    public void setHelpersDepth(int depth)
    {
        DEPTH = depth;
    }

    public void setLastPlayerPlayed(int player)
    {
        lastPlayerPlayed = player;
        currentPlayer.set(turn , player*(-1));
    }

    public void setLastMove(Move lastMove)
    {
        this.lastMove.setRow(lastMove.getRow());
        this.lastMove.setCol(lastMove.getCol());
        this.lastMove.setValue(lastMove.getValue());
    }

    public void setGameBoard(int[][] gameBoard)
    {
        for( int i = 0; i< HEIGHT; i++)
        {
            for(int j = 0; j < WIDTH; j++)
            {

                this.gameBoard[i][j] = gameBoard[i][j];

            }
        }
    }

    public void setPlayer(int PLAYER)
    {
        this.PLAYER = PLAYER;
    }

    public void setExportMode(boolean state)
    {
        EXPORT = false;
    }
    /* */

    /* Utilities */
    private void playSound(String soundName){
        try
        {
            AudioInputStream audioInputStream = AudioSystem.getAudioInputStream(new File(soundName));
            Clip clip = AudioSystem.getClip( );
            clip.open(audioInputStream);
            clip.start( );
        }
        catch(Exception ex)
        {
            ex.printStackTrace( );
        }
    }


    private void exportGameBoard()
    {
        PrintWriter out = null;
        try
        {
            out = new PrintWriter("TEST.txt" , "UTF-8");
        }
        catch(Exception e)
        {
            e.printStackTrace();
        }

        String[] lines = new String[8];
        for(int i = 0; i<HEIGHT; i++)
        {
            lines[i] = "";
            for(int j = 0; j<WIDTH; j++)
            {
                lines[i] += String.valueOf(gameBoard[i][j]) + " ";
            }
        }

        for(String line : lines)
        {
            out.println(line);
        }

        out.println("PLAYER : "+lastPlayerPlayed*(-1));
        out.close();

        System.out.println("~GameBoard saved to file 'TEST.txt'\n~EXITING PROGRAM!");
    }
    /* */

    @Override
    public void actionPerformed(ActionEvent e) {

        // prev state
        // turn --
        if(e.getSource().equals(left))
        {
            if(turn > 0)
            {
                turn--;
            }
            else{
                turn = total_turns;
            }

            setGameBoard(states.get(turn));
            updateGUI();
        }
        // next state
        // turn ++
        else if(e.getSource().equals(right)){

            if(turn < total_turns)
            {
                turn++;
            }
            else{
                turn = 0;
            }

            setGameBoard(states.get(turn));
            updateGUI();
        }
        //help
        else if(!END && e.getSource().equals(help))
        {
            // running miniMax algorithm with helper's set maximum depth to find the best possible move for PLAYER
            GamePlayer player = new GamePlayer( DEPTH , PLAYER );
            Move move = player.MiniMax(this);
            // showing the PLAYER best move's position on the Board
            new Thread(new AnimationHandler(move.getRow(),move.getCol())).start();

        }
        //start
        else if(e.getSource().equals(start))
        {
            START = true;
            start.setEnabled(false);
        }
        //any board_piece[x][y] (input from PLAYER)
        else if(!END && WAITING_INPUT){ //!END && START && WAITING_INPUT && !capturing
            for( int i = 0; i < HEIGHT; i++)
            {
                for( int j = 0; j < WIDTH; j++)
                {
                    if(board_pieces[i][j].equals(e.getSource()))
                    {
                        if(isValidMove(i , j))
                        {
                            MAKING_MOVE = true;
                            userInput = true;
                            WAITING_INPUT = false;

                            makeMove(i , j , PLAYER , true);
                            playSound("sounds/checker_sound.wav");

                        }
                    }
                }
            }
        }




    }

    //Class that Holds indices [x][y]
    class BoardIndex {

        private int x = -1;
        private int y = -1;

        public BoardIndex(int x , int y)
        {
            this.x = x;
            this.y = y;
        }
    }



    //Class that Handles a graphic animation
    class AnimationHandler implements Runnable
    {

        private int x;
        private int y;
        private ArrayList<BoardIndex> captured_pieces;

        public AnimationHandler(int x, int y)
        {
            this.x = x;
            this.y = y;

        }

        public AnimationHandler(ArrayList<BoardIndex> captured_pieces)
        {
            this.captured_pieces = new ArrayList<BoardIndex>(captured_pieces);
            capturing = true;
        }
        public void run()
        {
            //HELP animation
            if(!capturing)
            {
                MAKING_MOVE = false;

                Color def_color = BoardChecker.getDefaultColor();

                final int def_r = def_color.getRed();
                final int def_g = def_color.getGreen();
                final int def_b = def_color.getBlue();

                int r = def_r;
                int g = def_g;
                int b = def_b;

                while(g < 255 && !MAKING_MOVE)
                {
                    r +=2;
                    g++;
                    b++;

                    board_pieces[x][y].setBackground(new Color(r,g,b));
                    board_pieces[x][y].revalidate();
                    board_pieces[x][y].repaint();

                    try
                    {
                        Thread.sleep(5);
                    }
                    catch(Exception e)
                    {

                    }

                }
                while(g > def_g && !MAKING_MOVE)
                {
                    r -= 2;
                    g--;
                    b--;

                    board_pieces[x][y].setBackground(new Color(r,g,b));
                    board_pieces[x][y].revalidate();
                    board_pieces[x][y].repaint();

                    try
                    {
                        Thread.sleep(10);
                    }
                    catch(Exception e)
                    {

                    }


                }
                board_pieces[x][y].setBackground(new Color(39,163,26));
            } // if
            //Capturing Animation
            else
            {

                for( BoardIndex capturePiece : captured_pieces)
                {
                    int x = capturePiece.x;
                    int y = capturePiece.y;
                    if(gameBoard[x][y] == BLACK)
                    {
                        board_pieces[x][y].setIMG(BLACK);
                    }
                    else
                    {
                        board_pieces[x][y].setIMG(WHITE);
                    }
                    board_pieces[x][y].revalidate();
                    board_pieces[x][y].repaint();



                    try
                    {
                        Thread.sleep(50);
                    }
                    catch (Exception e)
                    {
                        e.printStackTrace();
                    }
                }
                capturing = false;

            }


        }
    }//


}