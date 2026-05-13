#include "chaloPakistan.hpp" 
#include <iostream> 
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>
using namespace std; 

string trim(string s) { //removes \r if found to match values
    if (!s.empty() && s.back()=='\r') {
        s.pop_back();}
    return s;}

// TRANSPORT
Transport::Transport() {   
    type = ""; 
    baseFare = 0; } 

Transport::Transport(const string& t, double b) { 
    type = t; 
    baseFare = b; } 

Transport::~Transport() {} 

string Transport::getType(){
    return type;}

// BUS 
Bus::Bus() { //default route 
    type = "Bus"; 
    busCompany = "FADWheels"; 
    baseFare = 500; 

    startCity[0] = "Karachi"; 
    endCity[0] = "Islamabad"; 
    date[0] = "09 Nov 2025"; 
    time[0] = "8:00 AM"; 

    startCity[1] = "Islamabad"; 
    endCity[1] = "Swat"; 
    date[1] = "09 Nov 2025"; 
    time[1] = "2:30 PM"; 

    startCity[2] = "Swat"; 
    endCity[2] = "Kashmir"; 
    date[2] = "10 Nov 2025"; 
    time[2] = "8:00 AM"; 

    startCity[3] = "Kashmir"; 
    endCity[3] = "Karachi"; 
    date[3] = "11 Nov 2025"; 
    time[3] = "7:00 PM"; 

    totalRoutes = 4; } 

Bus::Bus(int choice) {
    scheduleChoice = choice;
    type = "Bus";
    busCompany = "FADWheels";
    baseFare = 500;
    totalRoutes = 4;

    if (choice == 1) {
        // ——— ROUTE A ——
        startCity[0] = "Karachi"; endCity[0] = "Islamabad";
        startCity[1] = "Islamabad"; endCity[1] = "Swat"; 
        startCity[2] = "Swat"; endCity[2] = "Kashmir";
        startCity[3] = "Kashmir"; endCity[3] = "Karachi";

        date[0] = "09 Nov 2025"; time[0] = "8:00 AM";
        date[1] = "09 Nov 2025"; time[1] = "2:30 PM";
        date[2] = "10 Nov 2025"; time[2] = "8:00 AM";
        date[3] = "11 Nov 2025"; time[3] = "7:00 PM";
    }
    else if (choice == 2) {
        // ——— ROUTE B ——
        startCity[0] = "Karachi"; endCity[0] = "Quetta";
        startCity[1] = "Quetta"; endCity[1] = "Zhob";
        startCity[2] = "Zhob"; endCity[2] = "Multan";
        startCity[3] = "Multan"; endCity[3] = "Karachi";

        date[0] = "05 Nov 2025"; time[0] = "9:00 AM";
        date[1] = "06 Nov 2025"; time[1] = "11:00 AM";
        date[2] = "07 Nov 2025"; time[2] = "2:00 PM";
        date[3] = "08 Nov 2025"; time[3] = "5:00 PM";
    }
    else {
        // ——— ROUTE C ——
        startCity[0] = "Karachi"; endCity[0] = "Gwadar";
        startCity[1] = "Gwadar"; endCity[1] = "Ormara";
        startCity[2] = "Ormara"; endCity[2] = "Pasni";
        startCity[3] = "Pasni"; endCity[3] = "Karachi";

        date[0] = "12 Nov 2025"; time[0] = "7:00 AM";
        date[1] = "12 Nov 2025"; time[1] = "12:00 PM";
        date[2] = "13 Nov 2025"; time[2] = "4:00 PM";
        date[3] = "14 Nov 2025"; time[3] = "9:00 AM";
    }}

int Bus::getScheduleChoice() const {return scheduleChoice;}
void Bus::displaySchedule() const { 
    cout << "\n Below is the Bus Schedule" << endl; 
    for (int i = 0; i < totalRoutes; i++) { 
        cout << startCity[i] << " -> " << endCity[i] << endl; 
        cout << "Date: " << date[i] << endl; 
        cout << "Departure: " << time[i] << endl; 
        cout << "----------------------------------" << endl;}} 

double Bus::calculateFare() const { 
    return baseFare; } 

void Bus::displayInfo() const { 
    cout << "\n--- BUS BOOKING DETAILS ---\n"; 
    cout << "Transport Type: " << type << endl; 
    cout << "Schedule Chosen: " << scheduleChoice << endl;
    cout << "Company: " << busCompany << endl; 
    displaySchedule(); 
}

