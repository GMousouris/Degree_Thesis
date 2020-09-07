




struct GLFWwindow;

bool        ImGui_ImplGlfwGL2_Init(GLFWwindow* window, bool install_callbacks);
void        ImGui_ImplGlfwGL2_Shutdown();
void        ImGui_ImplGlfwGL2_NewFrame();
void        ImGui_ImplGlfwGL2_RenderDrawData(ImDrawData* draw_data);

// Use if you want to reset your rendering device without losing ImGui state.
void        ImGui_ImplGlfwGL2_InvalidateDeviceObjects();
bool        ImGui_ImplGlfwGL2_CreateDeviceObjects();

// GLFW callbacks (registered by default to GLFW if you enable 'install_callbacks' during initialization)
// Provided here if you want to chain callbacks yourself. You may also handle inputs yourself and use those as a reference.
void        ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void        ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void        ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void        ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);

