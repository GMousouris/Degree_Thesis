import java.util.ArrayList;
import java.util.Random;


public class GamePlayer
{
    //Max depth that miniMax algorithm is going to reach
    private int maxDepth;
    private int player;

    private boolean TEST = false;
    private boolean AB_PRUNING = true;
    private boolean CLOSED_SET = true;

    private ArrayList<Board> closed_set;

    /*constructors*/
    public GamePlayer()
    {
        maxDepth = 2;
        player = Board.BLACK;
    }

    public GamePlayer(int mD, int player)
    {
        maxDepth = mD;
        this.player = player;
        closed_set = new ArrayList<Board>();
    }


    /*Minimax_Algorithm*/
    public Move MiniMax(Board board)
    {


        if(player == Board.BLACK)
        {
            if(TEST)
            {
                System.out.println("\n\n~MAX ALGORITHM | BLACK IS PLAYING~");
            }
            return MAX(new Board(board), 0 , Integer.MIN_VALUE , Integer.MAX_VALUE);
        }
        else
        {
            if(TEST)
            {
                System.out.println("\n\n~MIN ALGORITHM | WHITE IS PLAYING~");
            }

            return MIN(new Board(board), 0 , Integer.MIN_VALUE , Integer.MAX_VALUE);
        }
    }
    //


    public Move MAX(Board board, int depth , int a  , int b )
    {
        if(TEST)
        {
            System.out.println("\n-Entering : MAX("+depth+")");
        }
        Random r = new Random();



        // in case that MAX is called when max_depth has been already reached or
        // the state is terminal or the Board has no available moves for current Player
        if((board.isTerminal()) || (depth == maxDepth) || !board.hasMoves() )
        {

            Move lastMove = new Move(board.getLastMove().getRow(), board.getLastMove().getCol(), board.evaluate());
            if(TEST)
            {
                if(board.isTerminal())
                {
                    System.out.println(" -BOARD IS TERMINAL-");
                }
                System.out.println("RETURNING -> MAXMOVE[TERMINAL] :  ["+lastMove.getRow()+"]["+lastMove.getCol()+"] | VALUE : "+lastMove.getValue());
            }
            return lastMove;
        }



        ArrayList<Board> children = new ArrayList<Board>(board.getChildren(Board.BLACK));
        Move maxMove = new Move(Integer.MIN_VALUE);

        for(Board child : children)
        {


            Move move = MIN(child , depth + 1 , a , b);

            if(move.getValue() >= maxMove.getValue())
            {

                if((move.getValue() == maxMove.getValue()))
                {
                    if(r.nextInt(2) == 0)
                    {
                        maxMove.setRow(child.getLastMove().getRow());
                        maxMove.setCol(child.getLastMove().getCol());
                        maxMove.setValue(move.getValue());
                    }
                }
                else
                {
                    maxMove.setRow(child.getLastMove().getRow());
                    maxMove.setCol(child.getLastMove().getCol());
                    maxMove.setValue(move.getValue());
                }
            }

            // if A-B PRUNING IS ACTIVE
            if(AB_PRUNING)
            {
                if ( maxMove.getValue() >= b )
                {
                    return maxMove;
                }
                else
                {
                    a = max(a , maxMove.getValue());
                }
            }


        }

        if(TEST)
        {
            System.out.println("RETURNING -> MAXMOVE : ["+maxMove.getRow()+"]["+maxMove.getCol()+"] | value : "+maxMove.getValue());
        }

        return maxMove;
    }
    //



    public Move MIN(Board board, int depth , int a , int b )
    {
        if(TEST)
        {
            System.out.println("\n-Entering : MIN("+depth+")");
        }
        Random r = new Random();

        // in case that MAX is called when max_depth has been already reached or
        // the state is terminal or the Board has no available moves for current Player
        if((board.isTerminal()) || (depth == maxDepth) || !board.hasMoves() )
        {
            Move lastMove = new Move(board.getLastMove().getRow(), board.getLastMove().getCol(), board.evaluate());
            if(TEST)
            {
                if(board.isTerminal())
                {
                    System.out.println(" -BOARD IS TERMINAL-");
                }
                System.out.println("RETURNING -> MINMOVE[TERMINAL] : ["+lastMove.getRow()+"]["+lastMove.getCol()+"] | value : "+lastMove.getValue());
            }
            return lastMove;
        }

        ArrayList<Board> children = new ArrayList<Board>(board.getChildren(Board.WHITE));
        Move minMove = new Move(Integer.MAX_VALUE);


        for(Board child : children)
        {


            Move move = MAX(child , depth + 1 , a , b );

            if(move.getValue() <= minMove.getValue())
            {

                if((move.getValue() == minMove.getValue()))
                {
                    if(r.nextInt(2) == 0)
                    {
                        minMove.setRow(child.getLastMove().getRow());
                        minMove.setCol(child.getLastMove().getCol());
                        minMove.setValue(move.getValue());
                    }
                }
                else
                {
                    minMove.setRow(child.getLastMove().getRow());
                    minMove.setCol(child.getLastMove().getCol());
                    minMove.setValue(move.getValue());
                }
            }


            //if A-B PRUNNING IS ACTIVE
            if(AB_PRUNING)
            {

                if (minMove.getValue() <= a)
                {
                    return minMove;
                }
                else
                {
                    b = min( b , minMove.getValue());
                }

            }


        }

        if(TEST)
        {
            System.out.println("RETURNING -> MINMOVE : ["+minMove.getRow()+"]["+minMove.getCol()+"] | value : "+minMove.getValue());
        }
        return minMove;

    }
    //

    public int getMaxDepth()
    {
        return maxDepth;
    }

    public void setAB_PRUNING(boolean state)
    {
        AB_PRUNING = state;
    }

    public void setCLOSED_SET(boolean state)
    {
        CLOSED_SET = state;
    }

    public void TESTmode(boolean state)
    {
        TEST = state;
    }


    private boolean closed_set_contains(Board child)
    {
        int size = closed_set.size();

        for(Board board : closed_set)
        {
            if(board.isEqual(child))
            {
                return true;
            }
        }

        return false;
    }



    public int max( int a , int b)
    {
        if( a >= b )
        {
            return a;
        }
        else{
            return b;
        }
    }

    public int min( int a , int b)
    {
        if( a <= b)
        {
            return a;
        }
        else
        {
            return b;
        }
    }

}
