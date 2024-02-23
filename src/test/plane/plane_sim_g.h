#ifndef __PLANE_SIM_G_H__
#define __PLANE_SIM_G_H__

#include "shared/scene_graph/scene_graph.h"

typedef struct PSG {
    float time;
    float deltaTime; 

    SceneGraph* sceneGraph;
    
    SceneComponentTypeId meshRendererComponentId;
    SceneComponentTypeId planeBehaviorComponentId;
    SceneComponentTypeId linearVelocityComponentId;

    Model model;
    Mesh* meshPlane;
    Mesh* meshPropellerPin;
    Mesh* meshPropellerBlade;

    SceneObjectId playerPlane;
} PSG;

typedef struct MeshRendererComponent {
    Material material;
    Mesh* mesh;
} MeshRendererComponent;

typedef struct PlaneBehaviorComponent {
    float phase;
    float rolling;
    float smoothedXAccel;
    float time;
    Vector3 prevVelocity;
    SceneObjectId propeller;
    SceneComponentId velocityComponentId;
} PlaneBehaviorComponent;

typedef struct LinearVelocityComponent {
    Vector3 velocity;
    Vector3 drag;
} LinearVelocityComponent;

extern PSG psg;

#endif