// ================= TRAIN =================
Train::Train() { 
    type = "Train"; 
    trainName = "NationalEngine"; 
    baseFare = 800; 

    startCity[0] = "Karachi"; 
    endCity[0] = "Hyderabad"; 
    date[0] = "09 Nov 2025"; 
    time[0] = "6:00 AM"; 

    startCity[1] = "Hyderabad"; 
    endCity[1] = "Lahore"; 
    date[1] = "09 Nov 2025"; 
    time[1] = "1:00 PM"; 

    startCity[2] = "Lahore"; 
    endCity[2] = "Rawalpindi"; 
    date[2] = "10 Nov 2025";
    time[2] = "9:00 AM"; 

    startCity[3] = "Rawalpindi"; 
    endCity[3] = "Gilgit"; 
    date[3] = "11 Nov 2025"; 
    time[3] = "7:30 PM"; 

    totalRoutes = 4; 
} 

Train::Train(int choice) {
    scheduleChoice = choice;
    type = "Train";
    baseFare = 800;
    totalRoutes = 4;

    if (choice == 1) {
        trainName = "NationalEngine";
        startCity[0] = "Karachi"; endCity[0] = "Hyderabad";
        startCity[1] = "Hyderabad"; endCity[1] = "Lahore";
        startCity[2] = "Lahore"; endCity[2] = "Rawalpindi";
        startCity[3] = "Rawalpindi"; endCity[3] = "Gilgit";

        date[0] = "09 Nov 2025"; time[0] = "6:00 AM";
        date[1] = "09 Nov 2025"; time[1] = "1:00 PM";
        date[2] = "10 Nov 2025"; time[2] = "9:00 AM";
        date[3] = "11 Nov 2025"; time[3] = "7:30 PM";
    }
    else if (choice == 2) {
        trainName = "MountainExpress";
        startCity[0] = "Karachi"; endCity[0] = "Naran";
        startCity[1] = "Naran"; endCity[1] = "Kaghan";
        startCity[2] = "Kaghan"; endCity[2] = "Chilas";
        startCity[3] = "Chilas"; endCity[3] = "Karachi";

        date[0] = "05 Nov 2025"; time[0] = "8:00 AM";
        date[1] = "05 Nov 2025"; time[1] = "2:00 PM";
        date[2] = "06 Nov 2025"; time[2] = "11:00 AM";
        date[3] = "07 Nov 2025"; time[3] = "9:00 AM";
    }
    else {
        trainName = "CoastalRail";
        startCity[0] = "Karachi"; endCity[0] = "Badin";
        startCity[1] = "Badin"; endCity[1] = "Sujawal";
        startCity[2] = "Sujawal"; endCity[2] = "Gwadar";
        startCity[3] = "Gwadar"; endCity[3] = "Karachi";

        date[0] = "10 Nov 2025"; time[0] = "7:00 AM";
        date[1] = "10 Nov 2025"; time[1] = "1:00 PM";
        date[2] = "11 Nov 2025"; time[2] = "5:00 PM";
        date[3] = "12 Nov 2025"; time[3] = "10:00 AM";
    }
}


void Train::displaySchedule() const { 
    cout << "\nBelow is the Train Schedule" << endl; 
    for (int i = 0; i < totalRoutes; i++) { 
        cout << startCity[i] << " -> " << endCity[i] << endl; 
        cout << "Date: " << date[i] << endl; 
        cout << "Departure: " << time[i] << endl; 
        cout << "----------------------------------" << endl; 
    } 
} 

double Train::calculateFare() const { 
    double fare=baseFare;
    if (seatClass=="Business"){
        fare = fare * 1.4;}
    return fare; } 

void Train::setSeatClass(const string& sc){
    seatClass = sc;
}  

string Train::getSeatClass(){
    return seatClass;
}  

void Train::displayInfo() const { 
    cout << "\n--- TRAIN BOOKING DETAILS ---\n"; 
    cout << "Transport Type: " << type << endl; 
    cout << "Seat class: " << seatClass << endl;
    cout << "Schedule Chosen: " << scheduleChoice << endl;
    cout << "Train: " << trainName << endl; 
    displaySchedule(); 
} 

int Train::getScheduleChoice() const {
    return scheduleChoice;
}

// PACKAGES BASE 
Packages::Packages() { 
    packageType = ""; 
    packageCost = 1500; 
    mealCost = 300;
    numOfTickets = 0; 
} 

Packages::~Packages() {} 

void Packages::setTickets(int t) { 
    numOfTickets = t; 
} 

int Packages::getNumOfTickets() const {
    return numOfTickets;
}

string Packages::getPackageType() const {
    return packageType;
}

