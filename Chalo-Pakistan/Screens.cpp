#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include "Screens.hpp"
#include "AppState.hpp"

using namespace std;
using namespace sf;
using std::string;

// UI HELPERS 

class Button {
public:
    RectangleShape rect;
    Text label;

    Button() = default;

    Button(const Font& font, const string& text,
           Vector2f pos, Vector2f size) 
    {
        rect.setSize(size);
        rect.setPosition(pos);
        rect.setOutlineThickness(1.f);
        rect.setOutlineColor(Color::White);
        rect.setFillColor(Color(60, 60, 60));

        label.setFont(font);
        label.setString(text);
        label.setCharacterSize(18);
        label.setFillColor(Color::White);

        //center text
        FloatRect bounds = label.getLocalBounds();
        label.setOrigin(bounds.left + bounds.width / 2.f,
                        bounds.top + bounds.height / 2.f);
        label.setPosition(
            pos.x + size.x / 2.f,
            pos.y + size.y / 2.f
        );
    }

    bool contains(Vector2f point) const {
        return rect.getGlobalBounds().contains(point);}

    void draw(RenderWindow& win) {
        win.draw(rect);
        win.draw(label);}};

class InputField {
public:
    RectangleShape rect;
    Text text;
    bool focused = false;
    bool password = false;
    string value;

    InputField() = default;

    InputField(const Font& font, Vector2f pos, Vector2f size, bool isPassword = false)
        : password(isPassword)
    {
        rect.setSize(size);
        rect.setPosition(pos);
        rect.setOutlineThickness(1.f);
        rect.setOutlineColor(Color::White);
        rect.setFillColor(Color(30, 30, 30));

        text.setFont(font);
        text.setCharacterSize(18);
        text.setFillColor(Color::White);
        text.setPosition(pos.x + 5.f, pos.y + 5.f);
    }

    bool contains(Vector2f p) const {
        return rect.getGlobalBounds().contains(p);
    }

    void setFocused(bool f) {
        focused = f;
        rect.setOutlineColor(f ? Color::Cyan : Color::White);
    }

    void handleTextEntered(const Event::TextEvent& te) {
        if (!focused) return;

        uint32_t ch = te.unicode;
        if (ch == 8) { // backspace
            if (!value.empty())
                value.pop_back();
        }
        else if (ch >= 32 && ch < 127) {
            value.push_back(static_cast<char>(ch));
        }

        string display = password ? string(value.size(), '*') : value;
        text.setString(display);
    }

    void draw(RenderWindow& win) {
        win.draw(rect);
        win.draw(text);
    }

    const string& getValue() const {
        return value;
    }
};

//WELCOME SCREEN 

