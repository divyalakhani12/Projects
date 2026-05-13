from PyQt6 import QtWidgets, uic
from PyQt6.QtCore import QDate
from PyQt6.QtWidgets import QMainWindow, QApplication, QMessageBox
import sys
import pyodbc
from datetime import datetime

server = 'localhost\\sqlserver1'
database = 'ShowTime'  # Name of your Northwind database
use_windows_authentication = True  # Set to True to use Windows Authentication
username = 'your_username'  # Specify a username if not using Windows Authentication
password = 'your_password'  # Specify a password if not using Windows Authentication

if use_windows_authentication:
    connection_string = f"""
    DRIVER={{ODBC Driver 17 for SQL Server}};
    SERVER={server};
    DATABASE={database};
    Trusted_Connection=yes;
    """
else:
    username = 'sa'
    password = 'Lynch7432@Barns'
    connection_string = f"""
    DRIVER={{ODBC Driver 17 for SQL Server}};
    SERVER={server};
    DATABASE={database};
    UID={username};
    PWD={password};
    """

class LoginUI(QMainWindow):
    def __init__(self):
        super(LoginUI, self).__init__()

        uic.loadUi('1.ui', self)

        self.pushButton.clicked.connect(self.login_user)
        self.pushButton_2.clicked.connect(self.open_signup)

    def login_user(self):
        email = self.lineEdit.text()
        password = self.lineEdit_2.text()

        if self.radioButton.isChecked():
            table = "Admin"
        else:
            table = "Customer"

        connection = pyodbc.connect(connection_string)
        cursor = connection.cursor()

        query = f"""
        SELECT COUNT(*) 
        FROM {table}
        WHERE Email=? AND Password=?
        """

        cursor.execute(query, (email, password))
        result = cursor.fetchone()[0]

        connection.close()

        if result == 1:
            QMessageBox.information(self, "Success", f"{table} Login Successful")

            if table == "Admin":
                self.admin_screen = AdminHomeUI(email)
                self.admin_screen.show()
            else:
                self.customer_screen = CustomerHomeUI(email)
                self.customer_screen.show()

            self.close()
        else: QMessageBox.warning(self,"Error","Invalid Email/Password")

    def open_signup(self):
        self.signup_window = SignupUI()
        self.signup_window.show()
        self.close()


class SignupUI(QMainWindow):
    def __init__(self):
        super(SignupUI, self).__init__()
        uic.loadUi('2.ui', self)
    
        self.pushButton.clicked.connect(self.register_user)
        self.pushButton_2.clicked.connect(self.back)

    def back(self):
        self.login_window= LoginUI()
        self.login_window.show()
        self.close()

    def register_user(self):
        first = self.lineEdit.text()
        last = self.lineEdit_2.text()
        email = self.lineEdit_3.text()
        password = self.lineEdit_4.text()

        role = "Admin" if self.radioButton.isChecked() else "Customer"

        connection = pyodbc.connect(connection_string)
        cursor = connection.cursor()

        check_query = """
        SELECT COUNT(*) FROM Admin WHERE Email=?
        UNION
        SELECT COUNT(*) FROM Customer WHERE Email=?
        """
        cursor.execute(check_query, (email, email))
        results = cursor.fetchall()
        exists = any(r[0] > 0 for r in results)

        if exists:
            QMessageBox.warning(self, "Error", "Email already exists")
            connection.close()
            return

        if role == "Admin":
            insert_query = """
            INSERT INTO Admin (Email, FirstName, LastName, Password)
            VALUES (?, ?, ?, ?)
            """
        else:
            insert_query = """
            INSERT INTO Customer (Email, First_Name, Last_Name, Password)
            VALUES (?, ?, ?, ?)
            """

        cursor.execute(insert_query, (email, first, last, password))
        connection.commit()
        connection.close()

        QMessageBox.information(self, "Success", f"{role} Registered Successfully!")

        self.login_window = LoginUI()     
        self.login_window.show()          
        self.close()                     

class AdminHomeUI(QMainWindow):
    def __init__(self, email):
        super(AdminHomeUI, self).__init__()
        uic.loadUi("3.ui", self)

        self.email = email
        self.pushButton.clicked.connect(self.open_add_event)      
        self.pushButton_2.clicked.connect(self.open_modify_event) 
        self.pushButton_3.clicked.connect(self.back)

    def back(self):
        self.login_window= LoginUI()
        self.login_window.show()
        self.close()

    def open_add_event(self):
        self.window_add = AddEventUI(self.email)
        self.window_add.show()

    def open_modify_event(self):
        self.window_modify = EditEventUI()
        self.window_modify.show()

class CustomerHomeUI(QMainWindow):
    def __init__(self, email):
        super(CustomerHomeUI, self).__init__()
        uic.loadUi("8.ui", self)

        self.email = email
    
        self.pushButton.clicked.connect(self.open_view_orders)      
        self.pushButton_2.clicked.connect(self.open_buy_tickets) 
        self.pushButton_3.clicked.connect(self.back)

    def back(self):
        self.login_window= LoginUI()
        self.login_window.show()
        self.close()
        
    def open_view_orders(self):
        self.orders_window = CustomerOrdersUI(self.email)
        self.orders_window.show()
    def open_buy_tickets(self):
        self.buy_window = BuyTicketsUI(self.email)
        self.buy_window.show()

