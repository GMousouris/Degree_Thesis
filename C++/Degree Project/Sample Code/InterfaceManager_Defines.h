#pragma once
#define UTILITY_ISEC_P_REPROJECT_TO_CENTER

#define OBJECT_SELECT_ON_MOUSE_PRESS
//#define OBJECT_SELECT_ON_MOUSE_RELEASE

#define ARBITRARY_LINKING_ON_MOUSE_PRESS



#define DEFAULT_VIEWPORT
//#define CUSTOM_VIEWPORT_TEST


#ifdef I_DEFAULT_VIEWPORT
    #define I_DEFAULT_VIEWPORT 1
    #define I_CUSTOM_VIEWPORT_TEST 0
#else
    #define I_CUSTOM_VIEWPORT_TEST 1
    #define I_DEFAULT_VIEWPORT 0
#endif


#define TAB_FONT_SIZE 16
#define API_FONT_SIZE 14


#define edit_res_tol_var_in_menu


//#define TOOLS_HELPERS_BAR


//#define SHOW_FVS_WHEN_G_RESTRICTIONS // define if want to show FaceVectors when attaching Restrictions..