bool runWelcomeScreen(sf::RenderWindow &window)
{
    //Load font
    sf::Font font;
    if (!font.loadFromFile("MomoTrustDisplay-Regular.ttf")) {
        std::cout << "Failed to load font\n";
    }

    //Text
    sf::Text welcomeText("Welcome to", font, 28);
    sf::Text title("Chalo Pakistan", font, 46);
    sf::Text subtitle("Pakistan Tour Service", font, 24);

    welcomeText.setFillColor(sf::Color::White);
    title.setFillColor(sf::Color::White);
    subtitle.setFillColor(sf::Color::White);

    auto winSize = window.getSize();
    float centerX = winSize.x / 2.f;

    //center text horizontally
    auto centerText = [centerX](sf::Text &t, float y) {
        sf::FloatRect b = t.getLocalBounds();
        t.setPosition(centerX - b.width / 2.f, y);
    };

    centerText(welcomeText, 130.f);
    centerText(title,       190.f);
    centerText(subtitle,    250.f);

    //Train image
    sf::Texture trainTexture;
    if (!trainTexture.loadFromFile("next button.png")) {
        std::cout << "Failed to load train.png\n";
    }

    sf::Sprite train(trainTexture);

    //scale image in width
    float targetWidth  = winSize.x * 0.6f;              //60% of window width
    float scale        = targetWidth / trainTexture.getSize().x;
    train.setScale(scale, scale);

    //position near bottom, centered
    float trainWidth  = trainTexture.getSize().x * scale;
    float trainHeight = trainTexture.getSize().y * scale;
    float trainX      = centerX - trainWidth / 2.f;
    float trainY      = winSize.y - trainHeight - 40.f; //40px margin from bottom
    train.setPosition(trainX, trainY);

    //Loop
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
                return false;
            }

            //keyboard shortcut
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                    return false;
                }
                if (event.key.code == sf::Keyboard::Enter ||
                    event.key.code == sf::Keyboard::Space)
                {
                    return true;   //go to login
                }
            }

            //mouse click 
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                sf::Vector2f mousePos(
                    static_cast<float>(event.mouseButton.x),
                    static_cast<float>(event.mouseButton.y)
                );

                if (train.getGlobalBounds().contains(mousePos)) {
                    //train acts as a button
                    return true;
                }
            }
        }

        //sky blue background
        window.clear(sf::Color(135, 206, 250));

        window.draw(welcomeText);
        window.draw(title);
        window.draw(subtitle);
        window.draw(train);

        window.display();
    }

    return false;
}

//ROUTE PREVIEW SCREEN
bool runRoutePreviewScreen(RenderWindow &window, int routeNumber, const string& transportType)
{
    Font font;
    font.loadFromFile("MomoTrustDisplay-Regular.ttf");

    //background
    Color bg(135,206,250);

    Text header(transportType + " Route Preview", font, 28);
    header.setFillColor(Color::White);
    header.setPosition(20, 40);

    //route description
    Text routeText("", font, 20);
    routeText.setFillColor(Color::White);
    routeText.setPosition(20, 150);

    // TRAIN ROUTES
    if (transportType == "Train")
    {
        if (routeNumber == 1)
            routeText.setString("Route A:\nKarachi -> Hyderabad -> \n Lahore -> Rawalpindi ->\n Gilgit -> Karachi");
        else if (routeNumber == 2)
            routeText.setString("Route B:\nKarachi -> Naran -> \n Kaghan -> Chilas ->\n Karachi");
        else if (routeNumber == 3)
            routeText.setString("Route C:\nKarachi -> Badin -> \n Sujawal -> Gwadar -> \n Karachi");
    }

    // BUS ROUTES
    else if (transportType == "Bus")
    {
        if (routeNumber == 1)
            routeText.setString("Route A:\nKarachi -> Islamabad -> \n Swat -> Kashmir -> Karachi");
        else if (routeNumber == 2)
            routeText.setString("Route B:\nKarachi -> Quetta -> \n Zhob -> Multan -> Karachi");
        else if (routeNumber == 3)
            routeText.setString("Route C:\nKarachi -> Gwadar -> \n Ormara -> Pasni -> Karachi");
    }

    //Buttons
    RectangleShape confirmBtn(Vector2f(140, 45));
    confirmBtn.setFillColor(Color(0, 90, 160));
    confirmBtn.setPosition(40, 500);

    Text confirmText("Confirm", font, 20);
    confirmText.setFillColor(Color::White);
    confirmText.setPosition(60, 505);

    RectangleShape backBtn(Vector2f(140, 45));
    backBtn.setFillColor(Color(60, 60, 60));
    backBtn.setPosition(200, 500);

    Text backText("Back", font, 20);
    backText.setFillColor(Color::White);
    backText.setPosition(240, 505);

    while (window.isOpen())
    {
        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed)
                return false;

            if (e.type == Event::MouseButtonPressed &&
                e.mouseButton.button == Mouse::Left)
            {
                Vector2f mp(e.mouseButton.x, e.mouseButton.y);

                if (confirmBtn.getGlobalBounds().contains(mp))
                    return true;   //user confirmed this route

                if (backBtn.getGlobalBounds().contains(mp))
                    return false;  //go back to schedule selection
            }
        }

        window.clear(bg);
        window.draw(header);
        window.draw(routeText);
        window.draw(confirmBtn);
        window.draw(confirmText);
        window.draw(backBtn);
        window.draw(backText);
        window.display();
    }

    return false;
}




