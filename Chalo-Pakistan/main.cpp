#include <SFML/Graphics.hpp>
#include <iostream>
#include "Screens.hpp"
#include "AppState.hpp"

using namespace sf;
using namespace std;

int main()
{
    // Create SFML window
    RenderWindow window(VideoMode(360, 640), "Chalo Pakistan", Style::Titlebar | Style::Close);
    window.setFramerateLimit(60);

    // STEP 1 — WELCOME SCREEN
    bool start = runWelcomeScreen(window);
    if (!start) {
        cout << "Application exited from Welcome Screen." << endl;
        return 0;
    }

    // STEP 2 — LOGIN SCREEN
    string role = runLoginScreen(window);
    if (role == "EXIT") {
        cout << "User exited at Login Screen." << endl;
        return 0;
    }

    // STEP 3 — ROUTE BASED SCREENS
    if (role == "TRAVELLER") {
        cout << "\nLogged in as Traveller: " << gAppState.traveller.getUsername() << endl;
        runTransportScreen(window);
    }
    else if (role == "ADMIN") {
        cout << "\nLogged in as Admin: " << gAppState.admin.getUsername() << endl;
        runAdminScreen(window);
    }

    cout << "\nApplication Closed." << endl;
    return 0;
}
