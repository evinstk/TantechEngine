#include "lua_game_state.h"

#include "tmx.h"
#include "shader.h"
#include "tiled_map.h"
#include "camera.h"
#include "command_system.h"
#include "view.h"

#include <glm/gtx/transform.hpp>
#include <SDL_events.h>

#include <iostream>
#include <cassert>

namespace te
{
    LuaGameState::LuaGameState(const std::shared_ptr<const TMX>& pTMX, const std::shared_ptr<Shader>& pShader, const glm::mat4& model)
        : LuaGameState(pTMX, pShader, model, AssetManager(pTMX))
    {}
    LuaGameState::LuaGameState(const std::shared_ptr<const TMX>& pTMX, const std::shared_ptr<Shader>& pShader, const glm::mat4& model, const AssetManager& assets)
        : GameState()
        , mAssets(assets)
        , mpTiledMap(new TiledMap(pTMX, pShader, model, assets.pTextureManager.get()))
        , mECS()
        , mECSWatchers(mECS, pShader)
        , mLuaStateECS(mECS, mECSWatchers)
    {
        assert(pTMX && pShader);

        loadObjects(*pTMX, model, mAssets, mECS);
        try {
            mLuaStateECS.loadScript(pTMX->meta.path + "/main.lua");
            mLuaStateECS.runScript();
        } catch (const std::runtime_error& ex) {
            std::clog << "Warning: LuaGameState: Could not load main.lua." << std::endl;
            std::clog << ex.what() << std::endl;
        }
    }

    bool LuaGameState::processInput(const SDL_Event& evt) {
        if (evt.type == SDL_KEYDOWN) {
            te::processInput(mECSWatchers, evt.key.keysym.sym, InputType::PRESS);
        } else if (evt.type == SDL_KEYUP) {
            te::processInput(mECSWatchers, evt.key.keysym.sym, InputType::RELEASE);
        }
        return false;
    }
    bool LuaGameState::update(float dt)
    {
        te::update(mECSWatchers, dt);
        return false;
    }
    void LuaGameState::draw()
    {
        mpTiledMap->draw(mECSWatchers.pCamera->getView());
        te::draw(mECSWatchers);
    }

    void LuaGameState::runConsole()
    {
        mLuaStateECS.runConsole();
    }
}
