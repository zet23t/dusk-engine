#include "../../game_g.h"
#include "../../game_state_level.h"

#define DUSK_GUI_IMPLEMENTATION
#include "dusk-gui.h"

void ObjectConfiguratorEditorComponent_init(SceneObject* sceneObject, SceneComponentId SceneComponent, void* componentData, void* initArg)
{
    Font font = ResourceManager_loadFont(&psg.resourceManager, "assets/myfont-regular.png");
    

    SceneObjectId camera = SceneGraph_createObject(psg.sceneGraph, "Camera");
    psg.camera = camera;
    SceneGraph_addComponent(psg.sceneGraph, camera, psg.cameraComponentId, &(CameraComponent) {
        .fov = 45,
        .nearPlane = 0.1f,
        .farPlane = 1000,
    });
    SceneGraph_setLocalPosition(psg.sceneGraph, camera, (Vector3) { 0, 0, -10 });
    SceneObjectId cameraPivotYaw = SceneGraph_createObject(psg.sceneGraph, "CameraPivotYaw");
    SceneObjectId cameraPivotPitch = SceneGraph_createObject(psg.sceneGraph, "CameraPivotPitch");
    SceneGraph_setParent(psg.sceneGraph, camera, cameraPivotPitch);
    SceneGraph_setParent(psg.sceneGraph, cameraPivotPitch, cameraPivotYaw);
    SceneGraph_setLocalRotation(psg.sceneGraph, cameraPivotPitch, (Vector3) { 30, 0, 0 });
    
    // SceneObjectId plane = SceneGraph_createObject(psg.sceneGraph, "Plane");
    // AddMeshRendererComponentByName(plane, "fighter-plane-1", 1.0f);
    SpawnEnemy(psg.sceneGraph, 0, 0, (EnemyBehaviorComponent) {0});


    ObjectConfiguratorEditorComponent *data = (ObjectConfiguratorEditorComponent*)componentData;
    data->cameraPivotYawId = cameraPivotYaw;
    data->cameraPivotPitchId = cameraPivotPitch;

    DuskGui_init();
    DuskGui_setDefaultFont(font, font.baseSize, 1, BLACK, (Color){16,16,32,255}, (Color){32,32,64,255});

}

void ObjectConfiguratorEditorComponent_update(SceneObject* node, SceneComponentId id, float dt, void* componentData)
{
    
}

static void DrawHierarchyNode(SceneGraph *sceneGraph, SceneObjectId objId, int depth, int *y)
{
    SceneObject *obj = SceneGraph_getObject(sceneGraph, objId);
    if (obj == NULL) return;
    int x = 10 + depth * 10;
    int h = 16;
    int availableSpace = DuskGui_getAvailableSpace().x;
    char name[strlen(obj->name) + 10];
    sprintf(name, "%s (%d)", obj->name, objId.id);
    if (DuskGui_label((DuskGuiParams){.text = name, .bounds = (Rectangle) { x, *y, availableSpace - 10 - x, h }, .rayCastTarget = 1,
        .style = DuskGui_getStyle(DUSKGUI_STYLE_LABELBUTTON)
    }))
    {
        printf("Clicked on %s\n", obj->name);
    }
    *y += h;
    for (int i=0;i<obj->children_count;i++)
    {
        DrawHierarchyNode(sceneGraph, obj->children[i], depth + 1, y);
    }
}

void ObjectConfiguratorEditorComponent_draw2D(Camera2D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
    void* componentData, void* userdata)
{
    ObjectConfiguratorEditorComponent *data = (ObjectConfiguratorEditorComponent*)componentData;
    if (DuskGui_dragArea((DuskGuiParams){.text = "drag_base", .bounds = (Rectangle) { 0, 0, GetScreenWidth(), GetScreenHeight() }, .rayCastTarget = 1}))
    {
        Vector2 delta = GetMouseDelta();
        Vector3 rotationYaw = SceneGraph_getLocalRotation(psg.sceneGraph, data->cameraPivotYawId);
        Vector3 rotationPitch = SceneGraph_getLocalRotation(psg.sceneGraph, data->cameraPivotPitchId);

        rotationPitch.x += delta.y;
        rotationYaw.y -= delta.x;
        if (rotationPitch.x > 90) rotationPitch.x = 90;
        if (rotationPitch.x < -90) rotationPitch.x = -90;
        SceneGraph_setLocalRotation(psg.sceneGraph, data->cameraPivotYawId, rotationYaw);
        SceneGraph_setLocalRotation(psg.sceneGraph, data->cameraPivotPitchId, rotationPitch);
    }

    DuskGuiParamsEntry panel = DuskGui_beginPanel((DuskGuiParams){
        .text="##hierarchy_view", 
        .bounds = (Rectangle) { -1, -2, 200, GetScreenHeight()+4}, .rayCastTarget = 1});
    DuskGui_label((DuskGuiParams){.text = "Hierarchy", .bounds = (Rectangle) { 10, 40, 100, 50 }, .rayCastTarget = 1});
    if (DuskGui_button((DuskGuiParams){.text = "<- back", .bounds = (Rectangle) { 10, 10, 85, 20 }, .rayCastTarget = 1}))
    {
        GameStateLevel_Init();
    }
    if (DuskGui_button((DuskGuiParams){.text = "Reset view", .bounds = (Rectangle) { 100, 10, 85, 20 }, .rayCastTarget = 1}))
    {
        SceneGraph_setLocalRotation(psg.sceneGraph, data->cameraPivotYawId, (Vector3) { 0, 0, 0 });
        SceneGraph_setLocalRotation(psg.sceneGraph, data->cameraPivotPitchId, (Vector3) { 30, 0, 0 });
    }

    int y = 70;
    for (int i=0;i<psg.sceneGraph->objects_count;i++)
    {
        SceneObjectId objId = psg.sceneGraph->objects[i].id;
        SceneObject *obj = SceneGraph_getObject(psg.sceneGraph, objId);
        if (obj == NULL) continue;

        SceneObject *parent = SceneGraph_getObject(psg.sceneGraph, obj->parent);
        if (parent != NULL) continue;

        DrawHierarchyNode(psg.sceneGraph, objId, 0, &y);

    }


    DuskGui_endPanel(panel);

    DuskGui_evaluate();
}
void ObjectConfiguratorEditorComponent_draw(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
    void* componentData, void* userdata)
{
    // printf("Camera: %f %f %f -> %f %f %f\n", camera.position.x, camera.position.y, camera.position.z,
    //     camera.target.x, camera.target.y, camera.target.z);
}