public class Pointer{


    private int x;
    private int y;
    private word wordd;

	public Pointer(word wordd,int x, int y){
		this.x = x;
		this.y = y;
		this.wordd= wordd;
	}
	public Pointer(int x,int y){
		this.x = x;
		this.y = y;
	}

	public void setX(int x){
		this.x = x;
	}

	public void setY(int y){
		this.y = y;
	}


	public int getX(){
		return this.x;
	}

	public int getY(){
		return this.y;
	}

	public word getWord(){
		return this.wordd;
	}
}