import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.ArrayList;
import java.io.*;
import java.io.File;
import java.nio.file.Files;
import java.awt.event.*; 
import java.nio.file.Paths;
import java.nio.file.Path;



public class Database{

    private String[] texts;
    private String[] paths;
    private word wrd;

    private ArrayList<String> synonyms = new ArrayList<String>();
    private ArrayList<String> translations = new ArrayList<String>();
    private ArrayList<String> notes = new ArrayList<String>();
    private int added[] = new int[26];
    private int removed[] = new int[26];
    private String line;
    private ArrayList<ArrayList<word>> database = new ArrayList<ArrayList<word>>();
    private ArrayList<word> temp_array = new ArrayList<word>();
    private boolean done = false;
    private int pointer = 0;
    private int counter = 0;

    //---------------Constructor--------------------
	public Database(){
     

	texts = new String[26];
	paths = new String[26];

	line = "";


    texts[0] = "A.txt";
    texts[1] = "B.txt";
    texts[2] = "C.txt";
    texts[3] = "D.txt";
    texts[4] = "E.txt";
    texts[5] = "F.txt";
    texts[6] = "G.txt";
    texts[7] = "H.txt";
    texts[8] = "I.txt";
    texts[9] = "J.txt";
    texts[10] = "K.txt";
    texts[11] = "L.txt";
    texts[12] = "M.txt";
    texts[13] = "N.txt";
    texts[14] = "O.txt";
    texts[15] = "P.txt";
    texts[16] = "Q.txt";
    texts[17] = "R.txt";
    texts[18] = "S.txt";
    texts[19] = "T.txt";
    texts[20] = "U.txt";
    texts[21] = "V.txt";
    texts[22] = "W.txt";
    texts[23] = "X.txt";
    texts[24] = "Y.txt";
    texts[25] = "Z.txt";

    paths[0] = "A";
    paths[1] = "B";
    paths[2] = "C";
    paths[3] = "D";
    paths[4] = "E";
    paths[5] = "F";
    paths[6] = "G";
    paths[7] = "H";
    paths[8] = "I";
    paths[9] = "J";
    paths[10] = "K";
    paths[11] = "L";
    paths[12] = "M";
    paths[13] = "N";
    paths[14] = "O";
    paths[15] = "P";
    paths[16] = "Q";
    paths[17] = "R";
    paths[18] = "S";
    paths[19] = "T";
    paths[20] = "U";
    paths[21] = "V";
    paths[22] = "W";
    paths[23] = "X";
    paths[24] = "Y";
    paths[25] = "Z";

	}

	public ArrayList<ArrayList<word>> get_database(){
		return this.database;
	}

    public int[] get_added(){
        return this.added;
    }

    public int[] get_removed(){
        return this.removed;
    }

    public void set_added(int[] array){
        this.added = array.clone();
    }

    public void set_removed(int[] array){
        this.removed = array.clone();
    }

    

	


    //------------------------read A~Z & save to ArrayList of ArrayLists---->>
    public  void read_database(){

     
        
        
        

    	int count = 0;
        String check = "Synonyms{";
        String temp_line="";
        String name ="";
        String line2 ="";
        int reading = 0;

		
        for(int i=0; i<26; i++){

              
        	    count = 0;
        	    temp_line="";
        	    name ="";
        	    line2 ="";
        	    reading = 0;

            	try(BufferedReader br = new BufferedReader(new FileReader("Database\\"+texts[i]))){
    
		    	while((line = br.readLine()) !=null){

		    		temp_line = line;
		    		name = line;
                   
                    if(!name.equals("")){
                        read_synonyms(paths[i],name);
                        read_translations(paths[i],name);
                        read_notes(paths[i],name);
                        wrd = new word(name,synonyms,translations,notes);
                        temp_array.add(wrd);
                    }
		    		

                    synonyms.clear();
                    translations.clear();
                    notes.clear();
		    		

		    	}

		    }catch(Exception E){System.out.println("Error! Text = "+texts[i]);}



		    this.database.add(i,new ArrayList<word>(temp_array));
		    temp_array.clear();
    

        }
        

		System.out.println("...Reading Database Completed!");
        
        

    }

