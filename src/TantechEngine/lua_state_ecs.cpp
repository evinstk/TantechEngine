#include "ecs.h"

#include "entity_manager.h"
#include "data_component.h"
#include "transform_component.h"
#include "command_component.h"
#include "camera.h"
#include "command_system.h"
#include "commands.h"

#include <lua.hpp>
#include <LuaBridge.h>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include <functional>
#include <iostream>

namespace te
{
    struct LuaStateECS::Impl {
        // members
        std::unique_ptr<lua_State, std::function<void(lua_State*)>> pL;
        ECS ecs;
        ECSWatchers ecsWatchers;
        luabridge::LuaRef mainRef;

        // methods for Lua
        Entity getEntity(unsigned tiledId)
        {
            return ecs.pDataComponent->getEntity(tiledId);
        }
        void cameraFollow(const Entity& entity)
        {
            ecsWatchers.pCamera->follow(entity);
        }
        void setTypeMask(const Entity& entity, int typeMask)
        {
            ecs.pCommandComponent->setTypeMask(entity, typeMask);
        }
        void setKeyPress(const std::string& ch, int fnID, luabridge::LuaRef args, int mask)
        {
            assert(ch.length() == 1);
            ecsWatchers.pInputSystem->setKeyPress(ch[0], Command(mask, 0, getFunction((FuncID)fnID)(args)));
        }

        glm::mat4 translatef(const Entity& entity, float x, float y, float z)
        {
            return ecs.pTransformComponent->multiplyTransform(entity, glm::translate(glm::vec3(x, y, z)));
        }
        glm::mat4 translateWorldf(const Entity& entity, float x, float y, float z)
        {
            return ecs.pTransformComponent->multiplyTransform(entity, glm::translate(glm::vec3(x, y, z)), TransformComponent::Space::WORLD);
        }
        glm::mat4 translatev(const Entity& entity, glm::vec3 translation)
        {
            return ecs.pTransformComponent->multiplyTransform(entity, glm::translate(translation));
        }
        glm::mat4 translateWorldv(const Entity& entity, glm::vec3 translation)
        {
            return ecs.pTransformComponent->multiplyTransform(entity, glm::translate(translation), TransformComponent::Space::WORLD);
        }
        glm::mat4 scalef(const Entity& entity, float x, float y, float z)
        {
            return ecs.pTransformComponent->multiplyTransform(entity, glm::scale(glm::vec3(x, y, z)));
        }
        glm::mat4 scalev(const Entity& entity, glm::vec3 scale)
        {
            return ecs.pTransformComponent->multiplyTransform(entity, glm::scale(scale));
        }

        void printEntities()
        {
            ecs.pDataComponent->forEach([](const Entity& entity, const DataInstance& instance) {
                std::cout << instance.id << ": " << instance.name << std::endl;
            });
        }

        void destroyEntity(const Entity& entity)
        {
            ecs.pEntityManager->destroy(entity);
        }

        // constructors
        Impl(const ECS& ecs, const ECSWatchers& watchers)
            : pL(luaL_newstate(),
                 [](lua_State* L) { lua_close(L); })
            , ecs(ecs)
            , ecsWatchers(watchers)
            , mainRef(luabridge::LuaRef(pL.get()))
        {
            lua_State* L = pL.get();
            luaL_openlibs(L);
            luabridge::getGlobalNamespace(L)
                .beginNamespace("tt")

                    .beginClass<Entity>("Entity")
                    .endClass()

                    .beginClass<glm::vec3>("vec3")
                        .addConstructor<void(*)(float, float, float)>()
                    .endClass()
                    .beginClass<glm::mat4>("mat4")
                    .endClass()

                    .beginClass<Impl>("State")
                        .addFunction("getEntity", &Impl::getEntity)
                        .addFunction("cameraFollow", &Impl::cameraFollow)
                        .addFunction("destroyEntity", &Impl::destroyEntity)
                        .addFunction("setTypeMask", &Impl::setTypeMask)
                        .addFunction("setKeyPress", &Impl::setKeyPress)
                        .addFunction("translatef", &Impl::translatef)
                        .addFunction("translateWorldf", &Impl::translateWorldf)
                        .addFunction("translatev", &Impl::translatev)
                        .addFunction("translateWorldv", &Impl::translateWorldv)
                        .addFunction("scalef", &Impl::scalef)
                        .addFunction("scalev", &Impl::scalev)
                        .addFunction("printEntities", &Impl::printEntities)
                    .endClass()

                .endNamespace();

            luabridge::push(L, this);
            lua_setglobal(L, "te");
            luabridge::push(L, (int)FuncID::MOVE);
            lua_setglobal(L, "TE_MOVE");
        }
    };

    LuaStateECS::LuaStateECS(const ECS& ecs, const ECSWatchers& watchers)
        : mpImpl(new Impl(ecs, watchers)) {}
    LuaStateECS::~LuaStateECS()
    {
        delete mpImpl;
    }

    void LuaStateECS::loadScript(const std::string& path) const
    {
        lua_State* L = mpImpl->pL.get();
        int status = luaL_dofile(L, path.c_str());
        if (status) {
            throw std::runtime_error("LuaStateECS::loadScript: Could not load Lua script.");
        }

        luabridge::LuaRef mainRef(luabridge::getGlobal(L, "main"));
        if (mainRef.isNil() || !mainRef.isFunction()) {
            throw std::runtime_error("LuaStateECS::loadScript: Could not find main function.");
        }

        mpImpl->mainRef = mainRef;
    }

    void LuaStateECS::runScript() const
    {
        luabridge::LuaRef& mainRef = mpImpl->mainRef;
        if (!mainRef.isNil()) {
            mainRef();
        } else {
            throw std::runtime_error("LuaStateECS::runScript: No Lua script loaded.");
        }
    }

    void LuaStateECS::runConsole() const
    {
        lua_State* L = mpImpl->pL.get();
        int status = luaL_dofile(L, "assets/lua/console.lua");
        if (status) {
            throw std::runtime_error("LuaGameState::runConsole: Could not load console.lua");
        }
        luabridge::LuaRef mainRef(luabridge::getGlobal(L, "main"));
        if (mainRef.isNil()) {
            throw std::runtime_error("LuaGameState::runConsole: Could not load console.lua main function.");
        }

        while (true) {
            try {
                mainRef();
            }
            catch (const luabridge::LuaException& ex) {
                std::cerr << ex.what() << std::endl;
            }
        }
    }
}
