-- ADMIN TABLE
CREATE TABLE Admin (
    Email       VARCHAR(255) PRIMARY KEY,
    FirstName   VARCHAR(255),
    LastName    VARCHAR(255),
    Password    VARCHAR(255)
);

--               CUSTOMER TABLE
CREATE TABLE Customer (
    Email       VARCHAR(255) PRIMARY KEY,
    First_Name  VARCHAR(255),
    Last_Name   VARCHAR(255),
    Password    VARCHAR(255)
);

--               EVENT TABLE
CREATE TABLE Event (
    EventID     INT PRIMARY KEY,
    EventName   VARCHAR(255),
    Location    VARCHAR(255),
    EventDate   DATE,
    StartTime   TIME,
    EndTime     TIME,
    Description VARCHAR(MAX),
    Price       FLOAT,
    Capacity    INT,
    Sold        INT,
    Email       VARCHAR(255),
    FOREIGN KEY (Email) REFERENCES Admin(Email)
);

--               ORDER TABLE

CREATE TABLE [Order] (
    OrderID        INT IDENTITY(20001,1) PRIMARY KEY,
    PurchaseDate   DATE,
    Total_Tickets  INT,
    Amount         INT,
    EventID        INT,
    Email          VARCHAR(255),
    FOREIGN KEY (EventID) REFERENCES Event(EventID),
    FOREIGN KEY (Email) REFERENCES Customer(Email)
);

--               UPCOMING EVENTS TABLE

CREATE TABLE Upcoming_Events (
    Email   VARCHAR(255),
    EventID INT,
    PRIMARY KEY (Email, EventID),
    FOREIGN KEY (Email) REFERENCES Customer(Email),
    FOREIGN KEY (EventID) REFERENCES Event(EventID)
);

--               Modifying TABLE
CREATE TABLE Modifying (
    Email   VARCHAR(255),
    EventID INT,
    PRIMARY KEY (Email, EventID),
    FOREIGN KEY (Email) REFERENCES Customer(Email),
    FOREIGN KEY (EventID) REFERENCES Event(EventID)
);


--               INSERT DUMMY DATA



-- ADMIN DATA
INSERT INTO Admin (Email, FirstName, LastName, Password)
VALUES 
('benstokes_12@gmail.com', 'Ben', 'Stokes', 'admin123'),
('admin_showtime@gmail.com', 'Areesha', 'Ashfaq', 'areesha123');

-- CUSTOMER DATA
INSERT INTO Customer (Email, First_Name, Last_Name, Password)
VALUES
('sarahkhan98@gmail.com', 'Sarah', 'Khan', 'sarahpass'),
('bobthebuilder@gmail.com', 'Bob', 'Builder', 'bobpass'),
('john.doe@gmail.com', 'John', 'Doe', 'john123'),
('divya10267@gmail.com', 'Divya', 'Lakhani', 'divya999');

-- EVENT DATA
INSERT INTO Event (EventID, EventName, Location, EventDate, StartTime, EndTime, Description, Price, Capacity, Sold, Email)
VALUES
(1001, 'Cricket Match', 'National Stadium', '2025-09-17', '19:00', '23:30',
 'An exciting cricket match under the lights at National Stadium.', 1500, 1000, 120,
 'benstokes_12@gmail.com'),

(1002, 'Concert', 'Arts Auditorium', '2025-11-05', '20:00', '22:00',
 'A live music concert featuring top artists.', 2500, 500, 200,
 'benstokes_12@gmail.com'),

(1003, 'Football Final', 'KMC Stadium', '2025-12-01', '18:00', '21:00',
 'Championship football final — high-energy night with live crowd.', 1800, 1500, 850,
 'admin_showtime@gmail.com'),

(1004, 'Game Night', 'Expo Center Hall B', '2025-10-21', '17:00', '20:00',
 'A fun game night for families and friends.', 800, 300, 75,
 'admin_showtime@gmail.com');

-- ORDER DATA (auto-generated)

INSERT INTO [Order] (PurchaseDate, Total_Tickets, Amount, EventID, Email)
VALUES
('2025-11-01', 2, 5000, 1002, 'sarahkhan98@gmail.com'),
('2025-09-18', 3, 4500, 1001, 'john.doe@gmail.com'),
('2025-10-22', 1, 800, 1004, 'bobthebuilder@gmail.com'),
('2025-12-02', 4, 7200, 1003, 'sarahkhan98@gmail.com');


-- UPCOMING EVENTS DATA

INSERT INTO Upcoming_Events (Email, EventID)
VALUES
('sarahkhan98@gmail.com', 1002),
('john.doe@gmail.com', 1001),
('divya10267@gmail.com', 1003);

-------------------------
-- Modifying DATA
-------------------------
INSERT INTO Modifying (Email, EventID)
VALUES
('sarahkhan98@gmail.com', 1001),
('sarahkhan98@gmail.com', 1002),
('john.doe@gmail.com', 1004),
('divya10267@gmail.com', 1002);