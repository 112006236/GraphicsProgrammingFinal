#define GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
// Student-owned renderer hooks: implement the outdoor specification here.
#include "SceneRenderer.h"
#include "MyCameraManager.h"
#include "terrain/MyTerrain.h"
#include "ViewFrustumSceneObject.h"

static SceneRenderer renderer;
extern ShaderProgram* defaultShaderProgram;

bool outdoorInitialize(GLuint) {
    return renderer.initialize(1920, 1080, defaultShaderProgram);
}

void outdoorGui(INANOA::MyCameraManager*) {}

void outdoorDestroy() {}

void outdoorSmokeConfigure(int, INANOA::MyCameraManager*) {}

bool outdoorSmokeCheck(int) {
    return true;
}

void outdoorRender(MyTerrain* terrain, ViewFrustumSceneObject* frustum,
                   INANOA::MyCameraManager* cameras) {
    static bool initialized = false;
    if (!initialized) {
        renderer.appendTerrainSceneObject(terrain->sceneObject());
        renderer.appendDynamicSceneObject(frustum->sceneObject());
        initialized = true;
    }

    defaultShaderProgram->useProgram();

    // View 0 is the god view. View 1 is the player view.
    const int firstView = cameras->playerOnly() ? 1 : 0;
    for (int i = firstView; i < 2; i++) {
        auto vp = i ? cameras->playerViewport() : cameras->godViewport();
        renderer.setViewport(vp.x, vp.y, vp.z, vp.w);
        renderer.setView(i ? cameras->playerViewMatrix() : cameras->godViewMatrix());
        renderer.setProjection(i ? cameras->playerProjectionMatrix()
                                 : cameras->godProjectionMatrix());
        renderer.renderPass();
    }
}

// Optional renderer-history reset hook for instructor-host integration.
void outdoorReset() {}
