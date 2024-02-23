#include "config.h"
#include "math.h"
#include "shared/scene_graph/scene_graph.h"

#include "plane_sim_g.h"

#include <string.h>

int load_meshes()
{
    const char* gltfMeshFile = "assets/meshes.glb";

    psg.model = LoadModel(gltfMeshFile);
    const char* meshNamePlane = "fighter-plane-1";
    psg.meshPlane = NULL;
    for (int i = 0; i < psg.model.meshCount; i++) {
        if (strcmp(psg.model.meshes[i].name, meshNamePlane) == 0) {
            psg.meshPlane = &psg.model.meshes[i];
        }
        if (strcmp(psg.model.meshes[i].name, "propeller") == 0) {
            psg.meshPropellerPin = &psg.model.meshes[i];
        }
        if (strcmp(psg.model.meshes[i].name, "propeller-blade-1") == 0) {
            psg.meshPropellerBlade = &psg.model.meshes[i];
        }
    }

    if (psg.meshPlane == NULL || psg.meshPropellerPin == NULL || psg.meshPropellerBlade == NULL) {
        TraceLog(LOG_ERROR, "Mesh not found: %s\n", meshNamePlane);
        return 1;
    }

    for (int i = 0; i < psg.model.materialCount; i++) {
        for (int j = 0; j < MAX_MATERIAL_MAPS; j++) {
            if (psg.model.materials[i].maps[j].texture.id == 0) {
                continue;
            }
            SetTextureFilter(psg.model.materials[i].maps[j].texture, TEXTURE_FILTER_BILINEAR);
        }
    }

    return 0;
}

SceneObjectId plane_instantiate(Vector3 position)
{

    SceneObjectId plane = SceneGraph_createObject(psg.sceneGraph, "plane");
    SceneGraph_setLocalPosition(psg.sceneGraph, plane, position);
    SceneGraph_addComponent(psg.sceneGraph, plane, psg.meshRendererComponentId,
        &(MeshRendererComponent) {
            .material = psg.model.materials[1],
            .mesh = psg.meshPlane,
        });

    SceneObjectId propeller = SceneGraph_createObject(psg.sceneGraph, "propeller");
    SceneGraph_setParent(psg.sceneGraph, propeller, plane);
    SceneGraph_setLocalPosition(psg.sceneGraph, propeller, (Vector3) { 0, 0.062696f, 0.795618f });
    SceneGraph_addComponent(psg.sceneGraph, propeller, psg.meshRendererComponentId,
        &(MeshRendererComponent) {
            .material = psg.model.materials[1],
            .mesh = psg.meshPropellerPin,
        });

    for (int i = 0; i < 3; i += 1) {
        SceneObjectId propellerBlade = SceneGraph_createObject(psg.sceneGraph, "propeller-blade");
        SceneGraph_setParent(psg.sceneGraph, propellerBlade, propeller);
        SceneGraph_setLocalPosition(psg.sceneGraph, propellerBlade, (Vector3) { 0, 0, 0 });
        SceneGraph_setLocalRotation(psg.sceneGraph, propellerBlade, (Vector3) { 0, 0, 120 * i });
        SceneGraph_addComponent(psg.sceneGraph, propellerBlade, psg.meshRendererComponentId,
            &(MeshRendererComponent) {
                .material = psg.model.materials[1],
                .mesh = psg.meshPropellerBlade,
            });
    }

    SceneGraph_addComponent(psg.sceneGraph, plane, psg.planeBehaviorComponentId,
        &(PlaneBehaviorComponent) {
            .phase = GetRandomValue(0, 1000),
            .rolling = 0,
            .time = 0,
            .prevVelocity = (Vector3) { 0, 0, 0 },
            .propeller = propeller,
        });

    const float drag = 5.0f;
    SceneGraph_addComponent(psg.sceneGraph, plane, psg.linearVelocityComponentId,
        &(LinearVelocityComponent) {
            .velocity = (Vector3) { 1, 0, 0 },
            .drag = (Vector3) { drag, drag, drag } });

    return plane;
}

extern void MeshRendererComponentRegister();
extern void PlaneBehaviorComponentRegister();
extern void LinearVelocityComponentRegister();

int plane_sim_init()
{
    if (load_meshes()) {
        return 1;
    }

    psg.sceneGraph = SceneGraph_create();

    MeshRendererComponentRegister();
    PlaneBehaviorComponentRegister();
    LinearVelocityComponentRegister();

    // for (int i = 0; i < 1000; i += 1) {
    //     plane_instantiate((Vector3) {
    //         GetRandomValue(-50, 50) * .5f,
    //         GetRandomValue(-20, 20) * .5f,
    //         GetRandomValue(-50, 50) * .5f });
    // }
    psg.playerPlane = plane_instantiate((Vector3) { 0, 0, 0 });
    // plane_instantiate((Vector3) { 0, 0, 1.5f });
    // plane_instantiate((Vector3) { 2.5f, 0, 0 });

    return 0;
}

static int textIndex = 0;
void plane_sim_draw()
{
    Camera3D camera = { 0 };
    camera.position = (Vector3) { 0, 7, -10 };
    camera.target = (Vector3) { 0, 0, 0 };
    camera.up = (Vector3) { 0, 1, 0 };
    camera.fovy = 45;
    camera.far = 1000;
    camera.near = 0.1f;

    BeginMode3D(camera);
    SceneGraph_draw(psg.sceneGraph, camera, NULL);
    EndMode3D();

    if (IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        int width = GetScreenWidth();
        int height = GetScreenHeight();
        const char *texts[] = {
            "Ratatatta",
            "Pew pew",
            "Bang bang",
        };
        const char *buffer = texts[textIndex % 3];
        if (GetRandomValue(0,5) == 0)
        {
            textIndex++;
        }
        int textWidth = MeasureText(buffer, 20);
        int offsetX = GetRandomValue(-3, 3);
        int offsetY = GetRandomValue(-3, 3);
        int x = (width - textWidth) / 2 + offsetX;
        int y = height - 40 + offsetY;
        DrawText(buffer, x+2, y+2, 20, BLACK);
        DrawText(buffer, x, y, 20, WHITE);
        
    }
}

void HandlePlayerInputUpdate();

void plane_sim_update(float dt)
{
    psg.time += dt;
    psg.deltaTime = dt;
    SceneGraph_updateTick(psg.sceneGraph, dt);
    HandlePlayerInputUpdate();
}