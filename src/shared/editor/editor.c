#include "editor.h"
#include "shared/ui/dusk-gui.h"
#include "shared/util/arena.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "editor_styles.h"

void Editor_drawHierarchy(EditorState* state, SceneGraph *graph);
void Editor_drawInspector(EditorState* state, SceneGraph *graph);

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
    sprintf(buffer, "%d##%s:%p-int", *data, key, data);
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

void DrawSerializeData_uint16_t(const char* key, uint16_t* data, GUIDrawState* userData)
{
    int v = *data;
    DrawSerializeData_int(key, &v, userData);
    *data = (uint16_t)v;
}
void DrawSerializeData_int16_t(const char* key, int16_t* data, GUIDrawState* userData)
{
    int v = *data;
    DrawSerializeData_int(key, &v, userData);
    *data = (int16_t)v;
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

int DrawSerializeData_Vector4(const char* key, Vector4* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[120];
    Vector2 space = DuskGui_getAvailableSpace();
    int totalSpace = space.x - 10 - state->labelWidth;
    int spacing = 2;
    int cellWidth = (totalSpace - spacing * 4) / 4;
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
    offsetX += cellWidth + spacing;

    sprintf(buffer, "%.2f##%s:%d-w", data->w, key, state->selectedObjectId.id);
    int modW = DuskGui_floatInputField((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { offsetX, y, cellWidth, h }, .rayCastTarget = 1 }, &data->w, -FLT_MAX, FLT_MAX);

    state->y += 18;

    return modX || modY || modZ || modW;
}

int DrawSerializeData_Rectangle(const char* key, Rectangle* data, GUIDrawState* state)
{
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[120];
    Vector2 space = DuskGui_getAvailableSpace();
    int totalSpace = space.x - 10 - state->labelWidth;
    int spacing = 2;
    int cellWidth = (totalSpace - spacing * 4) / 4;
    int offsetX = 10 + state->labelWidth;
    int y = state->y;
    int h = 18;

    sprintf(buffer, "%.2f##%s:%d-x", data->x, key, state->selectedObjectId.id);
    int modX = DuskGui_floatInputField((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { offsetX, y, cellWidth, h }, .rayCastTarget = 1 }, &data->x, -FLT_MAX, FLT_MAX);
    offsetX += cellWidth + spacing;

    sprintf(buffer, "%.2f##%s:%d-y", data->y, key, state->selectedObjectId.id);
    int modY = DuskGui_floatInputField((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { offsetX, y, cellWidth, h }, .rayCastTarget = 1 }, &data->y, -FLT_MAX, FLT_MAX);
    offsetX += cellWidth + spacing;

    sprintf(buffer, "%.2f##%s:%d-width", data->width, key, state->selectedObjectId.id);
    int modZ = DuskGui_floatInputField((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { offsetX, y, cellWidth, h }, .rayCastTarget = 1 }, &data->width, -FLT_MAX, FLT_MAX);
    offsetX += cellWidth + spacing;

    sprintf(buffer, "%.2f##%s:%d-height", data->height, key, state->selectedObjectId.id);
    int modW = DuskGui_floatInputField((DuskGuiParams) { .text = buffer, .bounds = (Rectangle) { offsetX, y, cellWidth, h }, .rayCastTarget = 1 }, &data->height, -FLT_MAX, FLT_MAX);

    state->y += 18;

    return modX || modY || modZ || modW;
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
    // int showNumber = 1;
    Vector2 range;
    if (GUIDrawState_getAnnotation(state, "Range", "Vector2", sizeof(Vector2), &range)) {
        min = range.x;
        max = range.y;
        showSlider = 1;
    }
    DuskGui_label((DuskGuiParams) { .text = key, .bounds = (Rectangle) { 10 + state->indention * 10, state->y, state->labelWidth, 18 }, .rayCastTarget = 1 });
    char buffer[64];
    sprintf(buffer, "%.2f##%s:%p-float", *data, key, data);
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
    void (*_uint16_t)(const char* key, uint16_t* data, GUIDrawState* userData);
    void (*_int16_t)(const char* key, uint16_t* data, GUIDrawState* userData);
    void (*_uint8_t)(const char* key, uint8_t* data, GUIDrawState* userData);
    void (*_int8_t)(const char* key, int8_t* data, GUIDrawState* userData);
    void (*_size_t)(const char* key, size_t* data, GUIDrawState* userData);
    void (*_float)(const char* key, float* data, GUIDrawState* userData);
    void (*_int)(const char* key, int* data, GUIDrawState* userData);
    void (*_bool)(const char* key, bool* data, GUIDrawState* userData);
    void (*_Rectangle)(const char* key, Rectangle* data, GUIDrawState* userData);
    void (*_Vector4)(const char* key, Vector4* data, GUIDrawState* userData);
    void (*_Vector3)(const char* key, Vector3* data, GUIDrawState* userData);
    void (*_Vector2)(const char* key, Vector2* data, GUIDrawState* userData);
    void (*_Color)(const char* key, Color* data, GUIDrawState* userData);
#define SERIALIZABLE_STRUCT_START(name) void (*_##name)(const char* key, name* data, GUIDrawState* userData);
#include "shared/serialization/serializable_file_headers.h"
} DrawFunctionOverrides;

static DrawFunctionOverrides _drawFunctionOverrides;

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
    DrawSerializeData_SceneObjectTransformOverride(NULL, &data->transform, state);
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
        // printf("Drawing %s\n", componentType->name);
        if (drawFunction != NULL) {
            drawFunction(NULL, componentData, state);
        } else {
        }
        // DrawSerializeData_SceneComponentIdOverride(NULL, &component->id, state);
    }
    if (key != NULL)
        state->indention--;
}

