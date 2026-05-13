#ifndef CHALOPAKISTAN_H 
#define CHALOPAKISTAN_H 
#include <string> 
using namespace std; 
class Transport { 
    protected: 
    string type; 
    double baseFare; 
    public: 
    Transport(); 
    Transport(const string& type, double baseFare); 
    virtual ~Transport(); // MUST be const because derived classes use const 
    virtual void displayInfo() const = 0; 
    virtual double calculateFare() const = 0; 
    string getType();}; 
// BUS 
class Bus: 
public Transport { 
    private: string busCompany; 
    string startCity[4]; 
    string endCity[4]; 
    string date[4]; 
    string time[4]; 
    int totalRoutes; 
    public: 
    Bus(); 
    void displayInfo() const override; 
    void displaySchedule() const; 
    double calculateFare() const override; }; 
// TRAIN  
class Train : public Transport { 
    private: 
    string trainName; 
    string startCity[4]; 
    string endCity[4]; 
    string date[4]; 
    string time[4]; 
    int totalRoutes; 
    string seatClass;
    public: 
    Train(); 
    void displayInfo() const override; 
    void displaySchedule() const; 
    double calculateFare() const override;  
    void setSeatClass(const string& sc);
    string getSeatClass();};
// PACKAGES (ABSTRACT) 
class Packages { 
    protected: 
    string packageType; 
    double packageCost; 
    double mealCost;
    int numOfTickets; 
    public: Packages(); 
    virtual ~Packages(); 
    virtual double calculatePackageCost() = 0; 
    virtual void displayDetails() = 0; 
    void setTickets(int tickets);
    int getNumOfTickets() const;
    string getPackageType()const;  }; 
// SOLO PACKAGE  
class Solo : public Packages { 
    public: 
    Solo(); 
    double calculatePackageCost() override; 
    void displayDetails() override; }; 
// DUO PACKAGE  
class Duo : public Packages { 
    double discountRate; 
        public: 
    Duo(); 
    double calculatePackageCost() override; 
    void displayDetails() override; 
    void applyDiscount(); }; 
// GROUP PACKAGE 
class Group : public Packages { 
    double discountRate;  
    public: 
    Group(); 
    double calculatePackageCost() override; 
    void displayDetails() override; 
    void calculateDiscount(); }; 
    class User{
    protected:
    string username;
    int userid;
    string role;
    string password;
    public:
    User();
    User(string n,int id,string r,string pass);
    bool authenticate(string name,string pass);
    virtual void setUsername(string n);
    virtual string getUsername() const;
    virtual void setUserID(int id);
    virtual int getUserID() const;
    void setRole(string r);
    string getRole() const;
    void setPassword(string p);


}; 
// TRAVELLER 
class Traveller:public User{ 
    public: 
    Traveller(); 
    void chooseTransport(); 
    void choosePackage(); 
    void calculateTotal(); 
    void displaySummary();
    void displayTransportSchedule();
    void storeRecord();
    private: 
    Transport* transport; 
    Packages* package; 
    double totalCost; 
    
}; 

class Admin:public User {
public:
    Admin();
    void BusBookings();
    void TrainBookings();
    void TotalBusBookings();
    void TotalTrainBookings();
};  
  
#endif