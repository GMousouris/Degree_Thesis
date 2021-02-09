# ABOUT

# Project: Degree Thesis
- <b>Programming Language</b> : C++
- <b>APIs used</b> : <a href = "https://developer.nvidia.com/optix">Nvidia Optix</a>, <a href="https://www.opengl.org//">OpenGL</a>, <a href="https://github.com/ocornut/imgui">DearImGui</a>
- <b>Subject</b> : 3D scene design software, focused mainly on the interior spatial domain and the furniture layout arrangement problem. Supports a variety of object manipulation and relational utilities(essential transformations, parent-child relations, groups, etc.). Most importantly, provides the user with a set of specific spatial,functional and illumination constraints, addressing a more constraint-driven designing process. Lastly, implements a physical based path-tracer rendering pipeline, via the NVidia OptiX API.
- <b> Development Specifications </b> : Software's system-pipeline designed on C++. 
GUI designed using <a href ="https://github.com/ocornut/imgui">DearImGui</a> library in combination with <a href="https://www.glfw.org">GLFW</a>.
Default scene-rendering with <a href="https://www.opengl.org//">OpenGL</a> and optional PBR via <a href="https://developer.nvidia.com/optix">NVidia OptiX</a> pipeline. 
- <b>Hardware Dependencies</b> : GPU compatible with NVidia OptiX 6.0 or later version.


- <b> Software Specifications </b> : 

<b> Main-menu navigation: </b>
> Main-menu provides the user with I/O processes such as : { Import Object(currently supporting only '.obj' files), Import Scene, Save Scene, Export Scene(as '.json' format) }.
Additionally, covers most of the tools that also appear in the 'tools-bar', with some exceptions, and some unique one tools-actions, such as the Group category.

<b> Tools-bar: </b>
> User has control over a variety of GUI tools; which are ordered as groups. Each tool-group specifies a family of actions that take place upon an object or not; most common of them, being the Action-group{undo,redo}, Linking-group{Link,Unlink}, Transformation-group{Translate,Rotate,Scale}. Moreover, specific actions like {Group,Ungroup} can only be found by navigating over the main-menu bar.
Object transformations can be manipulated in a variety of ways, either by controlling directly the transformation-gizmo located at the selected object in the scene, or by navigating over the Transformations-tab at the right panel, or even via the bottom-panel inputs. A selection of the tranformation's 'axis' or the 'orthonormal-basis' is supported between {world-oriented,local and screen-axis}, in addition with an hierarchy-setting that indicates how the transformation will affect object's hierarchy.
The tool-group responsible for defining any of the supported object-constraints is reachable via the toolbar, or the main-menu bar.
Finally, at the right-most end, a set of tools act as a shortcut for Scene-Rendering configuration profiles(toggling over default-Rendering/PBR), and toggling user-defined lighting.

<b> GUI Window-Panels: </b>
> There are 3 main gui-panels{Left,Right,Bottom} displayed on the screen, each one responsible for different actions.
<b>Left-panel</b> acts as the main scene-explorer, representing scene as a tree data-structure, where each node refers to an object. User can navigate through the scene-explorer, and select any object or its attachment(facevector, light-sampler). Moreover, a set of buttons appear in a vertical-alignement along the left part of the Left-Panel, which control the visibility-state per object-type of the scene-explorer.
<b>Right-Panel</b> displays a set of tabs{General,Transformation,Materials,Relations,Constraints,PBR-settings,Scene-settings}; Eachone of them, provides relative information and a set of usually-object-dependent data-fields. Therefore, each tab gives access to a specific list of data relative to an object(mainly, the selected one) in most cases.
<b>Bottom-Panel</b> displays general information of the scene and the selected-object(if any), and gives immediate access to transformation-input fields. 
<b>Auxillary-Panels</b> appear when specific actions take place, e.g when importing an object,import-exporting scene,cloning an object, creating a group, aligning an object; and mainly provide detailed configuration over the action, or just act as approval popup-windows.

<b> Scene: </b>
> The scene is the final product of the selected rendering-pipeline{OpenGL, NVidia-OptiX}, and acts as the main "guide" for the user. Eventhough there can only processed one scene at a time, there is supported a set of <b>viewport-layouts</b>, that split and re-arrange the main rendering window into multiple rendering-windows or viewports; eachone of the viewports serving as an independent scene. Each <b>viewport</b>, supports unique rendering options, camera orientation, and other configurations, via buttons at its upper-left corner; and a camera-handle button on its upper-right corner. Thus, the user, can easily have a viewport-layout consisting of a main viewport, and a set of smaller, supportive viewports with different settings according to his liking. 

<b> Scene Navigation: </b>
> Each viewport-scene controls a <b>camera-object</b>, that can freely roam over the scene boundaries. Camera-object implements a <b>pinhole camera</b>, with a pre-defined field-of-view(fov) and movement-behavior. Movement-behavior is based on an orbiting-model relative to a reference-point in the 3D-scene, the camera's <b>orbiting-center</b>. By default, the orbiting-center is set to the scene's origin((x,y,z) = (0,0,0)), user defines a new orbiting-center every time he selects an object- either directly from the scene or via the scene-explorer, -and the selected-object's centroid lays inside the viewport's frustum boundaries. Except of <b>orbiting</b>; <b>zooming</b> and <b>panning</b> are also supported move-actions; Where <b>zooming</b> alters camera's position along the axis defined by the mouse-pointer and the camera-position, and <b>panning</b> moves camera's position along the plane perpendicular to its Z-axis(optical or looking-at axis).

<b>Camera Contros: </b>
>  - <b>Orbit</b> : Left_mouseButton{pressed} + mouse_delta
>  - <b>Zoom</b> : Middle_mouseButton{delta}
>  - <b>Pan</b> : Middle_mouseButton{pressed} + mouse_delta



![GitHub Logo](README_IMG/degree_project_preview.png)

#

- <b> Presenting some common user-cases </b>
> <b> Loading some objects into the scene </b>

![GitHub Logo](README_IMG/import.png)


> <b> Creating a Light Object </b>

![GitHub Logo](README_IMG/creating_light.png)


> <b> Defining a Facevector </b>

![GitHub Logo](README_IMG/facevector.png)


> <b> Defining a Constraint between two objects </b>

![GitHub Logo](README_IMG/restriction.png)


> <b> Transforming an object </b>

![GitHub Logo](README_IMG/trs.png)