//LOGIN SCREEN

std::string runLoginScreen(RenderWindow &window) {
    Font font;
    if (!font.loadFromFile("MomoTrustDisplay-Regular.ttf")) {
        std::cerr << "Failed to load font\n";
        return "EXIT";
    }

    Text title("Login", font, 28);
    title.setFillColor(Color::White);
    FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
    title.setPosition(180.f, 80.f);

    Text userLabel("Username:", font, 18);
    userLabel.setPosition(40.f, 140.f);
    Text passLabel("Password:", font, 18);
    passLabel.setPosition(40.f, 210.f);

    InputField userField(font, {40.f, 165.f}, {280.f, 30.f}, false);
    InputField passField(font, {40.f, 235.f}, {280.f, 30.f}, true);

    userField.setFocused(true);

    Button loginBtn(font, "Login", {40.f, 310.f}, {130.f, 45.f});
    Button backBtn(font, "Back", {190.f, 310.f}, {130.f, 45.f});

    Text msg("", font, 16);
    msg.setFillColor(Color::Red);
    msg.setPosition(40.f, 370.f);

    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) {
                window.close();
                return "EXIT";
            }
            if (e.type == Event::KeyPressed && e.key.code == Keyboard::Escape) {
                return "EXIT";
            }
            if (e.type == Event::MouseButtonPressed &&
                e.mouseButton.button == Mouse::Left)
            {
                Vector2f mp((float)e.mouseButton.x, (float)e.mouseButton.y);
                if (userField.contains(mp)) {
                    userField.setFocused(true);
                    passField.setFocused(false);
                }
                else if (passField.contains(mp)) {
                    passField.setFocused(true);
                    userField.setFocused(false);
                }
                else {
                    //Buttons
                    if (loginBtn.contains(mp)) {
                        string uname = userField.getValue();
                        string pwd   = passField.getValue();

                        if (uname.empty() || pwd.empty()) {
                            msg.setString("Please enter username and password");
                            continue;
                        }

                        User tmpUser;
                        if (!tmpUser.authenticate(uname, pwd)) {
                            msg.setString("Invalid credentials. Try again.");
                        } else {
                            // Copy into app state
                            string role = tmpUser.getRole();
                            if (role == "TRAVELLER") {
                                gAppState.role = AppRole::Traveller;
                                gAppState.traveller.setUsername(tmpUser.getUsername());
                                gAppState.traveller.setUserID(tmpUser.getUserID());
                                gAppState.traveller.setRole(tmpUser.getRole());
                                gAppState.traveller.setPassword(pwd);
                                return "TRAVELLER";
                            } 
                            else if (role == "ADMIN") {
                                gAppState.role = AppRole::Admin;
                                gAppState.admin.setUsername(tmpUser.getUsername());
                                gAppState.admin.setUserID(tmpUser.getUserID());
                                gAppState.admin.setRole(tmpUser.getRole());
                                gAppState.admin.setPassword(pwd);
                                return "ADMIN";
                            } 
                            else {
                                msg.setString("Unknown role in user.csv");
                            }
                        }
                    }
                    else if (backBtn.contains(mp)) {
                        return "EXIT";
                    }
                }
            }
            if (e.type == Event::TextEntered) {
                userField.handleTextEntered(e.text);
                passField.handleTextEntered(e.text);
            }
        }

        window.clear(sf::Color(135, 206, 250));
        window.draw(title);
        window.draw(userLabel);
        window.draw(passLabel);
        userField.draw(window);
        passField.draw(window);
        loginBtn.draw(window);
        backBtn.draw(window);
        window.draw(msg);
        window.display();
    }

    return "EXIT";
}

