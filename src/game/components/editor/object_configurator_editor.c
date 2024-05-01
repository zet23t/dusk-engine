#include "../../game_g.h"
#include "../../game_state_level.h"
#include "shared/serialization/serializers.h"

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
    SpawnEnemy(psg.sceneGraph, 0, 0, (EnemyBehaviorComponent) { 0 });

    ObjectConfiguratorEditorComponent* data = (ObjectConfiguratorEditorComponent*)componentData;
    data->cameraPivotYawId = cameraPivotYaw;
    data->cameraPivotPitchId = cameraPivotPitch;

    DuskGui_init();
    DuskGui_setDefaultFont(font, font.baseSize, 1);
}

void ObjectConfiguratorEditorComponent_update(SceneObject* node, SceneComponentId id, float dt, void* componentData)
{
}

typedef struct GUIDrawState {
    int y;
    int indention;
    int labelWidth;
    SceneObjectId selectedObjectId;
    SceneGraph* sceneGraph;
} GUIDrawState;

void DrawSerializeData_int(const char* key, int* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[32];
    sprintf(buffer, "%d", *data);
    Vector2 space = DuskGui_getAvailableSpace();
    DuskGui_label((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 });
    state->y += 18;
}
void DrawSerializeData_bool(const char* key, bool* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[32];
    sprintf(buffer, "%s", *data ? "true" : "false");
    Vector2 space = DuskGui_getAvailableSpace();
    DuskGui_label((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 });
    state->y += 18;
}

void DrawSerializeData_Color(const char* key, Color* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[32];
    sprintf(buffer, "%d %d %d %d", data->r, data->g, data->b, data->a);
    Vector2 space = DuskGui_getAvailableSpace();
    DuskGui_label((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 });
    state->y += 18;
}

void DrawSerializeData_int8_t(const char* key, int8_t* data, GUIDrawState* userData)
{
    int v = *data;
    DrawSerializeData_int(key, &v, userData);
    *data = (int8_t)v;
}

void DrawSerializeData_uint32_t(const char* key, uint32_t* data, GUIDrawState* userData)
{
    int v = *data;
    DrawSerializeData_int(key, &v, userData);
    *data = (uint32_t)v;
}
void DrawSerializeData_int32_t(const char* key, int32_t* data, GUIDrawState* userData)
{
    int v = *data;
    DrawSerializeData_int(key, &v, userData);
    *data = (int32_t)v;
}
void DrawSerializeData_uint8_t(const char* key, uint8_t* data, GUIDrawState* userData)
{
    int v = *data;
    DrawSerializeData_int(key, &v, userData);
    *data = (uint8_t)v;
}
void DrawSerializeData_size_t(const char* key, size_t* data, GUIDrawState* userData)
{
    int v = *data;
    DrawSerializeData_int(key, &v, userData);
    *data = (size_t)v;
}
void DrawSerializeData_Vector3(const char* key, Vector3* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[120];
    sprintf(buffer, "%.2f %.2f %.2f", data->x, data->y, data->z);
    Vector2 space = DuskGui_getAvailableSpace();
    DuskGui_label((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 });
    state->y += 18;
}
void DrawSerializeData_Vector2(const char* key, Vector2* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[120];
    sprintf(buffer, "%.2f %.2f", data->x, data->y);
    Vector2 space = DuskGui_getAvailableSpace();
    DuskGui_label((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 });
    state->y += 18;
}
void DrawSerializeData_float(const char* key, float* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[32];
    sprintf(buffer, "%.2f", *data);
    Vector2 space = DuskGui_getAvailableSpace();
    DuskGui_label((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 });
    state->y += 18;
}
void DrawSerializeData_cstr(const char* key, char* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    Vector2 space = DuskGui_getAvailableSpace();
    DuskGui_label((DuskGuiParams) { .text = data, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - state->labelWidth - 10, 18 }, .rayCastTarget = 1 });
    state->y += 18;
}

void DrawSerializeData_SceneComponentIdOverride(const char* key, SceneComponentId* data, GUIDrawState* state)
{
    SceneComponentType* sceneComponent = SceneGraph_getComponentType(state->sceneGraph, data->typeId);
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[120];
    sprintf(buffer, "%s (%d)", sceneComponent ? sceneComponent->name : "MISSING", data->id);
    Vector2 space = DuskGui_getAvailableSpace();
    DuskGui_label((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 });
    state->y += 18;
}

