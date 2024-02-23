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
    SceneComponentTypeId shootingComponentId;
    SceneComponentTypeId autoDestroyComponentId;

    Model model;
    Mesh* meshPlane;
    Mesh* meshPropellerPin;
    Mesh* meshPropellerBlade;
    Mesh* meshPlayerBullet;

    SceneObjectId playerPlane;
} PSG;

typedef struct ShootingConfig {
    float shotInterval;
    float bulletSpeed;
    float bulletLifetime;
    void (*onShoot)(SceneGraph*, SceneComponentId shooter, struct ShootingComponent*, struct ShootingConfig* shootingComponent);
} ShootingConfig;

typedef struct AutoDestroyComponent
{
    float lifeTimeLeft;
} AutoDestroyComponent;

typedef struct ShootingComponent {
    float cooldown;
    int32_t shooting;
    ShootingConfig config;
    SceneObjectId spawnPoint;
} ShootingComponent;

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