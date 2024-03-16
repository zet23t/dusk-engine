#include "plane_sim_g.h"

#include <stdio.h>

void GameUi_Init()
{
    SceneObjectId uiPlaneId = SceneGraph_createObject(psg.sceneGraph, "ui-plane");
    psg.uiRootId = uiPlaneId;
    SceneGraph_setParent(psg.sceneGraph, uiPlaneId, psg.camera);
    SceneGraph_addComponent(psg.sceneGraph, uiPlaneId, psg.PrimitiveRendererComponentId, &(PrimitiveRendererComponent) {
                                                                                             .primitiveType = PRIMITIVE_TYPE_CUBE,
                                                                                             .size = (Vector3) { 15, 10, 0 },
                                                                                             .color = (Color) { 255, 255, 255, 255 },
                                                                                             .isWireframe = 1,
                                                                                         });
    float uiDistance = 70.0f;
    SceneGraph_setLocalPosition(psg.sceneGraph, uiPlaneId, (Vector3) { 0, 0, uiDistance });
    SceneGraph_addComponent(psg.sceneGraph, uiPlaneId, psg.AspectRatioMatcherComponentId, &(AspectRatioMatcherComponent) {
                                                                                              .canvasDistance = uiDistance,
                                                                                              .width = 16.5f,
                                                                                              .height = 10.5f,
                                                                                          });

    // Vector3 planePos = SceneGraph_getWorldPosition(psg.sceneGraph, psg.playerPlane);
    // SceneGraph_setLocalPosition(psg.sceneGraph, uiPlaneId, (Vector3) { planePos.x, planePos.y, planePos.z });

    // printf("position: %f, %f, %f\n", planePos.x, planePos.y, planePos.z);
    for (int side = 0; side < 4; side++) {

        SceneObjectId uiBorderId = SceneGraph_createObject(psg.sceneGraph, "ui-border-side");
        SceneGraph_setParent(psg.sceneGraph, uiBorderId, uiPlaneId);
        float x = side % 2 == 0 ? 1 : -1;
        float y = side < 3 ? 1 : -1;
        if (side < 2) {
            y = 0;
        } else {
            x = 0;
        }
        SceneGraph_setLocalPosition(psg.sceneGraph, uiBorderId, (Vector3) { 9 * x, y * 6, fabsf(y) });
        SceneGraph_setLocalRotation(psg.sceneGraph, uiBorderId, (Vector3) { 0, 0, 180 * side + (side >= 2 ? 90 : 0) });
        for (int i = -3; i <= 3; i += 1) {
            SceneObjectId element = SceneGraph_createObject(psg.sceneGraph, "ui-border-element");
            SceneGraph_setParent(psg.sceneGraph, element, uiBorderId);
            SceneGraph_setLocalPosition(psg.sceneGraph, element, (Vector3) { 0, i * 4, 0 });
            AddMeshRendererComponentByName(element, "ui-border", 1.0f);
        }
    }
    SceneObjectId landscapeText = SceneGraph_createObject(psg.sceneGraph, "landscape-text");
    SceneGraph_setParent(psg.sceneGraph, landscapeText, uiPlaneId);
    SceneGraph_setLocalPosition(psg.sceneGraph, landscapeText, (Vector3) { 0, 8.5f, 0 });
    SceneGraph_addComponent(psg.sceneGraph, landscapeText, psg.textComponentId, &(TextComponent) {
                                                                                    .text = "Play in Landscape",
                                                                                    .fontSize = 15,
                                                                                    .fontSpacing = 1,
                                                                                    .align = 0.5f,
                                                                                    .color = (Color) { 255, 255, 255, 255 },
                                                                                });
    
    // SceneGraph_setLocalPosition(psg.sceneGraph, uiPlaneId, (Vector3) { 0, 0, 0 });
}