void DrawSerializeData_SceneObjectIdOverride(const char* key, SceneObjectId* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[120];
    SceneObject* sceneObject = SceneGraph_getObject(state->sceneGraph, *data);
    if (sceneObject == NULL) {
        sprintf(buffer, "NULL");
    } else {
        sprintf(buffer, "%s (%d)", sceneObject->name, data->id);
    }
    Vector2 space = DuskGui_getAvailableSpace();
    if (DuskGui_label((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 })) {
        state->selectedObjectId = *data;
    }
    state->y += 18;
}

typedef struct DrawFunctionOverrides {
    void (*_uint32_t)(const char* key, uint32_t* data, GUIDrawState* userData);
    void (*_int32_t)(const char* key, int32_t* data, GUIDrawState* userData);
    void (*_uint8_t)(const char* key, uint8_t* data, GUIDrawState* userData);
    void (*_int8_t)(const char* key, int8_t* data, GUIDrawState* userData);
    void (*_size_t)(const char* key, size_t* data, GUIDrawState* userData);
    void (*_float)(const char* key, float* data, GUIDrawState* userData);
    void (*_int)(const char* key, int* data, GUIDrawState* userData);
    void (*_bool)(const char* key, bool* data, GUIDrawState* userData);
    void (*_Vector3)(const char* key, Vector3* data, GUIDrawState* userData);
    void (*_Vector2)(const char* key, Vector2* data, GUIDrawState* userData);
    void (*_Color)(const char* key, Color* data, GUIDrawState* userData);
#define SERIALIZABLE_STRUCT_START(name) void (*_##name)(const char* key, name* data, GUIDrawState* userData);
#include "shared/serialization/serializable_file_headers.h"
} DrawFunctionOverrides;

DrawFunctionOverrides _drawFunctionOverrides;

#define SERIALIZABLE_STRUCT_START(name)                                                                                                                                \
    void DrawSerializeData_##name(const char* key, name* data, GUIDrawState* state)                                                                                    \
    {                                                                                                                                                                  \
        if (key != NULL) {                                                                                                                                             \
            DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 }); \
            state->indention++, state->y += 18;                                                                                                                        \
        }
