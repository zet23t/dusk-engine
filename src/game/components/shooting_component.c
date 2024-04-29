#include "../game_g.h"

static void ShootingComponentUpdate(SceneObject* node, SceneComponentId sceneComponentId, float dt, void* component)
{
    ShootingComponent* shooting = (ShootingComponent*)component;
    shooting->cooldown -= psg.deltaTime;
    if (shooting->cooldown <= 0 && shooting->shooting) {
        shooting->cooldown = shooting->config.shootInterval;
        shooting->shooting &= ~1;
        shooting->config.onShoot(node->graph, sceneComponentId, shooting, &shooting->config);
    }
}