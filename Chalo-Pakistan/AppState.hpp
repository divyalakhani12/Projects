#ifndef APPSTATE_HPP
#define APPSTATE_HPP

#include "ChaloPakistan.hpp"

enum class AppRole {
    None,
    Traveller,
    Admin
};

struct AppState {
    AppRole role;
    Traveller traveller;
    Admin admin;

    AppState() : role(AppRole::None) {}
};

extern AppState gAppState;

#endif
