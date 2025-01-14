// Copyright (c) 2008-2023 the Urho3D project
// Copyright (c) 2022-2023 the Dviglo project
// License: MIT

#pragma once

#include "appstate_base.h"

#include <dviglo/containers/hash_map.h>

using AppStateId = u32;

inline constexpr AppStateId APPSTATEID_NULL = 0;
inline constexpr AppStateId APPSTATEID_MAINSCREEN = 1;
inline constexpr AppStateId APPSTATEID_RESULTSCREEN = 2;
inline constexpr AppStateId APPSTATEID_BENCHMARK01 = 3;
inline constexpr AppStateId APPSTATEID_BENCHMARK02 = 4;
inline constexpr AppStateId APPSTATEID_BENCHMARK03 = 5;
inline constexpr AppStateId APPSTATEID_BENCHMARK04 = 6;

class AppStateManager : public Object
{
public:
    DV_OBJECT(AppStateManager);

    /// Только App может создать и уничтожить объект
    friend class App;

private:
    /// Инициализируется в конструкторе
    inline static AppStateManager* instance_ = nullptr;

public:
    static AppStateManager* instance() { return instance_; }

private:
    HashMap<AppStateId, SharedPtr<AppState_Base>> appStates_;
    AppStateId currentAppStateId_ = APPSTATEID_NULL;
    AppStateId previousAppStateId_ = APPSTATEID_NULL;
    AppStateId requiredAppStateId_ = APPSTATEID_NULL;

    AppStateManager();
    ~AppStateManager();

public:
    // Запрещаем копирование
    AppStateManager(const AppStateManager&) = delete;
    AppStateManager& operator =(const AppStateManager&) = delete;

    AppStateId GetCurrentAppStateId() const { return currentAppStateId_; }
    AppStateId GetPreviousAppStateId() const { return previousAppStateId_; }

    AppStateId GetRequiredAppStateId() const { return requiredAppStateId_; }
    void SetRequiredAppStateId(AppStateId id) { requiredAppStateId_ = id; }

    // Change state if currentAppStateId_ != requiredAppStateId_
    void Apply();

    const String& GetName(AppStateId appStateId) const
    {
        auto it = appStates_.Find(appStateId);
        assert(it != appStates_.End());
        return it->second_->GetName();
    }

    const FpsCounter& GetResult(AppStateId appStateId) const
    {
        auto it = appStates_.Find(appStateId);
        assert(it != appStates_.End());
        return it->second_->GetResult();
    }

    void ClearAllResults()
    {
        for (auto& pair : appStates_)
            pair.second_->ClearResult();
    }
};

#define APP_STATE_MANAGER (AppStateManager::instance())