static void Editor_drawHierarchyNode(EditorState* state, SceneGraph* sceneGraph, SceneObjectId objId, int depth, int* y)
{
    SceneObject* obj = SceneGraph_getObject(sceneGraph, objId);
    if (obj == NULL)
        return;
    int x = depth * 10;
    int h = 16;
    int availableSpace = DuskGui_getAvailableSpace().x;
    char name[strlen(obj->name) + 10];
    sprintf(name, "%s (%d)", obj->name, objId.id);
    if (DuskGui_label((DuskGuiParams) { .text = name, .bounds = (Rectangle) { x, *y, availableSpace - 10 - x, h }, .rayCastTarget = 1, .styleGroup = DuskGui_getStyleGroup(DUSKGUI_STYLE_LABELBUTTON) })) {
        state->selectedObjectId = objId;
        state->displayInspector = true;
    }
    *y += h;
    for (int i = 0; i < obj->children_count; i++) {
        Editor_drawHierarchyNode(state, sceneGraph, obj->children[i], depth + 1, y);
    }
}

void Editor_draw(EditorState* state, SceneGraph *graph)
{
    if (_editor_titleStyleGroup.normal == NULL)
    {
        EditorStyles_init();
    }

    _drawFunctionOverrides._SceneComponentId = DrawSerializeData_SceneComponentIdOverride;
    _drawFunctionOverrides._SceneObjectId = DrawSerializeData_SceneObjectIdOverride;
    _drawFunctionOverrides._SceneObject = DrawSerializedData_SceneObject;
    _drawFunctionOverrides._SceneObjectTransform = DrawSerializeData_SceneObjectTransformOverride;


    if (state->displayHierarchy)
        Editor_drawHierarchy(state, graph);
    if (state->displayInspector)
        Editor_drawInspector(state, graph);
}

static DuskGuiParamsEntryId Editor_beginPanelWindow(const char *title, Rectangle* bounds, bool *display)
{
    char textId[128];
    sprintf(textId, "##editor_%s_panel", title);
    DuskGuiParamsEntryId panelId = DuskGui_beginPanel((DuskGuiParams){
        .bounds = *bounds,
        .rayCastTarget = 1,
        .text = textId
    });
    
    DuskGui_label((DuskGuiParams){
        .text = title,
        .bounds = (Rectangle){0,0,bounds->width, 22},
        .rayCastTarget = 1,
        .styleGroup = &_editor_titleStyleGroup
    });

    if (DuskGui_hasLock(DuskGui_getLastEntry()))
    {
        Vector2 delta = GetMouseDelta();
        bounds->x += delta.x;
        bounds->y += delta.y;
    }

    sprintf(textId, "x##editor_%s_close_button", title);

    if (DuskGui_button((DuskGuiParams){
        .text = textId,
        .bounds = (Rectangle){bounds->width - 20, 2, 18, 18},
        .rayCastTarget = 1,
        .styleGroup = &_editor_closeButtonStyleGroup
    }))
    {
        *display = false;
    }
    return panelId;
}

