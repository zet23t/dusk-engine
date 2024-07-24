#include <inttypes.h>
#include <raylib.h>
#include <stddef.h>

SERIALIZABLE_STRUCT_START(SceneObjectTransform)
    SERIALIZABLE_FIELD(Vector3, position)
    SERIALIZABLE_FIELD(Vector3, eulerRotationDegrees)
    SERIALIZABLE_FIELD(Vector3, scale)
    NONSERIALIZED_FIELD(Matrix, localMatrix)
    NONSERIALIZED_FIELD(Matrix, worldMatrix)
    NONSERIALIZED_FIELD(int32_t, worldMatrixVersion)
SERIALIZABLE_STRUCT_END(SceneObjectTransform)

SERIALIZABLE_STRUCT_START(SceneObjectId)
    SERIALIZABLE_FIELD(uint32_t, id)
    SERIALIZABLE_FIELD(uint32_t, version)
SERIALIZABLE_STRUCT_END(SceneObjectId)

SERIALIZABLE_STRUCT_START(SceneComponentTypeId)
    SERIALIZABLE_FIELD(uint32_t, id)
    SERIALIZABLE_FIELD(uint32_t, version)
SERIALIZABLE_STRUCT_END(SceneComponentTypeId)

SERIALIZABLE_STRUCT_START(SceneComponentId)
    SERIALIZABLE_FIELD(uint32_t, id)
    SERIALIZABLE_FIELD(uint32_t, version)
    SERIALIZABLE_FIELD(SceneComponentTypeId, typeId)
SERIALIZABLE_STRUCT_END(SceneComponentId)


SERIALIZABLE_STRUCT_START(SceneComponentTypeMethods)
    NONSERIALIZED_FIELD(void, (*onInitialize)(SceneObject* sceneObject, SceneComponentId SceneComponent, void* componentData, void* initArg))
    NONSERIALIZED_FIELD(void, (*updateTick)(SceneObject* sceneObject, SceneComponentId SceneComponent,
        float delta, void* componentData))
    NONSERIALIZED_FIELD(void, (*updateSystem)(SceneGraph *graph, SceneComponentType type, float delta))
    NONSERIALIZED_FIELD(void, (*drawSystem)(Camera3D camera, SceneGraph* graph, SceneComponentType type, void* userdata))
    NONSERIALIZED_FIELD(void, (*draw)(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
        void* componentData, void* userdata))
    NONSERIALIZED_FIELD(void, (*draw2D)(Camera2D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
        void* componentData, void* userdata))
    NONSERIALIZED_FIELD(void, (*sequentialDraw)(Camera3D camera, SceneObject* sceneObject, SceneComponentId sceneComponent,
        void* componentData, void* userdata))
    NONSERIALIZED_FIELD(void, (*onDestroy)(SceneObject* sceneObject, SceneComponentId sceneComponent, void* componentData))

    NONSERIALIZED_FIELD(int, (*getValue)(SceneObject* sceneObject, SceneComponent* sceneComponent, void* componentData, char* name, int bufferSize, void* buffer))
    NONSERIALIZED_FIELD(int, (*setValue)(SceneObject* sceneObject, SceneComponent* sceneComponent, void* componentData, char* name, int bufferSize, void* buffer))
SERIALIZABLE_STRUCT_END(SceneComponentTypeMethods)


SERIALIZABLE_STRUCT_START(SceneComponent)
    SERIALIZABLE_FIELD(SceneComponentId, id)
    SERIALIZABLE_FIELD(SceneObjectId, objectId)
    SERIALIZABLE_FIELD(SceneComponentTypeId, typeId)
    SERIALIZABLE_CSTR(name)
    SERIALIZABLE_FIELD(uint32_t, flags)
SERIALIZABLE_STRUCT_END(SceneComponent)

SERIALIZABLE_STRUCT_START(SceneComponentType)
    SERIALIZABLE_FIELD(SceneComponentTypeId, id)
    SERIALIZABLE_CSTR(name)
    SERIALIZABLE_FIELD(size_t, dataSize)
    SERIALIZABLE_STRUCT_LIST_ELEMENT(SceneComponent, components)
    NONSERIALIZABLE_STRUCT_LIST_ELEMENT(uint8_t, componentData)
    NONSERIALIZED_FIELD(SceneComponentTypeMethods, methods)
    POST_SERIALIZE_CALLBACK(SceneComponentType, SceneComponentType_onSerialized)
SERIALIZABLE_STRUCT_END(SceneComponentType)

SERIALIZABLE_STRUCT_START(SceneObject)
    SERIALIZABLE_FIELD(SceneObjectId, id)
    NONSERIALIZED_FIELD(SceneGraph*, graph)
    SERIALIZABLE_CSTR(name)
    SERIALIZABLE_FIELD(int32_t, flags)
    NONSERIALIZED_FIELD(int32_t, marker)
    NONSERIALIZED_FIELD(int32_t, parentWorldMatrixVersion)
    SERIALIZABLE_FIELD(SceneObjectId, parent)
    SERIALIZABLE_FIELD(SceneObjectTransform, transform)

    SERIALIZABLE_STRUCT_LIST_ELEMENT(SceneObjectId, children)
    SERIALIZABLE_STRUCT_LIST_ELEMENT(SceneComponentId, components)
SERIALIZABLE_STRUCT_END(SceneObject)

SERIALIZABLE_STRUCT_START(SceneGraph)
    SERIALIZABLE_FIELD(int32_t, versionCounter)
    SERIALIZABLE_FIELD(int32_t, markerCounter)
    SERIALIZABLE_STRUCT_LIST_ELEMENT(SceneObject, objects)
    SERIALIZABLE_STRUCT_LIST_ELEMENT(SceneComponentType, componentTypes)
    POST_DESERIALIZE_CALLBACK(SceneGraph, SceneGraph_onDeserialized)
SERIALIZABLE_STRUCT_END(SceneGraph)