void runAdminBookingsScreen(RenderWindow &window, const string &mode)
{
    Font font;
    font.loadFromFile("MomoTrustDisplay-Regular.ttf");

    Color bg(135, 206, 250);

    Text header("", font, 26);
    header.setFillColor(Color::White);
    header.setPosition(20, 20);

    if (mode == "BUS_LIST")        header.setString("Bus Bookings");
    else if (mode == "TRAIN_LIST") header.setString("Train Bookings");
    else if (mode == "TOTAL_BUS")  header.setString("Total Bus Revenue");
    else if (mode == "TOTAL_TRAIN")header.setString("Total Train Revenue");

    //LOAD CSV
    vector<string> lines;
    ifstream file("admin_records.csv");
    string line;
    while (getline(file, line)) {
        if (!line.empty())
            lines.push_back(line);
    }

    //FILTERED LIST
    vector<string> filtered;

    long long totalSum = 0;

    for (auto &l : lines)
    {
        bool isBus   = l.find(",Bus,")   != string::npos;
        bool isTrain = l.find(",Train,") != string::npos;

        bool match =
            (mode == "BUS_LIST"   && isBus)   ||
            (mode == "TRAIN_LIST" && isTrain) ||
            (mode == "TOTAL_BUS"  && isBus)   ||
            (mode == "TOTAL_TRAIN"&& isTrain);

        if (!match) continue;

        //add to filtered list
        filtered.push_back(l);

        //extract COST
        stringstream ss(l);
        string part;
        int col = 0;
        int cost = 0;

        while (getline(ss, part, ',')) {
            col++;
            if (col == 9) {  //TOTAL_COST is column 9
                cost = stoi(part);
                break;
            }
        }
        totalSum += cost;
    }

    //UI ELEMENTS
    RectangleShape back(Vector2f(120, 45));
    back.setFillColor(Color(60, 60, 60));
    back.setPosition(20, 550);

    Text backTxt("Back", font, 20);
    backTxt.setFillColor(Color::White);
    backTxt.setPosition(55, 555);

    int scroll = 0;

    while (window.isOpen())
    {
        Event e;
        while (window.pollEvent(e))
        {
            if (e.type == Event::Closed) return;

            if (e.type == Event::MouseButtonPressed)
            {
                Vector2f mp(e.mouseButton.x, e.mouseButton.y);

                if (back.getGlobalBounds().contains(mp))
                    return;

                if (mode == "BUS_LIST" || mode == "TRAIN_LIST")
                {
                    if (e.mouseButton.button == Mouse::Left)
                        scroll = min(scroll + 1, (int)filtered.size() - 1);

                    if (e.mouseButton.button == Mouse::Right)
                        scroll = max(scroll - 1, 0);
                }
            }
        }

        //DRAW
        window.clear(bg);
        window.draw(header);
        window.draw(back);
        window.draw(backTxt);

        float y = 100;

        //SHOW SUM SCREEN
        if (mode == "TOTAL_BUS" || mode == "TOTAL_TRAIN")
        {
            Text sumText("Total Revenue: Rs. " + to_string(totalSum), font, 26);
            sumText.setFillColor(Color::White);
            sumText.setPosition(20, 150);
            window.draw(sumText);

            window.display();
            continue;
        }

        //SHOW LIST SCREEN
        if (filtered.empty())
        {
            Text t("No records found.", font, 20);
            t.setFillColor(Color::White);
            t.setPosition(20, 120);
            window.draw(t);
        }
        else
        {
            int maxVisible = 8;
            for (int i = scroll; i < scroll + maxVisible && i < filtered.size(); i++)
            {
                Text rec(filtered[i], font, 15);
                rec.setFillColor(Color::White);
                rec.setPosition(20, y);
                y += 35;
                window.draw(rec);
            }
        }

        window.display();
    }
}



