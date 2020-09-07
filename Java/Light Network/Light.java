import javax.swing.JButton;
import javax.swing.JLabel;
import java.awt.Color;
import javax.swing.BorderFactory;
import java.awt.event.*;

/*
Base class : <Light>
Holding data {
-String label : holds the label of each Light instance
-Light_Info info : holds the info_panel for each Light
-boolean state : holds the state of each light [on/off]
-boolean hover_state : holds the hover_state(mouse_entered) of each light
-boolean pressed_state : holds the pressed_state(mouse_pressed) of each light
-boolean disconnected : holds the state of connection[true/false] of each light
-Color color : holds the color data for each light
}
*/

public class Light extends JButton {

    /*Data*/
    private String label;
    private Light_Info info;
    private Light_Info2 info2;
    private boolean state = false;
    private boolean pressed_state = false;
    private boolean hover_state = false;
    private boolean disconnected = false;
    private boolean selected = false;
    private Color color;
    private int x=-1;
    private int y =-1;
	/* */

    /*Constructor() #1*/
	/*Label : Light's label
	/*Color color : Light's color
	/*light_type : {regular , rgb }
	*/
    public Light(String label, Color color, String light_type) {
        super();
        this.label = label;
        this.color = color;
        this.info = new Light_Info(label, light_type); //constructing Light's info panel
        this.info2 = new Light_Info2();

        reset();

    }

    /*Constructor() #2*/
	/*Label : Light's label
	/*light_type : {regular , rgb }
	*/
    public Light(String label, String light_type) {
        super();
        this.label = label;
        this.color = Color.white;
        this.info = new Light_Info(label, light_type); //constructing Light's info panel
        this.info2 = new Light_Info2();

        reset();
    }

    /*Constructor() #3*/
	/*light_type : {regular , rgb }
	*/
    public Light(String light_type) {
        super();
        this.color = Color.white;
        this.info = new Light_Info("Default", light_type); //constructing Light's info panel
        this.info2 = new Light_Info2();

        reset();
    }

    /*Method that reset's Light's states to default*/
    public void reset() {
        this.setBackground(Color.black);
        this.setBorder(BorderFactory.createLineBorder(Color.red, 2));
        this.state = false;
        this.hover_state = false;
        this.pressed_state = false;
        this.disconnected = false;
        this.info.set_State(false);

    }

    /*Method that returns Light's current state[on/off]*/
    public boolean get_State() {
        return this.state;
    }

    /*Method that returns Light's current Background_Color*/
    public Color get_Color() {
        return this.color;
    }


    /*Method that switches Light's state and updated Light's GUI if boolean update is[true/false]*/
	/*boolean update indicates when do we actually need to change light's gui ,instead of  just chaning it's state ,
	in case main_switch is : [off] , update should be [true] because light is getting powered ,in case main switch is : [on]
	we only need to change light's state to [on/off] instead of updating it's gui also . Gui should only be updated if
	the light is being powered .
	*/
    public void swap(boolean update) {

        this.state = !this.state;
        this.info.set_State(this.state);
        if (this.isConnected()) {
            if (!this.state && update) {
                this.setBackground(Color.black);
                this.setBorder(BorderFactory.createLineBorder(Color.red, 2));
            }
            else if (state && update) {
                this.setBackground(color);
                this.setBorder(BorderFactory.createLineBorder(Color.green));
            }
        }

    }

    /*Method that updates only light's GUI to [OFF] state but not it's internal state */
	/*it's being used when main_switch is turning to : [on] , so we can simulate the light's power going off.
	*/
    public void turn_off() {
        this.setBackground(Color.black);
        this.setBorder(BorderFactory.createLineBorder(Color.red, 2));
    }


    /*Method that check's the internal state of Light and updates it's GUI accordingly*/
    public void update() {
        if (this.isConnected()) {

            if (!state) {
                this.setBackground(Color.black);
                this.setBorder(BorderFactory.createLineBorder(Color.red, 2));

            }
            else {
                this.setBackground(color);
                this.setBorder(BorderFactory.createLineBorder(Color.green, 2));

            }

        }

    }


    /*Method that returns Light's internal info_panel*/
    public Light_Info get_info_panel() {
        return this.info;
    }
    public Light_Info2 get_info2_panel(){return this.info2;}


    /*Method that sets the Light to Pressed state[true/false]
    and changes it's GUI accordingly
    */
    public void setPressed(boolean state) {
        pressed_state = state;
        if (state) {
            this.setBorder(BorderFactory.createLineBorder(Color.white));
            this.info.setBorder(BorderFactory.createLineBorder(Color.white));
        }
        else {
            if (this.state && this.isConnected()) {
                this.setBorder(BorderFactory.createLineBorder(Color.green, 2));
            }
            else {
                this.setBorder(BorderFactory.createLineBorder(Color.red, 2));
            }
            this.info.setBorder(null);
        }

    }

    /*Method that sets the Light to Hover state[true/false]
    and changes it's GUI accordingly
    */
    public void setHover(boolean state) {
        this.hover_state = state;



        if (state) {
            //this.info2.setVisible(true);
            //this.setBorder(BorderFactory.createLineBorder(Color.lightGray));
            this.info.setBorder(BorderFactory.createLineBorder(Color.lightGray));
            this.info.setBackground(new Color(47,52,50));
        }
        else {
            //this.info2.setVisible(false);
            this.info.setBackground(Color.darkGray);

            if (this.state && this.isConnected()) {

                //this.setBorder(BorderFactory.createLineBorder(Color.green, 2));
            }
            else {

                //this.setBorder(BorderFactory.createLineBorder(Color.red, 2));
            }

            if (!isPressed()) {
                this.info.setBorder(null);
            }
            else {

                //this.setBorder(BorderFactory.createLineBorder(Color.white));
                this.info.setBorder(BorderFactory.createLineBorder(Color.white));
            }

        }
    }

    public boolean isHover(){
        return this.hover_state;
    }

    /*Method that returns the Pressed_state of the Light*/
    public boolean isPressed() {
        return this.pressed_state;
    }

    /*Method that returns the label of the Light*/
    public String get_Label() {
        return this.label;
    }

    /*Method that returns the Light's disconnected state*/
    public boolean isConnected() {
        return !this.disconnected;
    }

    /*Method that 'disconnects/connects' the Light from it's source power*/
    public void swap_connection() {
        this.disconnected = !this.disconnected;
    }


    public void set_Label(String label) {
        this.label = label;
        this.info.set_Label(this.label);

    }

    public void set_Color(Color c) {
        this.color = c;
    }

    public void setX(int x){
        this.x = x;
    }

    public void setY(int y){
        this.y = y;
    }

    public int getXX(){
        return this.x;
    }

    public int getYY(){
        return this.y;
    }

    public boolean is_Selected(){
        return this.selected;
    }

    public void Set_Selected(boolean s){
        this.selected = s;
    }

}