// SOLO PACKAGE
Solo::Solo() { 
    numOfTickets = 1;
    packageType = "Solo"; 
} 

double Solo::calculatePackageCost() { 
    return (packageCost + mealCost); 
} 

void Solo::displayDetails() { 
    cout << "Package: " << packageType << endl; 
    cout << "Travelers: 1" << endl; 
} 

// ============== DUO PACKAGE ==============
Duo::Duo() { 
    numOfTickets = 2;
    packageType = "Duo"; 
    discountRate = 0.20;  
} 

double Duo::calculatePackageCost() { 
    double cost = (packageCost + mealCost) * 2 * (1.00 - 0.20);
    return cost; 
} 

void Duo::displayDetails() { 
    cout << "Package: " << packageType << endl; 
    cout << "Travelers: 2" << endl; 
} 

// GROUP PACKAGE 
Group::Group() { 
    packageType = "Group"; 
    discountRate = 0.40; 
} 

double Group::calculatePackageCost() { 
    return (packageCost + mealCost) * numOfTickets * (1 - discountRate); 
} 

void Group::displayDetails() { 
    cout << "Package: " << packageType << endl; 
    cout << "Travelers: " << numOfTickets << endl; 
} 


// TRAVELLER & USER 
Traveller::Traveller() { 
    transport = nullptr; 
    package = nullptr; 
    totalCost = 0;
    role = "TRAVELLER"; 
} 

void Traveller::chooseTransport() {
    int pick = 0;

    while (true) {
        cout << "1. Bus\n2. Train\nChoice: ";
        cin >> pick;

        if (!cin || (pick != 1 && pick != 2)) { //if some other value then dont accept
            cin.clear();
            string trash;
            getline(cin, trash);
            cout << "Invalid input. Please enter 1 or 2.\n";
            continue;
        }

        if (pick == 1) {
            cout << "\n=== Select Bus Schedule ===\n";
            cout << "1. Route A (Karachi -> Islamabad -> Swat -> Kashmir -> Karachi)\n";
            cout << "2. Route B (Karachi -> Quetta -> Zhob -> Multan -> Karachi)\n";
            cout << "3. Route C (Karachi -> Gwadar -> Ormara -> Pasni -> Karachi)\n";

            cout << "Choice: ";
            cin >> scheduleChoice;

            while (!cin || scheduleChoice < 1 || scheduleChoice > 3) {
                cin.clear();
                string trash; getline(cin, trash);
                cout << "Invalid. Choose 1-3: ";
                cin >> scheduleChoice;
            }

            transport = new Bus(scheduleChoice);
            break;}

        if (pick == 2) {
            cout << "\n=== Select Train Schedule ===\n";
            cout << "1. Route A (Karachi -> Hyderabad -> Lahore -> Rawalpindi -> Gilgit)\n";
            cout << "2. Route B (Karachi -> Naran -> Kaghan -> Chilas -> Karachi)\n";
            cout << "3. Route C (Karachi -> Badin -> Sujawal -> Gwadar -> Karachi)\n";

            cout << "Choice: ";
            cin >> scheduleChoice;

            while (!cin || scheduleChoice < 1 || scheduleChoice > 3) {
                cin.clear();
                string trash; getline(cin, trash);
                cout << "Invalid. Choose 1-3: ";
                cin >> scheduleChoice;
            }

            int seatChoice = 0;
            cout << "\n===== Choose Seat Class =====\n";
            cout << "1. Economy\n2. Business\nChoice: ";
            cin >> seatChoice;

            while (!cin || (seatChoice != 1 && seatChoice != 2)) {
                cin.clear();
                string trash; getline(cin, trash);
                cout << "Invalid input. Choose 1 or 2: ";
                cin >> seatChoice;
            }

            Train* train = new Train(scheduleChoice);
            train->setSeatClass(seatChoice == 2 ? "Business" : "Economy");
            transport = train;
            break;
        }
    }
}

void Traveller::choosePackage() { 
    int pick = 0;
    while (true) {
        cout << "1. Solo\n2. Duo\n3. Group\nChoice: ";
        cin >> pick;
        if (!cin) {
            cin.clear();
            string trash;
            getline(cin, trash);
            cout << "Invalid input. Try again.\n";
        }

        if (pick == 1) {
            package = new Solo();
            break;
        }

        if (pick == 2) {
            package = new Duo();
            break;
        }

        if (pick == 3) {
            int t = 0;
            while (true) {
                cout << "Tickets (enter number of travelers): ";
                cin >> t;
                if (!cin) {
                    cin.clear();
                    cout << "Invalid input. Try again\n";
                    continue;
                }
                if (t <= 2) {
                    cout << "Number of tickets must be at least 3.\n";
                    continue;
                }
                break;
            }
            package = new Group();
            package->setTickets(t);
            break;
        }

        cout << "Invalid option. Please enter 1, 2 or 3.\n";
    }
}