void runThankYou(RenderWindow &window)
{
    Color bgColor(200, 230, 255);

    Font font;
    font.loadFromFile("MomoTrustDisplay-Regular.ttf");

    //Title
    Text title("Thank You!", font, 30);
    title.setFillColor(Color(0, 70, 140));
    FloatRect tBounds = title.getLocalBounds();
    title.setOrigin(tBounds.width / 2.f, tBounds.height / 2.f);
    title.setPosition(180, 200);

    //Subtitle
    Text subtitle("Your purchase has been confirmed.", font, 16);
    subtitle.setFillColor(Color(60, 60, 60));
    FloatRect sBounds = subtitle.getLocalBounds();
    subtitle.setOrigin(sBounds.width / 2.f, sBounds.height / 2.f);
    subtitle.setPosition(180, 260);

    //Message
    Text message("We hope you have a wonderful journey\nwith Chalo Pakistan!", font, 14);
    message.setFillColor(Color(80, 80, 80));
    FloatRect mBounds = message.getLocalBounds();
    message.setOrigin(mBounds.width / 2.f, mBounds.height / 2.f);
    message.setPosition(180, 320);

    //Close Button
    RectangleShape exitBtn(Vector2f(140, 45));
    exitBtn.setFillColor(Color(0, 90, 160));
    exitBtn.setOrigin(exitBtn.getSize().x / 2.f, exitBtn.getSize().y / 2.f);
    exitBtn.setPosition(180, 450);

    Text exitText("Close", font, 18);
    exitText.setFillColor(Color::White);
    FloatRect eBounds = exitText.getLocalBounds();
    exitText.setOrigin(eBounds.width / 2.f, eBounds.height / 2.f);
    exitText.setPosition(180, 443);

    //LOOP
    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                return;

            if (event.type == Event::MouseButtonPressed)
            {
                Vector2f mouse(event.mouseButton.x, event.mouseButton.y);

                if (exitBtn.getGlobalBounds().contains(mouse))
                {
                    std::cout << "Thank you screen closed.\n";
                    window.close();
                    return;
                }
            }
        }

        window.clear(bgColor);
        window.draw(title);
        window.draw(subtitle);
        window.draw(message);
        window.draw(exitBtn);
        window.draw(exitText);
        window.display();
    }
}

//TRANSPORT SCREEN (TRAVELLER)