void Editor_endPanelWindow(DuskGuiParamsEntryId panelId, Rectangle *bounds)
{
    DuskGuiParamsEntry* panel = DuskGui_getEntryById(panelId);
    char textId[128];
    sprintf(textId, "##editor_%s_resize_area", panel->txId);
    DuskGui_label((DuskGuiParams){
        .text = textId,
        .bounds = (Rectangle){bounds->width - 15, bounds->height-15, 14, 14},
        .rayCastTarget = 1,
        .styleGroup = &_editor_resizeAreaStyleGroup
    });
    if (DuskGui_hasLock(DuskGui_getLastEntry()))
    {
        Vector2 delta = GetMouseDelta();
        bounds->width += delta.x;
        bounds->height += delta.y;
    }

    DuskGui_endPanel(panelId);
}

void Editor_drawHierarchy(EditorState* state, SceneGraph *graph)
{
    if (state->hierarchyPanelBounds.width == 0)
    {
        state->hierarchyPanelBounds = (Rectangle) {
            .x = 0,
            .y = 50,
            .width = 200,
            .height = 300
        };
    }

    DuskGuiParamsEntryId panelId = Editor_beginPanelWindow("Hierarchy", &state->hierarchyPanelBounds, &state->displayHierarchy);

    DuskGuiParamsEntryId scrollAreaId = DuskGui_beginScrollArea((DuskGuiParams){
        .bounds = (Rectangle){.x = 5,
                              .y = 25,
                              .width = state->hierarchyPanelBounds.width - 10,
                              .height = state->hierarchyPanelBounds.height - 30},
        .rayCastTarget = 1,
        .styleGroup = &_editor_invisibleStyleGroup,
        .text = "##editor_hierarchy_scroll_area"
    });

    int y = 0;
    for (int i = 0; i < graph->objects_count; i++)
    {
        SceneObjectId objId = graph->objects[i].id;
        SceneObject* obj = SceneGraph_getObject(graph, objId);
        if (obj == NULL)
            continue;

        SceneObject* parent = SceneGraph_getObject(graph, obj->parent);
        if (parent != NULL)
            continue;

        Editor_drawHierarchyNode(state, graph, objId, 0, &y);
    }

    DuskGuiParamsEntry* scrollArea = DuskGui_getEntryById(scrollAreaId);
    scrollArea->contentSize = (Vector2) { 300, y };

    DuskGui_endScrollArea(scrollAreaId);

    Editor_endPanelWindow(panelId, &state->hierarchyPanelBounds);
}


void Editor_drawInspector(EditorState* state, SceneGraph *graph)
{
    if (state->inspectorPanelBounds.width == 0)
    {
        state->inspectorPanelBounds = (Rectangle) {
            .x = GetScreenWidth() - 380,
            .y = 50,
            .width = 380,
            .height = GetScreenHeight()-100
        };
    }

    DuskGuiParamsEntry* panel = Editor_beginPanelWindow("Inspector", &state->inspectorPanelBounds, &state->displayInspector);

    DuskGuiParamsEntryId scrollAreaId = DuskGui_beginScrollArea((DuskGuiParams){
        .bounds = (Rectangle){.x = 5,
                              .y = 25,
                              .width = state->inspectorPanelBounds.width - 10,
                              .height = state->inspectorPanelBounds.height - 30},
        .rayCastTarget = 1,
        .styleGroup = &_editor_invisibleStyleGroup,
        .text = "##editor_inspector_scroll_area"
    });

    DuskGuiParamsEntry* scrollArea;
    SceneObject* selectedObj = SceneGraph_getObject(graph, state->selectedObjectId);
    if (selectedObj != NULL) {
        GUIDrawState guiState = { .labelWidth = 140, .sceneGraph = graph, .selectedObjectId = state->selectedObjectId };
        guiState.y = 0;
        DrawSerializedData_SceneObject(NULL, selectedObj, &guiState);

        scrollArea = DuskGui_getEntryById(scrollAreaId);
        scrollArea->contentSize = (Vector2) { 300, guiState.y };
        if (state->selectedObjectId.id != guiState.selectedObjectId.id) {
            state->selectedObjectId = guiState.selectedObjectId;
            scrollArea->contentOffset = (Vector2) { 0, 0 };
        }
    } else {
        scrollArea = DuskGui_getEntryById(scrollAreaId);
        scrollArea->contentOffset = (Vector2) { 0, 0 };
    }

    DuskGui_endScrollArea(scrollAreaId);
    Editor_endPanelWindow(panel, &state->inspectorPanelBounds);
}