class BuyTicketsUI(QMainWindow):
    def __init__(self, email):
        super(BuyTicketsUI, self).__init__()
        uic.loadUi("6.ui", self)

        self.email = email
        self.load_events()
        self.load_selected_event()
        self.comboBox.currentIndexChanged.connect(self.load_selected_event)
        self.pushButton.clicked.connect(self.purchase_tickets)

    def load_events(self):
        conn = pyodbc.connect(connection_string)
        cursor = conn.cursor()

        cursor.execute("""
            SELECT EventID, EventName, Location, EventDate, StartTime, EndTime, Description, Price
            FROM Event
            ORDER BY EventDate
        """)

        self.events = cursor.fetchall()
        conn.close()

        self.comboBox.clear()
        for e in self.events:
            display = f"{e.EventID} - {e.EventName}"
            self.comboBox.addItem(display, e.EventID)

    def load_selected_event(self):
        index = self.comboBox.currentIndex()
        if index < 0:
            return
        e = self.events[index]
        self.lineEdit.setText(e.EventName)
        self.lineEdit_2.setText(e.Location)
        self.lineEdit_3.setText(e.EventDate.strftime("%m-%d-%y"))
        self.lineEdit_4.setText(e.StartTime.strftime("%H:%M"))
        self.lineEdit_5.setText(e.EndTime.strftime("%H:%M"))
        self.textEdit.setPlainText(e.Description)
        self.lineEdit_6.setText(str(e.Price))

    def purchase_tickets(self):
        tickets_requested = self.spinBox.value()

        if tickets_requested <= 0:
            QMessageBox.warning(self, "Error", "Please select number of tickets.")
            return

        index = self.comboBox.currentIndex()
        event = self.events[index]
        today = datetime.now().date()
        if event.EventDate < today:
            QMessageBox.warning(self, "Error", "This event has already passed. Tickets cannot be purchased.")
            return
        event_id = event.EventID
        conn = pyodbc.connect(connection_string)
        cursor = conn.cursor()

        cursor.execute("""
            SELECT Capacity, Sold 
            FROM Event 
            WHERE EventID = ?
        """, (event_id,))

        row = cursor.fetchone()
        conn.close()

        capacity = row.Capacity
        sold = row.Sold
        available = capacity - sold

        if tickets_requested > available:
            QMessageBox.warning(self, "Error", f"Only {available} tickets available.")
            return

        price_per_ticket = event.Price
        total_amount = tickets_requested * price_per_ticket

        self.receipt_window = CheckoutUI(self.email, event, tickets_requested)
        self.receipt_window.show()
        self.close()

class CheckoutUI(QMainWindow):
    def __init__(self, email, event, tickets):
        super().__init__()
        uic.loadUi("7.ui", self)

        self.email = email
        self.event = event
        self.tickets = tickets

        self.load_receipt_data()

        self.pushButton_3.clicked.connect(self.confirm_order)
        self.pushButton_2.clicked.connect(self.go_back)

    def load_receipt_data(self):
        e = self.event

        self.lineEdit_2.setText(e.EventName)
        self.lineEdit_3.setText(e.EventDate.strftime("%m-%d-%y"))
        self.lineEdit_4.setText(e.Location)
        self.lineEdit_5.setText(str(self.tickets))

        total = float(e.Price) * int(self.tickets)
        self.lineEdit_6.setText(str(total))

        today = QDate.currentDate().toString("MM-dd-yyyy")
        self.lineEdit_7.setText(today)

    def confirm_order(self):
        try:
            e = self.event
            event_id = e.EventID
            total_amount = float(e.Price) * int(self.tickets)

            conn = pyodbc.connect(connection_string)
            cursor = conn.cursor()

            cursor.execute("""
                INSERT INTO [Order] (Email, EventID, Total_Tickets, Amount, PurchaseDate)
                VALUES (?, ?, ?, ?, GETDATE())
            """, (self.email, event_id, self.tickets, total_amount))

            cursor.execute("""
                UPDATE Event
                SET Sold = Sold + ?
                WHERE EventID = ?
            """, (self.tickets, event_id))

            conn.commit()
            conn.close()

            QMessageBox.information(self, "Success", "Order confirmed!")
            self.close()

        except Exception as ex:
            QMessageBox.warning(self, "Database Error", str(ex))

    def go_back(self):
        self.buy_window = BuyTicketsUI(self.email)
        self.buy_window.show()
        self.close()

