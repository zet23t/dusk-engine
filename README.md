# Dusk Engine

Dusk Engine aims to be a simple 3D engine for games based on raylib. Vision goals:

* Similar architectural pillars as original Unity engine: Gameobjects and components - not ECS
* Pure C codebase
* Simple rendering pipeline, not going to compete with other 3D engines
* Focus on workflow for making games, following Unity's footsteps
* Target platforms: Windows, Android, Web. Linux and Mac are up to others, I don't have those platforms to develop and test on.

# Overview

The core of the engine is the scene graph, which is a tree of scene objects. Each scene object can have components attached to it. Components are the building blocks of the game logic. The engine provides a set of built-in components, such as Transform, Camera, MeshRenderer, etc. Projects can also create your own components by registering structs via the scene graph API.

Serialization is done via JSON. Structs must be declared via dedicated macros to be serializable. 

# TODOs

Miro board link for planning: https://miro.com/app/board/uXjVM0NWYxU=/#tpicker-content 

# Vision

This is how I imagine it to be at _some point_ in the future:

The editor is started with a project path as an argument. The editor will look for a `dusk-project.json` file in the project path that contains the project's configuration. 

## Structures

* Assets: Contains the assets of the project, such as textures, models, etc.
* assembly.c files: All files with this names are sequentially included (in whatever order) and compiled into the project. This is where the game logic is written. The assembly.c file includes all .c files it wants to have compiled.
* .json files: Assets are serialized in general to JSON files. 