void Editor_drawControls(EditorState* state, SceneGraph *graph)
{
    float w = GetScreenWidth();
    int btnCnt = 4;
    float padding = 5.0f;
    float btnS = 20.0f;
    float spacing = 4.0f;
    float textWidth = 180.0f;
    float cw = btnS * btnCnt + spacing * (btnCnt) + padding * 2.0f + textWidth;
    DuskGuiParamsEntryId panel = DuskGui_beginPanel((DuskGuiParams){
        .bounds = (Rectangle){(w-cw)*.5f,-5.0f,cw, 35.0f},
        .rayCastTarget = 1,
        .text = "##editor_controls_panel"
    });

    float x = padding;
    float y = padding + 5;
    if (DuskGui_button((DuskGuiParams){
        .text = "##ctrl-Hierarchy",
        .bounds = (Rectangle){x,y,btnS,btnS},
        .rayCastTarget = 1,
        .icon = {
            .texture = _editor_iconHierarchy,
            .src.width = 16,
            .src.height = 16,
            .dst.x = 2,
            .dst.y = 2,
            .dst.width = 16,
            .dst.height = 16,
            .color = WHITE
        }
    }))
    {
        state->displayHierarchy = !state->displayHierarchy;
    }

    x += btnS + spacing;
    if (DuskGui_button((DuskGuiParams){
        .text = "##ctrl-Inspector",
        .bounds = (Rectangle){x,y,btnS,btnS},
        .rayCastTarget = 1,
        .icon = {
            .texture = _editor_iconInspector,
            .src.width = 16,
            .src.height = 16,
            .dst.x = 2,
            .dst.y = 2,
            .dst.width = 16,
            .dst.height = 16,
            .color = WHITE
        }
    }))
    {
        state->displayInspector = !state->displayInspector;
    }

    x += btnS + spacing;

    char buffer[64];
    sprintf(buffer, "%4d | %.3fs | %d FPS", state->frameCount, state->gameTime, GetFPS());
    DuskGui_label((DuskGuiParams){
        .text = buffer,
        .bounds = (Rectangle){x,y,textWidth,btnS},
        .rayCastTarget = 1,
        .styleGroup = &_editor_labelCenteredStyleGroup
    });

    x += textWidth + spacing;
    if (DuskGui_button((DuskGuiParams){
        .text = "##ctrl-Pause",
        .bounds = (Rectangle){x,y,btnS,btnS},
        .rayCastTarget = 1,
        .icon = {
            .texture = state->paused ? _editor_iconPlay : _editor_iconPause,
            .src.width = 16,
            .src.height = 16,
            .dst.x = 2,
            .dst.y = 2,
            .dst.width = 16,
            .dst.height = 16,
            .color = WHITE
        }
    }))
    {
        state->paused = !state->paused;
    }

    x += btnS + spacing;
    if (DuskGui_button((DuskGuiParams){
        .text = "##ctrl-Step",
        .bounds = (Rectangle){x,y,btnS,btnS},
        .rayCastTarget = 1,
        .icon = {
            .texture = _editor_iconStep,
            .src.width = 16,
            .src.height = 16,
            .dst.x = 2,
            .dst.y = 2,
            .dst.width = 16,
            .dst.height = 16,
            .color = WHITE
        }
    }))
    {
        state->paused = true;
        state->stepped = true;
    }

    DuskGui_endPanel(panel);
}