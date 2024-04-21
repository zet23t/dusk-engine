#include <raylib.h>

SERIALIZABLE_STRUCT_START(TestStruct)
    SERIALIZABLE_FIELD(int, iValue)
    SERIALIZABLE_FIELD(Vector3, v3Value)
SERIALIZABLE_STRUCT_END(TestStruct)