void Traveller::displayTransportSchedule() {
    if (transport) {
        transport->displayInfo();
    }
} 

void Traveller::calculateTotal() { 
    double transportcost = 0;
    double packagecost = 0;
    if (transport) 
        transportcost = transport->calculateFare();
    if (package) 
        packagecost = package->calculatePackageCost(); 
    int numofppl = package ? package->getNumOfTickets() : 0;
    totalCost = (transportcost * numofppl) + packagecost;
} 

void Traveller::displaySummary() { 
    cout << "===== Fare & Travel Details =====\n"; 
    if (transport) 
        transport->displayInfo(); 
    if (package) 
        package->displayDetails(); 
    cout << "Total Fare: Rs." << totalCost << endl; 
    cout << "\nIncludes discounts, meals, sleeper berths & luggage.\n"; 
}

// setters for sfml
void Traveller::setTransport(Transport* t, int sched) {
    transport = t;
    scheduleChoice = sched;
}

void Traveller::setPackage(Packages* p) {
    package = p;
}

double Traveller::getTotalCost() const {
    return totalCost;
}

void Traveller::storeRecord() {
    // Get next booking ID
    int BookingID = 1;
    ifstream infile("admin_records.csv");

    if (infile.is_open()) {
        string line;
        getline(infile, line); //Skip header
        while (getline(infile, line)) {
            BookingID++;
        }
        infile.close();
    }

    //get today's date
    time_t now = time(0);
    tm* ltm = localtime(&now);

    char dateBuf[40];
    sprintf(dateBuf, "%04d-%02d-%02d",
            1900 + ltm->tm_year,
            1 + ltm->tm_mon,
            ltm->tm_mday);

    string bookingDate = dateBuf;

    //prepare fields
    string transportType = transport ? transport->getType() : "";
    string busPackage = "";
    string trainPackage = "";
    string trainClass = "";

    if (transportType == "Bus") {
        busPackage = package ? package->getPackageType() : "";
    } 
    else if (transportType == "Train") { 
        trainPackage = package ? package->getPackageType() : "";
        //train has extra feature that bus does not so this checks whether 
        // the Transport* is really a train
        Train* t = dynamic_cast<Train*>(transport);
        if (t) trainClass = t->getSeatClass();
    }

    //write to CSV admin_records
    ofstream outfile("admin_records.csv", ios::app);

    if (!outfile.is_open()) {
        cout << "Error: Could not open file" << endl;
        return;
    }

    outfile << BookingID << ","
            << userid << ","
            << username << ","
            << transportType << "," 
            << busPackage << ","
            << trainPackage << ","
            << trainClass << ","
            << bookingDate << ","
            << totalCost << ","
            << scheduleChoice 
            << endl;

    outfile.close();
}

// USER BASE 
User::User(){
    username = "";
    userid = 0;
    role = "";
    password = "";
}

User::User(string n,int id,string r,string pass){
    username = n;
    userid = id;
    role = r;
    password = pass;
}

bool User::authenticate(string name,string pass){
    ifstream file("user.csv");
    if (!file.is_open()) {
        cout << "Error: Could not open user.csv" << endl;
        return false;
    }
    
    string line;
    getline(file, line); 
    
    while (getline(file, line)) {
        stringstream ss(line);
        string id, uname, pwd, r;
        
        getline(ss, id, ',');
        getline(ss, uname, ',');
        getline(ss, pwd, ',');
        getline(ss, r, ',');
        
        uname = trim(uname);
        pwd   = trim(pwd);
        r     = trim(r);

    if (uname == name && pwd == pass) {
        userid = stoi(trim(id));
        username = uname;
        password = pwd;
        role = r;
        return true;
}

    }
    
    file.close();
    return false;
}

//storing everything so can be used later
void User::setUsername(string n){username = n;}
string User::getUsername() const{return username;}
void User::setUserID(int id){userid = id;}
int User::getUserID() const{return userid;}
void User::setRole(string r){role = r;}
string User::getRole() const{return role;}
void User::setPassword(string p){password = p;}

//ADMIN 
Admin::Admin(){
    role = "ADMIN";
}

