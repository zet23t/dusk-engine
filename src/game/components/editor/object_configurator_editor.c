#include "../../game_g.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#define DUSK_GUI_IMPLEMENTATION
#include "dusk-gui.h"

void ObjectConfiguratorEditorComponent_init(SceneObject* sceneObject, SceneComponentId SceneComponent, void* componentData, void* initArg)
{
    Font font = ResourceManager_loadFont(&psg.resourceManager, "assets/myfont.png");
    GuiSetFont(font);
    GuiSetStyle(DEFAULT, TEXT_SIZE, font.baseSize);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0xffffffff);


    SceneObjectId camera = SceneGraph_createObject(psg.sceneGraph, "Camera");
    psg.camera = camera;
    SceneGraph_addComponent(psg.sceneGraph, camera, psg.cameraComponentId, &(CameraComponent) {
        .fov = 45,
        .nearPlane = 0.1f,
        .farPlane = 1000,
    });
    SceneGraph_setLocalPosition(psg.sceneGraph, camera, (Vector3) { 0, 0, -10 });
    SceneObjectId cameraPivot = SceneGraph_createObject(psg.sceneGraph, "CameraPivot");
    SceneGraph_setParent(psg.sceneGraph, camera, cameraPivot);
    SceneGraph_setLocalRotation(psg.sceneGraph, cameraPivot, (Vector3) { 30, 0, 0 });
    
    SceneObjectId plane = SceneGraph_createObject(psg.sceneGraph, "Plane");
    AddMeshRendererComponentByName(plane, "fighter-plane-1", 1.0f);

    ObjectConfiguratorEditorComponent *data = (ObjectConfiguratorEditorComponent*)componentData;
    data->cameraPivotId = cameraPivot;

    DuskGui_init();
    DuskGui_setDefaultFont(font, font.baseSize, 0, WHITE, BLUE, RED);

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
    if (GuiButton((Rectangle) { 10, 10, 100, 40 }, "Hello")) {
        printf("Hello\n");
    }
    if (GuiButton((Rectangle) { 20, 40, 100, 40 }, "World")) {
        printf("World\n");
    }

    if (DuskGui_button((DuskGuiParams){.text = "Dusk Hello", .bounds = (Rectangle) { 10, 70, 100, 40 }, .rayCastTarget = 1}))
    {
        printf("Dusk Hello\n");
    }
    DuskGui_button((DuskGuiParams){.text = "Overlapping", .bounds = (Rectangle) { 20, 100, 100, 40 }, .rayCastTarget = 1});
    DuskGui_evaluate();
}
void ObjectConfiguratorEditorComponent_draw(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
    void* componentData, void* userdata)
{
    // printf("Camera: %f %f %f -> %f %f %f\n", camera.position.x, camera.position.y, camera.position.z,
    //     camera.target.x, camera.target.y, camera.target.z);
}