
# ABOUT

# Project: Distributed System over local Network
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
- <b>Purpose</b> : University Project.