void Admin::BusBookings(){
    ifstream file("admin_records.csv");
    if (!file.is_open()) {
        cout<<"Error!File does not exist"<<endl;
        return;
    }    
    string line;
    getline(file, line); 
    cout<<"======Bus Bookings========"<<endl;
    while (getline(file, line)) {
        string bookingID,userID,username,TransportType,BusPackage,TrainPackage,Trainclass,BookingDate,Total,scheduleChoice;
        stringstream ss(line);
        getline(ss, bookingID, ',');
        getline(ss, userID, ',');
        getline(ss, username, ',');
        getline(ss, TransportType, ',');
        getline(ss, BusPackage, ',');
        getline(ss, TrainPackage, ',');
        getline(ss, Trainclass, ',');
        getline(ss, BookingDate, ',');
        getline(ss, Total,',');
        getline(ss, scheduleChoice);

        if (TransportType=="Bus"){
            cout<<"Booking ID: "<<bookingID
                <<" | Username: "<<username
                <<" | Transport type: "<<TransportType
                <<" | Bus Package: "<<BusPackage
                <<" | Booking Date: "<<BookingDate
                <<" | Total Cost: "<<Total
                <<" | Schedule Choice: "<< scheduleChoice 
                <<endl; 
        }
    }
    file.close();        
}

void Admin::TrainBookings(){
    ifstream file("admin_records.csv");
    if (!file.is_open()) {
        cout<<"Error!File does not exist"<<endl;
        return;
    }
    string line;
    getline(file, line);
    cout<<"======Train Bookings========"<<endl;
    while (getline(file, line)) {
        string bookingID,userID,username,TransportType,BusPackage,TrainPackage,Trainclass,BookingDate,Total,scheduleChoice;
        stringstream ss(line);
        getline(ss, bookingID, ',');
        getline(ss, userID, ',');
        getline(ss, username, ',');
        getline(ss, TransportType, ',');
        getline(ss, BusPackage, ',');
        getline(ss, TrainPackage, ',');
        getline(ss, Trainclass, ',');
        getline(ss, BookingDate, ',');
        getline(ss, Total,',');
        getline(ss, scheduleChoice);
        if (TransportType=="Train"){
            cout<<"Booking ID: "<<bookingID
                <<" | Username: "<<username
                <<" | Transport type: "<<TransportType
                <<" | Train Package: "<<TrainPackage
                <<" | Train Seat Class: "<<Trainclass
                <<" | Booking Date: "<<BookingDate
                <<" | Total Cost: "<<Total
                <<" | Schedule Choice: "<< scheduleChoice 
                <<endl;
        }
    }
    file.close();        
}

void Admin::TotalBusBookings(){
    ifstream file("admin_records.csv");
    int count=0;
    if (!file.is_open()) {
        cout<<"Error!File does not exist"<<endl;
        return;
    }    
    string line;
    getline(file, line); 
    while (getline(file, line)) {
        string bookingID,userID,username,TransportType,BusPackage,TrainPackage,Trainclass,BookingDate,Total,scheduleChoice;
        stringstream ss(line);
        getline(ss, bookingID, ',');
        getline(ss, userID, ',');
        getline(ss, username, ',');
        getline(ss, TransportType, ',');
        getline(ss, BusPackage, ',');
        getline(ss, TrainPackage, ',');
        getline(ss, Trainclass, ',');
        getline(ss, BookingDate, ',');
        getline(ss, Total,',');
        getline(ss, scheduleChoice);
        if (TransportType=="Bus"){
            count+=1;
        }
    }    
    file.close(); 
    cout<<"Total Bus bookings: "<<count<<endl;   
}

void Admin::TotalTrainBookings(){
    ifstream file("admin_records.csv");
    int count=0;
    if (!file.is_open()) {
        cout<<"Error!File does not exist"<<endl;
        return;
    }
    string line;
    getline(file, line); 
    while (getline(file, line)) {
        string bookingID,userID,username,TransportType,BusPackage,TrainPackage,Trainclass,BookingDate,Total,scheduleChoice;
        stringstream ss(line);
        getline(ss, bookingID, ',');
        getline(ss, userID, ',');
        getline(ss, username, ',');
        getline(ss, TransportType, ',');
        getline(ss, BusPackage, ',');
        getline(ss, TrainPackage, ',');
        getline(ss, Trainclass, ',');
        getline(ss, BookingDate, ',');
        getline(ss, Total,',');
        getline(ss, scheduleChoice);
        if (TransportType=="Train"){
            count+=1;
        }
    }    
    file.close(); 
    cout<<"Total Train bookings: "<<count<<endl;   
}
