#ifndef TE_ECS_H
#define TE_ECS_H

#include <glm/glm.hpp>

#include <memory>
#include <string>

namespace te
{
    struct TMX;

    class Shader;

    class TextureManager;
    class MeshManager;
    class AnimationFactory;

    struct AssetManager {
        const std::shared_ptr<TextureManager> pTextureManager;
        const std::shared_ptr<MeshManager> pMeshManager;
        const std::shared_ptr<AnimationFactory> pAnimationFactory;

        AssetManager(TMX&&);
        AssetManager(std::shared_ptr<const TMX>);
    };

    class TransformComponent;
    class AnimationComponent;
    class DataComponent;

    class EntityManager;

    class RenderSystem;

    struct ECS {
        ECS(std::shared_ptr<Shader>);

        const std::shared_ptr<TransformComponent> pTransformComponent;
        const std::shared_ptr<AnimationComponent> pAnimationComponent;
        const std::shared_ptr<DataComponent> pDataComponent;

        const std::shared_ptr<EntityManager> pEntityManager;

        const std::shared_ptr<RenderSystem> pRenderSystem;
    };

    void update(const ECS&, float dt);
    void draw(const ECS&, const glm::mat4& viewTransform = glm::mat4());

    class LuaStateECS {
    public:
        LuaStateECS(const ECS&);
        ~LuaStateECS();

        void loadScript(const std::string& path) const;
        void runScript() const;
        void runConsole() const;
    private:
        // May add lots of scripting, so avoid recompilation
        // with private implementation.
        struct Impl;
        // Use of unique_ptr causes awkward compilation errors
        Impl* mpImpl;
    };
}

#endif