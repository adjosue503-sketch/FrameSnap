#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

class $modify(FrameSnapLayer, GJBaseGameLayer) {
    struct Fields {
        bool pending = false;
        bool releaseAfter = false;
        bool applying = false;
        bool isP1 = true;
        float age = 0.f;
    };

    bool canAct(PlayerObject* p) {
        if (!p) return false;
        if (p->m_isOnGround) return true;
        return p->m_touchingRings && p->m_touchingRings->count() > 0;
    }

    bool active() {
        return Mod::get()->getSettingValue<bool>("enabled")
            && typeinfo_cast<PlayLayer*>(this) != nullptr;
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        auto f = m_fields.self();

        if (!active() || f->applying || button != 1) {
            GJBaseGameLayer::handleButton(down, button, isPlayer1);
            return;
        }

        if (down) {
            auto p = isPlayer1 ? m_player1 : m_player2;
            if (canAct(p)) {
                GJBaseGameLayer::handleButton(true, button, isPlayer1);
            } else {
                f->pending = true;
                f->releaseAfter = false;
                f->isP1 = isPlayer1;
                f->age = 0.f;
            }
        } else {
            if (f->pending && f->isP1 == isPlayer1) {
                f->releaseAfter = true;
            } else {
                GJBaseGameLayer::handleButton(false, button, isPlayer1);
            }
        }
    }

    void processCommands(float dt) {
        auto f = m_fields.self();

        if (f->pending && active()) {
            f->age += dt;
            float window = Mod::get()->getSettingValue<int64_t>("window-ms") / 1000.f;
            auto p = f->isP1 ? m_player1 : m_player2;

            if (canAct(p) || f->age > window) {
                f->applying = true;
                GJBaseGameLayer::handleButton(true, 1, f->isP1);
                if (f->releaseAfter) {
                    GJBaseGameLayer::handleButton(false, 1, f->isP1);
                }
                f->applying = false;
                f->pending = false;
            }
        }

        GJBaseGameLayer::processCommands(dt);
    }
};