#define SERIALIZABLE_FIELD(type, name)                       \
    if (_drawFunctionOverrides._##type == NULL)              \
        DrawSerializeData_##type(#name, &data->name, state); \
    else                                                     \
        _drawFunctionOverrides._##type(#name, &data->name, state);
#define SERIALIZABLE_FIXED_ARRAY(type, name, count)                                                                                                                                \
    {                                                                                                                                                                              \
        char name##Title[strlen(#name) + 10 + strlen(#type)];                                                                                                                      \
        sprintf(name##Title, "%s %s[%d]", #type, #name, count);                                                                                                                    \
        if (DuskGui_foldout((DuskGuiParams) { .text = name##Title, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 })) { \
            state->y += 18;                                                                                                                                                        \
            state->indention++;                                                                                                                                                    \
            for (int i = 0; i < count; i++) {                                                                                                                                      \
                char num[32];                                                                                                                                                      \
                sprintf(num, "%d", i);                                                                                                                                             \
                if (_drawFunctionOverrides._##type == NULL)                                                                                                                        \
                    DrawSerializeData_##type(num, &data->name[i], state);                                                                                                          \
                else                                                                                                                                                               \
                    _drawFunctionOverrides._##type(num, &data->name[i], state);                                                                                                    \
            }                                                                                                                                                                      \
            state->indention--;                                                                                                                                                    \
        } else {                                                                                                                                                                   \
            state->y += 18;                                                                                                                                                        \
        }                                                                                                                                                                          \
    }
#define SERIALIZABLE_CSTR(name) DrawSerializeData_cstr(#name, data->name, state);
#define SERIALIZABLE_STRUCT_LIST_ELEMENT(type, name)                                                                                                             \
    DuskGui_label((DuskGuiParams) { .text = #name, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 }); \
    state->indention++, state->y += 18;                                                                                                                          \
    for (int i = 0; i < data->name##_count; i++) {                                                                                                               \
        char num[32];                                                                                                                                            \
        sprintf(num, "%d", i);                                                                                                                                   \
        if (_drawFunctionOverrides._##type == NULL)                                                                                                              \
            DrawSerializeData_##type(num, &data->name[i], state);                                                                                                \
        else                                                                                                                                                     \
            _drawFunctionOverrides._##type(num, &data->name[i], state);                                                                                          \
    }                                                                                                                                                            \
    state->indention--;
// #define POST_SERIALIZE_CALLBACK(type, name) name(key, data, element, userData);
#define SERIALIZABLE_STRUCT_END(name) \
    if (key != NULL)                  \
        state->indention--;           \
    }
#include "shared/serialization/serializable_file_headers.h"

typedef void (*DrawFunction)(const char* key, void* data, GUIDrawState* state);

const char* _serializableStructNames[] = {
#define SERIALIZABLE_STRUCT_START(name) #name,
#include "shared/serialization/serializable_file_headers.h"
};
DrawFunction _serializableStructDrawFunctions[] = {
#define SERIALIZABLE_STRUCT_START(name) (DrawFunction) DrawSerializeData_##name,
#include "shared/serialization/serializable_file_headers.h"
};

DrawFunction GetDrawFunction(const char* name)
{
    for (int i = 0; i < sizeof(_serializableStructNames) / sizeof(_serializableStructNames[0]); i++) {
        if (strcmp(_serializableStructNames[i], name) == 0) {
            return _serializableStructDrawFunctions[i];
        }
    }
    return NULL;
}

void DrawSerializedData_SceneObject(const char* key, SceneObject* data, GUIDrawState* state)
{
    if (key != NULL)
        state->indention++;
    DrawSerializeData_cstr("name", data->name, state);
    DuskGui_horizontalLine((DuskGuiParams) { .text = "Transform", .bounds = (Rectangle) { 1, state->y, DuskGui_getAvailableSpace().x - 2, 18 } });
    state->y += 20;
    DrawSerializeData_SceneObjectTransform(NULL, &data->transform, state);
    state->y += 3;

    for (int i = 0; i < data->components_count; i++) {
        void* componentData;
        SceneComponentType* componentType = SceneGraph_getComponentType(state->sceneGraph, data->components[i].typeId);
        SceneComponent* component = SceneGraph_getComponent(state->sceneGraph, data->components[i], &componentData);
        if (component == NULL)
            continue;
        DuskGui_horizontalLine((DuskGuiParams) { .text = componentType->name, .bounds = (Rectangle) { 1, state->y, DuskGui_getAvailableSpace().x - 2, 18 } });
        state->y += 20;
        DrawFunction drawFunction = GetDrawFunction(componentType->name);
        if (drawFunction != NULL) {
            drawFunction(NULL, componentData, state);
        } else {
        }
        // DrawSerializeData_SceneComponentIdOverride(NULL, &component->id, state);
    }
    if (key != NULL)
        state->indention--;
}
static void DrawHierarchyNode(SceneGraph* sceneGraph, ObjectConfiguratorEditorComponent* cfgEdit, SceneObjectId objId, int depth, int* y)
{
    SceneObject* obj = SceneGraph_getObject(sceneGraph, objId);
    if (obj == NULL)
        return;
    int x = 10 + depth * 10;
    int h = 16;
    int availableSpace = DuskGui_getAvailableSpace().x;
    char name[strlen(obj->name) + 10];
    sprintf(name, "%s (%d)", obj->name, objId.id);
    if (DuskGui_label((DuskGuiParams) { .text = name, .bounds = (Rectangle) { x, *y, availableSpace - 10 - x, h }, .rayCastTarget = 1, .styleGroup = DuskGui_getStyle(DUSKGUI_STYLE_LABELBUTTON) })) {
        printf("Clicked on %s\n", obj->name);
        cfgEdit->selectedObjectId = objId;
    }
    *y += h;
    for (int i = 0; i < obj->children_count; i++) {
        DrawHierarchyNode(sceneGraph, cfgEdit, obj->children[i], depth + 1, y);
    }
}

void ObjectConfiguratorEditorComponent_draw2D(Camera2D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
    void* componentData, void* userdata)
{
    ObjectConfiguratorEditorComponent* data = (ObjectConfiguratorEditorComponent*)componentData;
    if (DuskGui_dragArea((DuskGuiParams) { .text = "drag_base", .bounds = (Rectangle) { 0, 0, GetScreenWidth(), GetScreenHeight() }, .rayCastTarget = 1 })) {
        Vector2 delta = GetMouseDelta();
        Vector3 rotationYaw = SceneGraph_getLocalRotation(psg.sceneGraph, data->cameraPivotYawId);
        Vector3 rotationPitch = SceneGraph_getLocalRotation(psg.sceneGraph, data->cameraPivotPitchId);

        rotationPitch.x += delta.y;
        rotationYaw.y -= delta.x;
        if (rotationPitch.x > 90)
            rotationPitch.x = 90;
        if (rotationPitch.x < -90)
            rotationPitch.x = -90;
        SceneGraph_setLocalRotation(psg.sceneGraph, data->cameraPivotYawId, rotationYaw);
        SceneGraph_setLocalRotation(psg.sceneGraph, data->cameraPivotPitchId, rotationPitch);
    }

    DuskGuiParamsEntry panel = DuskGui_beginPanel((DuskGuiParams) {
        .text = "##hierarchy_view",
        .bounds = (Rectangle) { -1, -2, 200, GetScreenHeight() + 4 },
        .rayCastTarget = 1 });
    DuskGui_label((DuskGuiParams) { .text = "Hierarchy", .bounds = (Rectangle) { 10, 40, 100, 50 }, .rayCastTarget = 1 });
    if (DuskGui_button((DuskGuiParams) { .text = "<- back", .bounds = (Rectangle) { 10, 10, 85, 20 }, .rayCastTarget = 1 })) {
        GameStateLevel_Init();
    }
    if (DuskGui_button((DuskGuiParams) { .text = "Reset view", .bounds = (Rectangle) { 100, 10, 85, 20 }, .rayCastTarget = 1 })) {
        SceneGraph_setLocalRotation(psg.sceneGraph, data->cameraPivotYawId, (Vector3) { 0, 0, 0 });
        SceneGraph_setLocalRotation(psg.sceneGraph, data->cameraPivotPitchId, (Vector3) { 30, 0, 0 });
    }

    if (DuskGui_button((DuskGuiParams) { .text = "Serialize", .bounds = (Rectangle) { 10, 70, 85, 20 }, .rayCastTarget = 1 })) {
        cJSON* obj = cJSON_CreateObject();
        SerializeData_SceneGraph(NULL, psg.sceneGraph, obj, NULL);
        char* serialized = cJSON_Print(obj);
        printf("Serialized:\n%s\n", serialized);
    }

    int y = 90;
    for (int i = 0; i < psg.sceneGraph->objects_count; i++) {
        SceneObjectId objId = psg.sceneGraph->objects[i].id;
        SceneObject* obj = SceneGraph_getObject(psg.sceneGraph, objId);
        if (obj == NULL)
            continue;

        SceneObject* parent = SceneGraph_getObject(psg.sceneGraph, obj->parent);
        if (parent != NULL)
            continue;

        DrawHierarchyNode(psg.sceneGraph, data, objId, 0, &y);
    }
    DuskGui_endPanel(panel);

    _drawFunctionOverrides._SceneComponentId = DrawSerializeData_SceneComponentIdOverride;
    _drawFunctionOverrides._SceneObjectId = DrawSerializeData_SceneObjectIdOverride;
    _drawFunctionOverrides._SceneObject = DrawSerializedData_SceneObject;
    const int inspectorHeight = 400;
    DuskGuiParamsEntry inspector_panel = DuskGui_beginPanel((DuskGuiParams) { .text = "##object_view", .bounds = (Rectangle) { GetScreenWidth() - 300, GetScreenHeight() - inspectorHeight, 300, inspectorHeight }, .rayCastTarget = 1 });
    DuskGuiParamsEntry inspector_scroll = DuskGui_beginScrollArea((DuskGuiParams) { .text = "##object_scroll", .bounds = (Rectangle) { 0, 0, 300, inspectorHeight }, .rayCastTarget = 1 });
    SceneObject* selectedObj = SceneGraph_getObject(psg.sceneGraph, data->selectedObjectId);
    if (selectedObj != NULL) {
        GUIDrawState state = { .labelWidth = 140, .sceneGraph = psg.sceneGraph, .selectedObjectId = data->selectedObjectId };
        state.y = 0;
        DrawSerializedData_SceneObject(NULL, selectedObj, &state);
        DuskGui_setContentSize(inspector_scroll, (Vector2) { 300, state.y });
        if (data->selectedObjectId.id != state.selectedObjectId.id) {
            data->selectedObjectId = state.selectedObjectId;
            DuskGui_setContentOffset(inspector_scroll, (Vector2) { 0, 0 });
        }
    } else {
        DuskGui_setContentOffset(inspector_scroll, (Vector2) { 0, 0 });
    }
    DuskGui_endScrollArea(inspector_scroll);
    DuskGui_endPanel(inspector_panel);

    DuskGui_evaluate();
}
void ObjectConfiguratorEditorComponent_draw(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
    void* componentData, void* userdata)
{
    // printf("Camera: %f %f %f -> %f %f %f\n", camera.position.x, camera.position.y, camera.position.z,
    //     camera.target.x, camera.target.y, camera.target.z);
}