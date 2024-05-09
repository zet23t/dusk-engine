#include "../../game_g.h"
#include "../../game_state_level.h"
#include "shared/serialization/serializers.h"

#define DUSK_GUI_IMPLEMENTATION
#include "shared/ui/dusk-gui.h"
#include "float.h"

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

typedef struct Arena10KB {
    uint8_t data[10000];
    size_t used;
} Arena10KB;

void* Arena_alloc(Arena10KB* arena, size_t size)
{
    if (arena->used + size > sizeof(arena->data)) {
        return NULL;
    }
    void* result = &arena->data[arena->used];
    arena->used += size;
    return result;
}

typedef struct AnnotationData {
    const char* key;
    const char* type;
    size_t size;
    void* data;
} AnnotationData;

#define ANNOTATION_MAX_STACK_SIZE 0x20
typedef struct GUIDrawState {
    int y;
    int indention;
    int labelWidth;
    SceneObjectId selectedObjectId;
    SceneGraph* sceneGraph;
    AnnotationData annotationStack[ANNOTATION_MAX_STACK_SIZE];
    int annotationStackCount;
    Arena10KB annotationData;
} GUIDrawState;

static void GUIDrawState_pushAnnotation(GUIDrawState* state, const char* key, const char* type, size_t size, void* data)
{
    if (state->annotationStackCount >= ANNOTATION_MAX_STACK_SIZE) {
        TraceLog(LOG_ERROR, "Annotation stack overflow: %s %s %d", key, type, size);
        return;
    }
    void* memory = Arena_alloc(&state->annotationData, size);
    if (memory == NULL) {
        TraceLog(LOG_ERROR, "Annotation memory overflow: %s %s %d", key, type, size);
        return;
    }

    AnnotationData* annotation = &state->annotationStack[state->annotationStackCount++];
    annotation->key = key;
    annotation->type = type;
    annotation->size = size;
    annotation->data = memory;
    memcpy(memory, data, size);
}

static void GUIDrawState_clearAnnotationStack(GUIDrawState* state)
{
    state->annotationData.used = 0;
    state->annotationStackCount = 0;
}

static int GUIDrawState_getAnnotation(GUIDrawState* state, const char* key, const char* type, size_t size, void* data)
{
    for (int i = 0; i < state->annotationStackCount; i++) {
        AnnotationData* annotation = &state->annotationStack[i];
        if (strcmp(annotation->key, key) == 0 && strcmp(annotation->type, type) == 0) {
            if (annotation->size != size) {
                TraceLog(LOG_ERROR, "Annotation size mismatch: %s %s %d != %d", key, type, size, annotation->size);
                return 0;
            }
            memcpy(data, annotation->data, annotation->size);
            return 1;
        }
    }
    return 0;
}

static void DrawSerializeData_rangedInt(const char* key, int* data, GUIDrawState* state, int min, int max, int showSlider, int showNumber)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[64];
    sprintf(buffer, "%d##%s:%d-int", *data, key, state->selectedObjectId.id);
    Vector2 space = DuskGui_getAvailableSpace();
    if (showSlider) {
        // if (DuskGui_slider((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 }, data, min, max)) {
        //     if (*data < min) {
        //         *data = min;
        //     }
        //     if (*data > max) {
        //         *data = max;
        //     }
        // }
    } else {
        DuskGui_label((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 });
    }
    state->y += 18;
}

