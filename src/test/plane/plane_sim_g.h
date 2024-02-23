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
    SceneComponentTypeId bulletComponentId;
    SceneComponentTypeId targetComponentId;

    Model model;
    Mesh* meshPlane;
    Mesh* meshPropellerPin;
    Mesh* meshPropellerBlade;
    Mesh* meshPlayerBullet;
    Mesh* meshTarget;

    SceneObjectId playerPlane;
} PSG;

typedef struct ShootingConfig ShootingConfig;
typedef struct ShootingComponent ShootingComponent;

typedef void (*OnShootFn)(SceneGraph*, SceneComponentId shooter, ShootingComponent*, ShootingConfig* shootingComponent);

typedef struct ShootingConfig {
    float shotInterval;
    float bulletSpeed;
    float bulletLifetime;
    OnShootFn onShoot;
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

typedef struct BulletComponent {
    float radius;
    uint32_t colliderFlags;
} BulletComponent;

typedef int (*OnHit)(SceneGraph*, SceneObjectId target, SceneObjectId bullet);
typedef struct TargetComponent {
    float radius;
    uint32_t colliderMask;
    OnHit onHit;
} TargetComponent;

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