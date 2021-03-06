#ifndef TE_ANIMATION_FACTORY
#define TE_ANIMATION_FACTORY

#include <memory>
#include <vector>
#include <map>

namespace te
{
    struct TMX;
    class Model;
    class MeshManager;

    struct Frame {
        std::shared_ptr<const Model> model;
        unsigned duration;
    };

    struct Animation {
        std::vector<Frame> frames;
        bool frozen;
    };

    class AnimationFactory
    {
    public:
        AnimationFactory(std::shared_ptr<const TMX> pTMX, std::shared_ptr<MeshManager> pMeshManager);

        Animation create(unsigned gid, bool frozen = false) const;
        Animation create(const std::map<std::string, std::string>& propertyMap, bool frozen = false) const;
    private:
        std::shared_ptr<const TMX> mpTMX;
        std::shared_ptr<MeshManager> mpMeshManager;
    };
}

#endif
