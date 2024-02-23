#include "config.h"
#include "math.h"
#include "shared/scene_graph/scene_graph.h"

#include <string.h>

static SceneGraph* sceneGraph;
static Model model;
static Mesh* meshPlane;
static Mesh* meshPropellerPin;
static Mesh* meshPropellerBlade;

int load_meshes()
{
    const char* gltfMeshFile = "assets/meshes.glb";

    model = LoadModel(gltfMeshFile);
    const char* meshNamePlane = "fighter-plane-1";
    meshPlane = NULL;
    for (int i = 0; i < model.meshCount; i++) {
        if (strcmp(model.meshes[i].name, meshNamePlane) == 0) {
            meshPlane = &model.meshes[i];
        }
        if (strcmp(model.meshes[i].name, "propeller") == 0) {
            meshPropellerPin = &model.meshes[i];
        }
        if (strcmp(model.meshes[i].name, "propeller-blade-1") == 0) {
            meshPropellerBlade = &model.meshes[i];
        }
    }

    if (meshPlane == NULL || meshPropellerPin == NULL || meshPropellerBlade == NULL) {
        TraceLog(LOG_ERROR, "Mesh not found: %s\n", meshNamePlane);
        return 1;
    }

    for (int i = 0; i < model.materialCount; i++) {
        for (int j = 0; j < MAX_MATERIAL_MAPS; j++) {
            if (model.materials[i].maps[j].texture.id == 0) {
                continue;
            }
            SetTextureFilter(model.materials[i].maps[j].texture, TEXTURE_FILTER_BILINEAR);
        }
    }

    return 0;
}

typedef struct MeshRendererComponent {
    Material material;
    Mesh* mesh;
} MeshRendererComponent;

void MeshRendererDraw(Camera3D camera, SceneObject* node, SceneComponentId sceneComponentId, void* component, void* userdata)
{
    MeshRendererComponent* meshRenderer = (MeshRendererComponent*)component;
    Matrix m = SceneObject_getWorldMatrix(node);
    DrawMesh(*meshRenderer->mesh, meshRenderer->material, m);
}

typedef struct PlaneBehaviorComponent {
    float phase;
    float time;
    SceneObjectId propeller;
} PlaneBehaviorComponent;

void PlaneBehaviorComponentUpdateTick(SceneObject* sceneObject, SceneComponentId sceneComponentId,
    float delta, void* componentData)
{
    PlaneBehaviorComponent* planeBehavior = (PlaneBehaviorComponent*)componentData;
    planeBehavior->time += delta;

    float yaw = sinf(planeBehavior->time * 3 + planeBehavior->phase * .5f);
    float roll = sinf(planeBehavior->time * 2.3f + planeBehavior->phase) + yaw * .25f;
    float pitch = sinf(planeBehavior->time * 2.7f + planeBehavior->phase * .7f) + yaw * .5f;
    SceneGraph_setLocalRotation(sceneObject->graph, sceneObject->id, (Vector3) { pitch * 3, yaw * 2, roll * 5 });
    SceneGraph_setLocalRotation(sceneGraph, planeBehavior->propeller, (Vector3) { 0, 0, -planeBehavior->time * 360 * 4 });
}

static SceneComponentTypeId meshRendererComponentId;
static SceneComponentTypeId planeBehaviorComponentId;

SceneObjectId plane_instantiate(Vector3 position)
{

    SceneObjectId plane = SceneGraph_createObject(sceneGraph, "plane");
    SceneGraph_setLocalPosition(sceneGraph, plane, position);
    SceneGraph_addComponent(sceneGraph, plane, meshRendererComponentId,
        &(MeshRendererComponent) {
            .material = model.materials[1],
            .mesh = meshPlane,
        });

    SceneObjectId propeller = SceneGraph_createObject(sceneGraph, "propeller");
    SceneGraph_setParent(sceneGraph, propeller, plane);
    SceneGraph_setLocalPosition(sceneGraph, propeller, (Vector3) { 0, 0.062696f, 0.795618f });
    SceneGraph_addComponent(sceneGraph, propeller, meshRendererComponentId,
        &(MeshRendererComponent) {
            .material = model.materials[1],
            .mesh = meshPropellerPin,
        });

    for (int i = 0; i < 3; i += 1) {
        SceneObjectId propellerBlade = SceneGraph_createObject(sceneGraph, "propeller-blade");
        SceneGraph_setParent(sceneGraph, propellerBlade, propeller);
        SceneGraph_setLocalPosition(sceneGraph, propellerBlade, (Vector3) { 0, 0, 0 });
        SceneGraph_setLocalRotation(sceneGraph, propellerBlade, (Vector3) { 0, 0, 120 * i });
        SceneGraph_addComponent(sceneGraph, propellerBlade, meshRendererComponentId,
            &(MeshRendererComponent) {
                .material = model.materials[1],
                .mesh = meshPropellerBlade,
            });
    }

    SceneGraph_addComponent(sceneGraph, plane, planeBehaviorComponentId,
        &(PlaneBehaviorComponent) {
            .time = 0,
            .propeller = propeller,
            .phase = GetRandomValue(0, 1000) });

    return plane;
}

int plane_sim_init()
{
    if (load_meshes()) {
        return 1;
    }

    sceneGraph = SceneGraph_create();
    meshRendererComponentId = SceneGraph_registerComponentType(sceneGraph, "MeshRenderer", sizeof(MeshRendererComponent),
        (SceneComponentTypeMethods) {
            .draw = MeshRendererDraw,
        });

    planeBehaviorComponentId = SceneGraph_registerComponentType(sceneGraph, "PlaneBehavior", sizeof(PlaneBehaviorComponent),
        (SceneComponentTypeMethods) {
            .updateTick = PlaneBehaviorComponentUpdateTick,
        });

    for (int i = 0; i < 1000; i += 1) {
        plane_instantiate((Vector3) {
            GetRandomValue(-50, 50) * .5f,
            GetRandomValue(-20, 20) * .5f,
            GetRandomValue(-50, 50) * .5f });
    }
    // plane_instantiate((Vector3) { -2.5f, 0, 0 });
    // plane_instantiate((Vector3) { 0, 0, 1.5f });
    // plane_instantiate((Vector3) { 2.5f, 0, 0 });

    return 0;
}

void plane_sim_draw()
{
    Camera3D camera = { 0 };
    camera.position = (Vector3) { 15, 10, 15 };
    camera.target = (Vector3) { 0, 0, 0 };
    camera.up = (Vector3) { 0, 1, 0 };
    camera.fovy = 45;
    camera.far = 1000;
    camera.near = 0.1f;

    BeginMode3D(camera);
    SceneGraph_draw(sceneGraph, camera, NULL);
    EndMode3D();
}

static float t = 0;
void plane_sim_update(float dt)
{
    // SceneGraph_setLocalRotation(sceneGraph, plane, (Vector3) { 0, t * 30, 0 });
    t += dt;
    SceneGraph_updateTick(sceneGraph, dt);
}