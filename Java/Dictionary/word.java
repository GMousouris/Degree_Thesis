import java.util.ArrayList;


public class word{


    private String name;
    private ArrayList<String> synonyms = new ArrayList<String>();
    private ArrayList<String> translations = new ArrayList<String>();
    private ArrayList<String> notes = new ArrayList<String>();
  


    /*-----------------------------CONSTRUCTORS<<<<---------------------------*/

    public word(){}


	public word(String name){

		this.name = name;
	}


	public word(String name,ArrayList<String> synonyms){
		this.name = name;
		this.synonyms = new ArrayList<String>(synonyms);
	}


	public word(String name,ArrayList<String> synonyms,ArrayList<String> translations){
		this.name = name;
		this.synonyms = new ArrayList<String>(synonyms);
		this.translations = new ArrayList<String>(translations);
	}

	public word(String name,ArrayList<String> synonyms,ArrayList<String> translations,ArrayList<String> notes){
		this.name = name;
		this.synonyms = new ArrayList<String>(synonyms);
		this.translations = new ArrayList<String>(translations);
		this.notes = new ArrayList<String>(notes);

	}

	

    /*-----------------------------CONSTRUCTORS>>>>>---*/


	public void add_synonyms(ArrayList<String> syn){
		this.synonyms = new ArrayList<String>(syn);

	}

	public void clear_synonyms(){
		this.synonyms.clear();
	}

	public void clear_translations(){
		this.translations.clear();
	}

	public void add_translations(ArrayList<String> trans){
        this.translations = new ArrayList<String>(trans);
	}

	public ArrayList<String> get_translations(){
		return this.translations;
	}

	public void print_info(){
		System.out.println("Word : "+this.name);
		System.out.println("Synonyms :");
		for(String s : this.synonyms){
			System.out.println(s);
		}
		System.out.println("\nTranslations :");
		for(String s: this.translations){
			System.out.println(s);
		}
	}




	public String get_word(){
		return this.name;
	}

	public void set_Name(String name){
		this.name = name;
	}

	public ArrayList<String> get_synonyms(){
		return this.synonyms;
	}

	public ArrayList<String> get_notes(){
		return this.notes;
	}

	public void add_notes(ArrayList<String> notes){
		this.notes = new ArrayList<String>(notes);
	}

	public void clear_notes(){
		this.notes.clear();
	}

	
}