void runTransportScreen(RenderWindow &window) {

    //Load Font
    Font font;
    if (!font.loadFromFile("MomoTrustDisplay-Regular.ttf")) {
        std::cerr << "Failed to load font\n";
        return;
    }

    //ENUM Steps 
    enum class Step {
        ChooseTransport,
        ChooseBusSchedule,
        ChooseTrainSchedule,
        ChooseTrainClass,
        ChoosePackage,
        Summary
    };

    Step step = Step::ChooseTransport;

    //State Variables
    int transportChoice = 0;
    int scheduleChoice  = 1;
    string seatClass    = "Economy";
    int packageChoice   = 1;
    int groupSize       = 3;

    //Header Text
    Text header("", font, 22);
    header.setFillColor(Color::White);
    header.setPosition(20.f, 40.f);

    //Load Images
    sf::Texture busTex, trainTex;
    busTex.loadFromFile("bus.png");
    trainTex.loadFromFile("train.png");

    sf::Sprite busImg(busTex);
    sf::Sprite trainImg(trainTex);

    float targetWidth = window.getSize().x * 0.55f;
    float busScale   = targetWidth / busTex.getSize().x;
    float trainScale = targetWidth / trainTex.getSize().x;

    busImg.setScale(busScale, busScale);
    trainImg.setScale(trainScale, trainScale);

    float cx = window.getSize().x / 2.f;

    busImg.setPosition(cx - busImg.getGlobalBounds().width / 2.f, 140.f);
    trainImg.setPosition(
        cx - trainImg.getGlobalBounds().width / 2.f,
        busImg.getPosition().y + busImg.getGlobalBounds().height + 50.f
    );

    //Buttons for Next Steps
    Button route1(font, "Route 1", {40.f, 120.f}, {280.f, 45.f});
    Button route2(font, "Route 2", {40.f, 180.f}, {280.f, 45.f});
    Button route3(font, "Route 3", {40.f, 240.f}, {280.f, 45.f});

    Button econBtn(font, "Economy",  {40.f, 120.f}, {280.f, 45.f});
    Button busiBtn(font, "Business", {40.f, 180.f}, {280.f, 45.f});

    Button soloBtn(font, "Solo", {40.f, 120.f}, {280.f, 40.f});
    Button duoBtn(font,  "Duo",  {40.f, 175.f}, {280.f, 40.f});
    Button grpBtn(font,  "Group",{40.f, 230.f}, {280.f, 40.f});

    Button minusBtn(font, "-",   {70.f, 280.f}, {60.f, 40.f});
    Button plusBtn(font,  "+",   {230.f, 280.f}, {60.f, 40.f});
    Button confirmPkg(font, "Continue", {110.f, 340.f}, {140.f, 45.f});

    Button backBtn(font, "Back", {10.f, 580.f}, {100.f, 40.f});
    Button doneBtn(font, "Close", {250.f, 580.f}, {100.f, 40.f});

    //CONFIRM BUTTON FOR SUMMARY SCREEN
    Button summaryConfirmBtn(font, "Confirm Booking", {80.f, 520.f}, {240.f, 45.f});

    Text groupText("", font, 18);
    groupText.setFillColor(Color::White);
    groupText.setPosition(150.f, 290.f);

    Text summaryText("", font, 18);
    summaryText.setFillColor(Color::White);
    summaryText.setPosition(20.f, 120.f);

    // Store objects until summary
    Transport* chosenTransport = nullptr;
    Packages* chosenPackage = nullptr;

    //MAIN LOOP 
    while (window.isOpen()) {
        Event e;
        while (window.pollEvent(e)) {

            if (e.type == Event::Closed) {
                window.close();
                return;
            }

            if (e.type == Event::MouseButtonPressed &&
                e.mouseButton.button == Mouse::Left)
            {
                Vector2f mp((float)e.mouseButton.x, (float)e.mouseButton.y);

                //BACK LOGIC
                if (backBtn.contains(mp)) {
                    if (step == Step::ChooseTransport) return;
                    if (step == Step::ChooseBusSchedule ||
                        step == Step::ChooseTrainSchedule)
                        step = Step::ChooseTransport;
                    else if (step == Step::ChooseTrainClass)
                        step = Step::ChooseTrainSchedule;
                    else if (step == Step::ChoosePackage)
                        step = (transportChoice == 1 ? Step::ChooseBusSchedule : Step::ChooseTrainClass);
                    else if (step == Step::Summary)
                        step = Step::ChoosePackage;
                }

                //STEP 1: Choose Transport (Images)
                if (step == Step::ChooseTransport) {
                    if (busImg.getGlobalBounds().contains(mp)) {
                        transportChoice = 1;
                        step = Step::ChooseBusSchedule;
                    }
                    else if (trainImg.getGlobalBounds().contains(mp)) {
                        transportChoice = 2;
                        step = Step::ChooseTrainSchedule;
                    }
                }

                //BUS SCHEDULE
                else if (step == Step::ChooseBusSchedule) {
                    if (route1.contains(mp)) {
                        if (runRoutePreviewScreen(window, 1, "Bus")) {
                            scheduleChoice = 1;
                            step = Step::ChoosePackage;
                        }
                    }
                    else if (route2.contains(mp)) {
                        if (runRoutePreviewScreen(window, 2, "Bus")) {
                            scheduleChoice = 2;
                            step = Step::ChoosePackage;
                        }
                    }
                    else if (route3.contains(mp)) {
                        if (runRoutePreviewScreen(window, 3, "Bus")) {
                            scheduleChoice = 3;
                            step = Step::ChoosePackage;
                        }
                    }
                }


                //TRAIN SCHEDULE 
                else if (step == Step::ChooseTrainSchedule) {
                    if (route1.contains(mp)) {
                        if (runRoutePreviewScreen(window, 1, "Train")) {
                            scheduleChoice = 1;
                            step = Step::ChooseTrainClass;
                        }
                    }
                    else if (route2.contains(mp)) {
                        if (runRoutePreviewScreen(window, 2, "Train")) {
                            scheduleChoice = 2;
                            step = Step::ChooseTrainClass;
                        }
                    }
                    else if (route3.contains(mp)) {
                        if (runRoutePreviewScreen(window, 3, "Train")) {
                            scheduleChoice = 3;
                            step = Step::ChooseTrainClass;
                        }
                    }
                }


                // TRAIN CLASS 
                else if (step == Step::ChooseTrainClass) {
                    if (econBtn.contains(mp)) { seatClass = "Economy"; step = Step::ChoosePackage; }
                    else if (busiBtn.contains(mp)) { seatClass = "Business"; step = Step::ChoosePackage; }
                }

                //PACKAGE SELECTION 
                else if (step == Step::ChoosePackage) {

                    if (soloBtn.contains(mp)) packageChoice = 1;
                    else if (duoBtn.contains(mp)) packageChoice = 2;
                    else if (grpBtn.contains(mp)) packageChoice = 3;

                    if (packageChoice == 3) {
                        if (minusBtn.contains(mp) && groupSize > 3) groupSize--;
                        if (plusBtn.contains(mp)  && groupSize < 20) groupSize++;
                    }

                    //Package confirmed
                    if (confirmPkg.contains(mp)) {

                        // Create transport object
                        if (chosenTransport) delete chosenTransport;
                        if (transportChoice == 1)
                            chosenTransport = new Bus(scheduleChoice);
                        else {
                            Train* t = new Train(scheduleChoice);
                            t->setSeatClass(seatClass);
                            chosenTransport = t;
                        }

                        //Create package object
                        if (chosenPackage) delete chosenPackage;
                        if (packageChoice == 1) chosenPackage = new Solo();
                        else if (packageChoice == 2) chosenPackage = new Duo();
                        else {
                            Group* g = new Group();
                            g->setTickets(groupSize);
                            chosenPackage = g;
                        }

                        //Build summary string
                        std::ostringstream oss;
                        oss << "Booking Confirmed!\n\n";
                        oss << "Transport: " << (transportChoice == 1 ? "Bus" : "Train") << "\n";
                        oss << "Route: " << scheduleChoice << "\n";
                        if (transportChoice == 2) oss << "Seat Class: " << seatClass << "\n";
                        if (packageChoice == 1) oss << "Package: Solo\n";
                        else if (packageChoice == 2) oss << "Package: Duo\n";
                        else oss << "Package: Group (" << groupSize << ")\n";

                        // Total cost (Preview only)
                        gAppState.traveller.setTransport(chosenTransport, scheduleChoice);
                        gAppState.traveller.setPackage(chosenPackage);
                        gAppState.traveller.calculateTotal();

                        oss << "Total Cost: Rs. " << gAppState.traveller.getTotalCost();

                        summaryText.setString(oss.str());
                        step = Step::Summary;
                    }
                }

                // SUMMARY SCREEN (Saving happens HERE)
                else if (step == Step::Summary) {

                //USER MAKES FINAL CONFIRMATION 
                if (summaryConfirmBtn.contains(mp)) {

                    // SAVE TO CSV
                    gAppState.traveller.storeRecord();
                    std::cout << "✔ Booking saved to admin_records.csv\n";

                    // GO TO THANK YOU SCREEN
                    runThankYou(window);

                    return; // AFTER thank you, exit booking flow
                }

                // CLOSE (ONLY exits summary, no save)
                if (doneBtn.contains(mp)) {
                    return;
                }
            }
            }
        }

        //DRAW
        window.clear(sf::Color(135, 206, 250));
        backBtn.draw(window);

        if (step == Step::ChooseTransport) {
            header.setString("Choose Transport");
            window.draw(header);
            window.draw(busImg);
            window.draw(trainImg);
        }
        else if (step == Step::ChooseBusSchedule) {
            header.setString("Bus: Choose Route");
            window.draw(header);
            route1.draw(window);
            route2.draw(window);
            route3.draw(window);
        }
        else if (step == Step::ChooseTrainSchedule) {
            header.setString("Train: Choose Route");
            window.draw(header);
            route1.draw(window);
            route2.draw(window);
            route3.draw(window);
        }
        else if (step == Step::ChooseTrainClass) {
            header.setString("Choose Seat Class");
            window.draw(header);
            econBtn.draw(window);
            busiBtn.draw(window);
        }
        else if (step == Step::ChoosePackage) {
            header.setString("Choose Package");
            window.draw(header);

            soloBtn.draw(window);
            duoBtn.draw(window);
            grpBtn.draw(window);

            if (packageChoice == 3) {
                minusBtn.draw(window);
                plusBtn.draw(window);
                groupText.setString(std::to_string(groupSize));
                window.draw(groupText);
            }

            confirmPkg.draw(window);
        }
        else if (step == Step::Summary) {
            header.setString("Booking Summary");
            window.draw(header);
            window.draw(summaryText);

            summaryConfirmBtn.draw(window);   // NEW BUTTON
            doneBtn.draw(window);
        }

        window.display();
    }
}

