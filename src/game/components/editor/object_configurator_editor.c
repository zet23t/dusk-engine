#include "../../game_g.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

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
    
    SceneObjectId plane = SceneGraph_createObject(psg.sceneGraph, "Plane");
    AddMeshRendererComponentByName(plane, "fighter-plane-1", 1.0f);

    ObjectConfiguratorEditorComponent *data = (ObjectConfiguratorEditorComponent*)componentData;
    data->cameraPivotYawId = cameraPivotYaw;
    data->cameraPivotPitchId = cameraPivotPitch;

    DuskGui_init();
    DuskGui_setDefaultFont(font, font.baseSize, 1, BLACK, (Color){16,16,32,255}, (Color){32,32,64,255});

}

void ObjectConfiguratorEditorComponent_update(SceneObject* node, SceneComponentId id, float dt, void* componentData)
{
    if (!GuiIsLocked() && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        // Vector2 delta = GetMouseDelta();
        // printf("Mouse delta: %f %f\n", delta.x, delta.y);
        // SceneGraph_rotateLocal(psg.sceneGraph, ((ObjectConfiguratorEditorComponent*)componentData)->cameraPivotId, (Vector3) { delta.y, delta.x, 0 });
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
        .text="hierarchy_view", 
        .bounds = (Rectangle) { 0, 20, 200, GetScreenHeight() }, .rayCastTarget = 1});
    if (DuskGui_button((DuskGuiParams){.text = "<- back", .bounds = (Rectangle) { 10, 10, 90, 30 }, .rayCastTarget = 1}))
    {
        GameStateLevel_Init();
    }
    if (DuskGui_button((DuskGuiParams){.text = "Reset view", .bounds = (Rectangle) { 100, 10, 100, 30 }, .rayCastTarget = 1}))
    {
        SceneGraph_setLocalRotation(psg.sceneGraph, data->cameraPivotYawId, (Vector3) { 0, 0, 0 });
        SceneGraph_setLocalRotation(psg.sceneGraph, data->cameraPivotPitchId, (Vector3) { 30, 0, 0 });
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