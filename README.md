# Projects Preview



# 1. Degree Thesis
- <b>Programming Language</b> : C++
- <b>APIs used</b> : <a href="https://developer.nvidia.com/optix">Nvidia Optix</a>, <a href="https://www.opengl.org//">OpenGL</a>, <a href="https://github.com/ocornut/imgui">DearImGui</a>
- <b>Subject</b> : 3D scene design software, focused mainly on the interior spatial domain and the furniture layout arrangement problem. Supports a variety of object manipulation and relational utilities(essential transformations, parent-child relations, groups, etc.). Most importantly, provides the user with a set of specific spatial,functional and illumination constraints, addressing a more constraint-driven designing process. Lastly, implements a physical based path-tracer rendering pipeline, via the NVidia OptiX API.
- <b>More Info</b> : https://github.com/GMousouris/Projects_preview/tree/old_state/C%2B%2B/Degree%20Project

![GitHub Logo](/Images/degree_project_preview.png)







# 2. Tower Defense Game
- <b>Programming Language</b> : C++
- <b>APIs used</b> : OpenGL, irrKlang Lib for audio handling
- <b>Subject</b> : A 3D Tower Defense game whose enemies are some pirates trying to reach the end of each map and get the Gold! The player tries to stop them pirates by putting defense towers in key-point positions and using 2 extra super powers , the Satelite Super Power or the Meteor one. If the whole gold gets stolen from the treasure chest before the end of the map, the player loses! 
- <b>More Info</b> : https://github.com/GMousouris/Projects_preview/tree/master/C%2B%2B/3d%20TD%20Game

![GitHub Logo](/Images/TD_game_preview.png)








# 3. Server-Workers Distributed System
- <b>Programming Language</b> : Java
- <b>APIs used</b> : Swing package, java.Net package, org.apache.commons.math3 package.
- <b>Subject</b> : A local-network, multi-threaded distributed-system based on server-client model, responsible for dynamically distributing partial-amount of work over a set of workers(clients) depending on each worker's system capabilities and workload, at runtime.
Application's input is a NxM matrix consisting of N:places of interest(POIs) and M:customers, where each [i,j]entry is {0,1} if customer j has visited POI i; And a set of user-visited POIs = {POI1,POI2, ... POIk} where POIi e [0,N-1].
Application's output is a set of suggested POIs calculated by applying 'Least Squares Linear Regression' to the input-data.
At the program initialization, a set of {W1,W2,...,Wk} client-workers is constructed, and a communication-pipeline is established via creating and maintaining a thread for each one of them server-side, and transferring packets via the java.Net framework.
On each frame, a heavy cpu-side computation has to be completed consisting mostly of matrix multiplications. In order to exploit the server-client model,
the Server communicates with the workers, and requests for information relative to each worker's capabalities and workload at that time. After all workers have answered back, the total amount of work A, gets segmented to {A1,A2,..,Ak} parts accordingly. Then each work-part Ai, gets distributed to each worker Wi, and compuation takes place client-side concurrently, while server is being in a 'stand-by' state. It's common that some workers will complete their work-part earlier than others, in that case, server retrieves the most work-burdened workers, and eases their workload by re-distributing part of their work to the workers that have already finished theirs. By doing that dynamically every frame, the time needed to compute the total work gets depleted substantially. 
When the frame's total amout of work has been completed, server is responsible for gathering and joining all the work-parts {Wk} in to a single format, and continue to the next frame until a pre-defined minimum treshold has been reached.
Finally, server, outputs a set of suggested POIs to the user, respective to the input data.
- <b>More Info</b> : https://github.com/GMousouris/Projects_preview/tree/master/Java/Server_Workers%20Distributed%20System






# 4. Reversi
- <b>Programming Language</b> : Java
- <b>APIs used</b> : Swing Package
- <b>Subject</b> : A desktop application implementing the ‘Reversi’ board game. The user plays vs. the computer
at different levels of difficulty. The computer chooses the ‘best’ action each turn  by using a ‘MinMax’ based prediction algorithm. Given higher levels of difficulty, computer traverses deeper the action graph, thus getting better decisions.
- <b>More Info</b> : University Project.
- <b>Repo</b> : https://github.com/GMousouris/Projects_preview/tree/master/Java/Reversi%20Board%20Game

![GitHub Logo](/Images/reversi_preview.png)







# 5. Email Classifier
- <b>Programming Language</b> : Java
- <b>APIs used</b> : 
- <b>Subject</b> : An email classification-predicting  application, based on Naïve Bayes / ID3 algorithm approaches
- <b>More Info</b> : https://github.com/GMousouris/Projects_preview/tree/master/Java/Email_Classification







# 6. Dictionary
- <b>Programming Language</b> : Java
- <b>APIs used</b> : Swing Package
- <b>Subject</b> : A virtual dictionary for keeping stored words, their synonyms and translations, with whatever notes you want. Supports loading / saving , searching, and viewing some simple statistics.
- <b>More Info</b> : https://github.com/GMousouris/Projects_preview/tree/master/Java/Dictionary

![GitHub Logo](/Images/Dictionairy_preview.png)







# 7. Task Scheduler
- <b>Programming Language</b> : Java
- <b>APIs used</b> : Swing Package
- <b>Subject</b> : A Task Scheduler, not finished, responsible for keeping in track your to-do list, notes, exam dates, etc.
- <b>More Info</b> : https://github.com/GMousouris/Projects_preview/tree/master/Java/Task%20Scheduler

![GitHub Logo](/Images/TaskSchedulr_preview.png)







# 8. Room Light-Network Simulation
- <b>Programming Language</b> : Java
- <b>APIs used</b> : Swing Package
- <b>Subject</b> : A room Light-Network controller simulation environment, support of : adding/removing lights, moving them, toggling their state (on/off), connecting / disconnecting them from the main source, etc.
- <b>More Info</b> : https://github.com/GMousouris/Projects_preview/tree/master/Java/Light%20Network

![GitHub Logo](/Images/LightNetwork_preview.png)