    //--------------------Write on Database from A~Z through ArrayList of ArrayLists------>>
    public void write_database(ArrayList<ArrayList<word>> database){

    	ArrayList<ArrayList<word>> temp_data = new ArrayList<ArrayList<word>>(database);



        for(int i=0; i<temp_data.size(); i++){


        	try {

        		FileWriter wr = new FileWriter("Database\\"+texts[i]);
        		BufferedWriter bw = new BufferedWriter(wr);




        		for(word s : temp_data.get(i)){

        			ArrayList<String> syn = new ArrayList<String>(s.get_synonyms());
                    ArrayList<String> trans = new ArrayList<String>(s.get_translations());
                    ArrayList<String> notess = new ArrayList<String>(s.get_notes());

        			bw.write(s.get_word());
        			bw.newLine();
                    try{

                   	    FileWriter wrInfo = new FileWriter("Database\\Info\\"+paths[i]+"\\"+s.get_word()+".txt");
        			    BufferedWriter bwInfo = new BufferedWriter(wrInfo);
    
        			    bwInfo.write("Synonyms{");
        			    bwInfo.newLine();
        			    for(int k=0; k<syn.size(); k++){
        			    	bwInfo.write(syn.get(k));
        			    	bwInfo.newLine();
        			    }
        			    bwInfo.write("}");

                        bwInfo.newLine();
                        bwInfo.newLine();

                        bwInfo.write("Translations{");
                        bwInfo.newLine();
                        for(int z=0; z<trans.size(); z++){
                            bwInfo.write(trans.get(z));
                            bwInfo.newLine();
                        }
                        bwInfo.write("}");
                        bwInfo.newLine();
                        bwInfo.newLine();
                        
                        bwInfo.write("Notes{");
                        bwInfo.newLine();
                        for(int k=0; k<notess.size(); k++){
                            bwInfo.write(notess.get(k));
                            bwInfo.newLine();
                        }
                        bwInfo.write("}");
                        bwInfo.newLine();
                        bwInfo.newLine();
                        
        			    bwInfo.close();

                    }catch(Exception e){}


        			
        		 }
        		 bw.close();

        		

        	}catch(Exception E){}



        }
      
        System.out.println("...Writing at Database Completed!");
    	    
  
    }

    public void read_translations(String path,String name){
        int count = 0;
        int reading = 1;
        int reading_trans = 0;
        String temp_line = "";
        String line =  "";

        try(BufferedReader brSyn = new BufferedReader(new FileReader("Database\\Info\\"+path+"\\"+name+".txt"))){
    
            while((line = brSyn.readLine()) !=null && reading==1){

                    temp_line = line;


                    if(reading_trans==1){

                        if(temp_line.equals("}")){
                            count++;
                            reading = 0;
                            reading_trans = 0;
                            break;
                        }
                        else if(temp_line!=""){
                            translations.add(temp_line);
                            
                        }
                    }


                    if(temp_line.equals("Translations{")){
                        reading_trans = 1;
                    }

                }
            }catch(Exception E){}

     
    } 



    public void read_synonyms(String path,String name){

    	int count = 0;
    	int reading = 1;
    	int reading_syn = 0;
    	String temp_line = "";
        String line = "";

    	try(BufferedReader brSyn = new BufferedReader(new FileReader("Database\\Info\\"+path+"\\"+name+".txt"))){
    
		    	while((line = brSyn.readLine()) !=null && reading==1){

		    		temp_line = line;


		    		if(reading_syn==1){

		    			if(temp_line.equals("}")){
		    				count++;
		    				reading = 0;
		    				reading_syn = 0;
		    				break;
		    			}
		    			else if(temp_line!=""){
		    				synonyms.add(temp_line);
		    				
		    			}
		    		}


		    		if(temp_line.equals("Synonyms{")){
		    			reading_syn = 1;
		    		}

		    	}
		    }catch(Exception E){}



    }


    public void read_notes(String path,String name){
        int count = 0;
        int reading = 1;
        int reading_notes = 0;
        String temp_line = "";
        String line =  "";

        try(BufferedReader brSyn = new BufferedReader(new FileReader("Database\\Info\\"+path+"\\"+name+".txt"))){
    
            while((line = brSyn.readLine()) !=null && reading==1){

                    temp_line = line;


                    if(reading_notes==1){

                        if(temp_line.equals("}")){
                            count++;
                            reading = 0;
                            reading_notes = 0;
                            break;
                        }
                        else if(temp_line!=""){
                            notes.add(temp_line);
                            
                        }
                    }


                    if(temp_line.equals("Notes{")){
                        reading_notes = 1;
                    }

                }
            }catch(Exception E){}


    } 


    public void delete_word_files(String word_name,int id){
      
        try{

            File file = new File("Database\\Info\\"+paths[id]+"\\"+word_name+".txt");
            if(file.delete()){
                System.out.println(file.getName() + " is deleted!");
            }else{
                System.out.println("Delete operation is failed.");
            }

        }catch(Exception e){

            e.printStackTrace();

        }


    }

 

   
    





}