void DrawSerializeData_int(const char* key, int* data, GUIDrawState* state)
{
    int min = INT_MIN;
    int max = INT_MAX;
    int showSlider = 0;
    int showNumber = 1;
    Vector2 range;
    if (GUIDrawState_getAnnotation(state, "Range", "Vector2", sizeof(Vector2), &range)) {
        min = (int)range.x;
        max = (int)range.y;
        showSlider = 1;
    }
    DrawSerializeData_rangedInt(key, data, state, min, max, showSlider, showNumber);
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
int DrawSerializeData_Vector3(const char* key, Vector3* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[120];
    Vector2 space = DuskGui_getAvailableSpace();
    int totalSpace = space.x - 10 - state->labelWidth;
    int spacing = 2;
    int cellWidth = (totalSpace - spacing * 3) / 3;
    int offsetX = 10 + state->labelWidth;
    int y = state->y;
    int h = 18;

    sprintf(buffer, "%.2f##%s:%d-x", data->x, key, state->selectedObjectId.id);
    int modX = DuskGui_floatInputField((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { offsetX, y, cellWidth, h }, .rayCastTarget = 1 }, &data->x, -FLT_MAX, FLT_MAX);
    offsetX += cellWidth + spacing;

    sprintf(buffer, "%.2f##%s:%d-y", data->y, key, state->selectedObjectId.id);
    int modY = DuskGui_floatInputField((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { offsetX, y, cellWidth, h }, .rayCastTarget = 1 }, &data->y, -FLT_MAX, FLT_MAX);
    offsetX += cellWidth + spacing;

    sprintf(buffer, "%.2f##%s:%d-z", data->z, key, state->selectedObjectId.id);
    int modZ = DuskGui_floatInputField((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { offsetX, y, cellWidth, h }, .rayCastTarget = 1 }, &data->z, -FLT_MAX, FLT_MAX);

    state->y += 18;

    return modX || modY || modZ;
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
    float min = -FLT_MAX;
    float max = FLT_MAX;
    int showSlider = 0;
    int showNumber = 1;
    Vector2 range;
    if (GUIDrawState_getAnnotation(state, "Range", "Vector2", sizeof(Vector2), &range)) {
        min = range.x;
        max = range.y;
        showSlider = 1;
    }
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[64];
    sprintf(buffer, "%.2f##%s:%d-float", *data, key, state->selectedObjectId.id);
    Vector2 space = DuskGui_getAvailableSpace();
    if (showSlider && max > min) {
        DuskGui_horizontalFloatSlider((DuskGuiParams) {
                                          .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - state->labelWidth - 10 - 10, 18 }, .rayCastTarget = 1 },
            data, min, max);
    } else
        DuskGui_floatInputField((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth - 10, 18 }, .rayCastTarget = 1 },
            data, min, max);

    // DuskGui_label((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - 10 - state->labelWidth, 18 }, .rayCastTarget = 1 });
    state->y += 18;
}
void DrawSerializeData_cstr(const char* key, char** data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    Vector2 space = DuskGui_getAvailableSpace();
    char* buffer = NULL;
    char id[strlen(key) + 20];
    sprintf(id, "%s##%s:%d-textfield", *data, key, state->selectedObjectId.id);
    DuskGui_textInputField((DuskGuiParams) { .text = id,
                               .bounds = (Rectangle) { 10 + state->labelWidth, state->y, space.x - state->labelWidth - 10, 18 },
                               .rayCastTarget = 1,
                               .isFocusable = 1 },
        &buffer);
    if (buffer != NULL) {
        free(*data);
        *data = buffer;
    }
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

void DrawSerializeData_SceneObjectTransformOverride(const char* key, SceneObjectTransform* data, GUIDrawState* state)
{
    int mod_p = DrawSerializeData_Vector3("position", &data->position, state);
    int mod_r = DrawSerializeData_Vector3("rotation", &data->eulerRotationDegrees, state);
    int mod_s = DrawSerializeData_Vector3("scale", &data->scale, state);
    if (mod_p || mod_r || mod_s) {
        SceneGraph_setLocalTransform(state->sceneGraph, state->selectedObjectId, data->position, data->eulerRotationDegrees, data->scale);
    }
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
#define SERIALIZABLE_ANNOTATION(key, type, value) \
    GUIDrawState_pushAnnotation(state, #key, #type, sizeof(value), value);
#define SERIALIZABLE_FIELD(type, name)                             \
    if (_drawFunctionOverrides._##type == NULL)                    \
        DrawSerializeData_##type(#name, &data->name, state);       \
    else                                                           \
        _drawFunctionOverrides._##type(#name, &data->name, state); \
    GUIDrawState_clearAnnotationStack(state);
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
    }                                                                                                                                                                              \
    GUIDrawState_clearAnnotationStack(state);
#define SERIALIZABLE_CSTR(name)                        \
    DrawSerializeData_cstr(#name, &data->name, state); \
    GUIDrawState_clearAnnotationStack(state);
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
    state->indention--;                                                                                                                                          \
    GUIDrawState_clearAnnotationStack(state);
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
    DrawSerializeData_cstr("name", &data->name, state);
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
    if (DuskGui_dragArea((DuskGuiParams) { .text = "drag_base", .bounds = (Rectangle) { 0, 0, GetScreenWidth(), GetScreenHeight() }, .rayCastTarget = 1, .isFocusable = 1 })) {
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

    DuskGuiParamsEntry* panel = DuskGui_beginPanel((DuskGuiParams) {
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

    if (DuskGui_button((DuskGuiParams) { .text = "Menu test", .bounds = (Rectangle) { 100, 70, 85, 20 }, .rayCastTarget = 1 })) {
        printf("Opening menu\n");
        DuskGui_openMenu("test_menu");
    }

    if (DuskGui_beginMenu((DuskGuiParams) { .text = "test_menu", .bounds = (Rectangle) { 100, 85, 200, 200 } })) {

        for (int i = 0; i < 5; i++) {
            char buffer[32];
            sprintf(buffer, i % 2 ? "Submenu %d" : "Test %d", i);
            if (DuskGui_menuItem(i % 2, (DuskGuiParams) { .text = buffer, .bounds = DuskGui_fillHorizontally(20 * i, 0, 0, 20), .rayCastTarget = 1 })) {
                if (i % 2) {
                    sprintf(buffer, "test_menu_%d", i);
                    DuskGui_openMenu(buffer);
                } else {
                    printf("%s clicked\n", buffer);
                }
            }
        }
        if (DuskGui_menuItem(0, (DuskGuiParams) { .text = "Close", .bounds = DuskGui_fillHorizontally(20 * 5, 0, 0, 20), .rayCastTarget = 1 })) {
            DuskGui_closeMenu("test_menu");
        }

        for (int i = 1; i < 5; i += 2) {
            char buffer[32];
            sprintf(buffer, "test_menu_%d", i);
            char buffer2[32];
            sprintf(buffer2, "test_menu_2_%d", i);
            if (DuskGui_beginMenu((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { 100, 20 * i, 200, 200 } })) {
                DuskGui_menuItem(0, (DuskGuiParams) { .text = "Test-1", .bounds = DuskGui_fillHorizontally(0, 0, 0, 20), .rayCastTarget = 1 });
                if (DuskGui_menuItem(1, (DuskGuiParams) { .text = "Test-2", .bounds = DuskGui_fillHorizontally(20, 0, 0, 20), .rayCastTarget = 1 }))
                {
                    DuskGui_openMenu(buffer2);
                }
                if (DuskGui_beginMenu((DuskGuiParams) { .text = buffer2, .bounds = (Rectangle) { 100, 20 * i, 200, 200 } })) {
                
                    DuskGui_endMenu();
                }
                DuskGui_endMenu();
            }
        }
        DuskGui_endMenu();
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
    _drawFunctionOverrides._SceneObjectTransform = DrawSerializeData_SceneObjectTransformOverride;

    const int inspectorHeight = 400;
    const int inspectorWidth = 350;
    DuskGuiParamsEntry* inspector_panel = DuskGui_beginPanel((DuskGuiParams) { .text = "##object_view", .bounds = (Rectangle) { GetScreenWidth() - inspectorWidth, GetScreenHeight() - inspectorHeight, inspectorWidth, inspectorHeight }, .rayCastTarget = 1 });
    DuskGuiParamsEntry* inspector_scroll = DuskGui_beginScrollArea((DuskGuiParams) { .text = "##object_scroll", .bounds = (Rectangle) { 0, 0, inspectorWidth, inspectorHeight }, .rayCastTarget = 1 });
    SceneObject* selectedObj = SceneGraph_getObject(psg.sceneGraph, data->selectedObjectId);
    if (selectedObj != NULL) {
        GUIDrawState state = { .labelWidth = 140, .sceneGraph = psg.sceneGraph, .selectedObjectId = data->selectedObjectId };
        state.y = 0;
        DrawSerializedData_SceneObject(NULL, selectedObj, &state);
        inspector_scroll->contentSize = (Vector2) { 300, state.y };
        if (data->selectedObjectId.id != state.selectedObjectId.id) {
            data->selectedObjectId = state.selectedObjectId;
            inspector_scroll->contentOffset = (Vector2) { 0, 0 };
        }
    } else {
        inspector_scroll->contentOffset = (Vector2) { 0, 0 };
    }
    DuskGui_endScrollArea(inspector_scroll);
    DuskGui_endPanel(inspector_panel);

    DuskGui_finalize();
}
void ObjectConfiguratorEditorComponent_draw(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
    void* componentData, void* userdata)
{
    // printf("Camera: %f %f %f -> %f %f %f\n", camera.position.x, camera.position.y, camera.position.z,
    //     camera.target.x, camera.target.y, camera.target.z);
}