//ADMIN SCREEN

void runAdminScreen(RenderWindow &window) 
{
    Font font;
    if (!font.loadFromFile("MomoTrustDisplay-Regular.ttf")) {
        cerr << "Failed to load font\n";
        return;
    }

    Color bg(135, 206, 250);

    Text header("Admin Panel", font, 28);
    header.setFillColor(Color::White);
    header.setPosition(20, 40);

    Button busBtn   (font, "View Bus Bookings",   {40, 140}, {280, 45});
    Button trainBtn (font, "View Train Bookings", {40, 200}, {280, 45});
    Button totalBus (font, "Total Bus Bookings",  {40, 260}, {280, 45});
    Button totalTrain(font,"Total Train Bookings",{40, 320}, {280, 45});
    Button backBtn  (font, "Back",                {110, 400},{140, 45});

    while (window.isOpen()) 
    {
        Event e;
        while (window.pollEvent(e)) 
        {
            if (e.type == Event::Closed) 
            {
                window.close();
                return;
            }

            if (e.type == Event::MouseButtonPressed &&
                e.mouseButton.button == Mouse::Left)
            {
                Vector2f mp(e.mouseButton.x, e.mouseButton.y);

                if (busBtn.contains(mp)) {
                    runAdminBookingsScreen(window, "BUS_LIST");
                }
                else if (trainBtn.contains(mp)) {
                    runAdminBookingsScreen(window, "TRAIN_LIST");
                }
                else if (totalBus.contains(mp)) {
                    runAdminBookingsScreen(window, "TOTAL_BUS");
                }
                else if (totalTrain.contains(mp)) {
                    runAdminBookingsScreen(window, "TOTAL_TRAIN");
                }
                else if (backBtn.contains(mp)) {
                    return;
                }
            }
        }

        window.clear(bg);
        window.draw(header);
        busBtn.draw(window);
        trainBtn.draw(window);
        totalBus.draw(window);
        totalTrain.draw(window);
        backBtn.draw(window);
        window.display();
    }
}