class AddEventUI(QMainWindow):
    def __init__(self, admin_email):
        super(AddEventUI, self).__init__()
        uic.loadUi("4.ui", self)

        self.admin_email = admin_email
        self.pushButton.clicked.connect(self.add_event)

    def add_event(self):
        event_id = self.lineEdit_eventID.text()
        event_name = self.lineEdit_eventName.text()
        location = self.lineEdit_location.text()
        event_date = self.dateEdit_eventDate.date().toString("MM-dd-yyyy")
        start_time = self.timeEdit_start.time().toString("HH:mm")
        end_time = self.timeEdit_end.time().toString("HH:mm")
        description = self.textEdit_desc.toPlainText()
        price = self.lineEdit_price.text()
        capacity = self.spinBox.value()

        if event_id == "" or event_name == "" or capacity == "":
            QMessageBox.warning(self, "Error", "Please fill all required fields.")
            return

        try:
            connection = pyodbc.connect(connection_string)
            cursor = connection.cursor()
            count_query = """
                SELECT COUNT(*) 
                FROM Event
                WHERE EventID = ? 
                  AND EventName = ? 
                  AND Location = ? 
                  AND StartTime = ?
            """
            cursor.execute(count_query, (event_id, event_name, location, start_time))
            result = cursor.fetchone()[0]

            if result > 0:
                QMessageBox.warning(self, "Duplicate", "Event already exists!")
                connection.close()
                return
            insert_query = """
            INSERT INTO Event 
                (EventID, EventName, Location, EventDate, StartTime, EndTime,
                 Description, Price, Capacity, Sold, Email)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, 0, ?)
            """
            cursor.execute(insert_query, (
                int(event_id), event_name, location, event_date,
                start_time, end_time, description, float(price),
                int(capacity), self.admin_email
            ))

            connection.commit()
            connection.close()

            QMessageBox.information(self, "Success", "Event created successfully!")

        except Exception as e:
            QMessageBox.warning(self, "Database Error", str(e))

class CustomerOrdersUI(QMainWindow):
    def __init__(self, email):
        super(CustomerOrdersUI, self).__init__()
        uic.loadUi("9.ui", self)

        self.email = email
        self.load_orders()

        self.pushButton.clicked.connect(self.go_back) 


    def load_orders(self):
        conn = pyodbc.connect(connection_string)
        cursor = conn.cursor()

        query = """
            SELECT 
                o.OrderID,
                e.EventName,
                e.EventDate,
                o.Total_Tickets,
                o.Amount,
                o.PurchaseDate
            FROM [Order] o
            JOIN Event e ON o.EventID = e.EventID
            WHERE o.Email = ?
            ORDER BY o.PurchaseDate DESC
        """

        cursor.execute(query, (self.email,))
        rows = cursor.fetchall()
        conn.close()

        self.tableWidget.setRowCount(len(rows))
        self.tableWidget.setColumnCount(6)

        row_index = 0
        for row in rows:
            col_index = 0
            for value in row:
                self.tableWidget.setItem(row_index, col_index, QtWidgets.QTableWidgetItem(str(value)))
                col_index += 1
            row_index += 1

    def go_back(self):
        from_screen_8 = CustomerHomeUI(self.email)   
        from_screen_8.show()
        self.close()

class EditEventUI(QMainWindow):
    def __init__(self):
        super(EditEventUI, self).__init__()
        uic.loadUi("5.ui", self)

        self.comboBox_eventID.currentIndexChanged.connect(self.load_event)
        self.pushButton.clicked.connect(self.delete_event)

        self.load_event_ids()

    def load_event_ids(self):
        conn = pyodbc.connect(connection_string)
        cursor = conn.cursor()
        cursor.execute("SELECT EventID FROM Event")
        rows = cursor.fetchall()
        conn.close()

        for r in rows:
            self.comboBox_eventID.addItem(str(r[0]))

    def load_event(self):
        event_id = self.comboBox_eventID.currentText()

        conn = pyodbc.connect(connection_string)
        cursor = conn.cursor()

        cursor.execute("SELECT * FROM Event WHERE EventID = ?", (event_id,))
        e = cursor.fetchone()
        conn.close()

        if e:
            self.lineEdit_eventName.setText(e.EventName)
            self.lineEdit_location.setText(e.Location)
            self.lineEdit_eventDate.setText(e.EventDate.strftime("%m-%d-%y"))
            self.lineEdit_start.setText(e.StartTime.strftime("%H:%M"))
            self.lineEdit_end.setText(e.EndTime.strftime("%H:%M"))
            self.textEdit_desc.setPlainText(e.Description)
            self.lineEdit_price.setText(str(e.Price))
            self.lineEdit_capacity.setText(str(e.Capacity))
            self.lineEdit_sold.setText(str(e.Sold))

    def delete_event(self):
        event_id = self.comboBox_eventID.currentText()

        conn = pyodbc.connect(connection_string)
        cursor = conn.cursor()
        cursor.execute("DELETE FROM [Order] WHERE EventID = ?", (event_id,))
        cursor.execute("DELETE FROM Upcoming_Events WHERE EventID = ?", (event_id,))
        cursor.execute("DELETE FROM Modifying WHERE EventID = ?", (event_id,))
        cursor.execute("DELETE FROM Event WHERE EventID = ?", (event_id,))

        conn.commit()
        conn.close()

        QMessageBox.information(self, "Deleted", "Event and related orders deleted successfully!")

        self.comboBox_eventID.clear()
        self.load_event_ids()


def main():
    app = QApplication(sys.argv)
    window = LoginUI()
    window.show()
    sys.exit(app.exec())


if __name__ == "__main__":
    main()