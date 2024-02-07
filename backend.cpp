#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include "httplib.h"
#include <string>
#include <stack>

using namespace std;
using namespace httplib;

class Book {
public:
    char books_number[30];
    char Book_Name[50];
    char Author_Name[20];
    int No_Copies;

    void Report(ostream& os) const {
        os << books_number << setw(30) << Book_Name << setw(30) << Author_Name
            << setw(30) << No_Copies << endl;
    }
};

class Node {
public:
    Book data;
    Node* prev;
    Node* next;

    Node(const Book& b) : data(b), prev(nullptr), next(nullptr) {}
};

class DoublyLinkedList {
private:
    Node* head;
    Node* tail;

public:
    DoublyLinkedList() : head(nullptr), tail(nullptr) {}

    ~DoublyLinkedList() {
        // Cleanup: Free the memory occupied by the linked list
        Node* current = head;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
    }

    void AddNode(const Book& book) {
        Node* newNode = new Node(book); 
        if (!head) {
            head = tail = newNode;
        }
        else {
            newNode->prev = tail;
            tail->next = newNode;
            tail = newNode;
        }
    }

    void UpdateNode(const string& bookNumber, const Book& updatedBook) {
        Node* current = head;
        while (current) {
            if (strcmp(current->data.books_number, bookNumber.c_str()) == 0) {
                current->data = updatedBook;
                break;
            }
            current = current->next;
        }
    }

    void DeleteNode(const string& bookNumber) {
        Node* current = head;
        while (current) {
            if (strcmp(current->data.books_number, bookNumber.c_str()) == 0) {
                if (current->prev) {
                    current->prev->next = current->next;
                }
                else {
                    head = current->next;
                }
                if (current->next) {
                    current->next->prev = current->prev;
                }
                delete current;
                break;
            }
            current = current->next;
        }
    }

    string DisplayAllNodes() const {
        stringstream ss;
        Node* current = head;
        ss << left << setw(30) << "Book Number" << "|" <<
            left << setw(50) << "Book Name" << "|" <<
            left << setw(20) << "Author Name" << "|" <<
            left << setw(10) << "No. of Copies" << "|" << endl;
        while (current) {
            ss << left << setw(30) << current->data.books_number << "|" << left
                << setw(50) << current->data.Book_Name << "|" << left << setw(20)
                << current->data.Author_Name << "|" << left << setw(10)
                << current->data.No_Copies << "|" << endl;

            current = current->next;
        }
        return ss.str();
    }

    string SearchNode(const string& bookNumber) const {
        stringstream ss;
        Node* current = head;
        while (current) {
            if (strcmp(current->data.books_number, bookNumber.c_str()) == 0) {
                current->data.Report(ss);
                break;
            }
            current = current->next;
        }
        return ss.str();
    }

  

    void SaveToFile() const {
        ofstream outFile("book.txt");
        if (outFile.is_open()) {
            Node* current = head;
            while (current) {
                outFile << current->data.books_number << ","
                    << current->data.Book_Name << ","
                    << current->data.Author_Name << ","
                    << current->data.No_Copies << "\n";
                current = current->next;
            }
            outFile.close();
        }
    }

    void LoadFromFile() {
        ifstream inFile("book.txt");
        if (inFile.is_open()) {
            while (inFile) {
                Book book;
                if (inFile.getline(book.books_number, sizeof(book.books_number), ',') &&
                    inFile.getline(book.Book_Name, sizeof(book.Book_Name), ',') &&
                    inFile.getline(book.Author_Name, sizeof(book.Author_Name), ',') &&
                    (inFile >> book.No_Copies)) {
                    AddNode(book);

                    // Ignore the newline character after reading No_Copies
                    inFile.ignore(numeric_limits<streamsize>::max(), '\n');

                }
            }
            inFile.close();
        }
    }

    
   
};



class BookManager {
private:
    DoublyLinkedList bookList;
    stack<DoublyLinkedList> undoStack;

public:
    void AddBook(const Book& book) {
        bookList.AddNode(book);
        undoStack.push(bookList);
    }

    void UpdateBook(const string& bookNumber, const Book& updatedBook) {
        bookList.UpdateNode(bookNumber, updatedBook);
        undoStack.push(bookList);
    }

    void DeleteBook(const string& bookNumber) {
        bookList.DeleteNode(bookNumber);
        undoStack.push(bookList);
    }

    string DisplayAllBooks() const {
        return bookList.DisplayAllNodes();
    }

    string SearchBook(const string& bookNumber) const {
        return bookList.SearchNode(bookNumber);
    }

    void SaveBooksToFile() const {
        bookList.SaveToFile();
    }

    void LoadBooksFromFile() {
        bookList.LoadFromFile();
    }

    void Undo() {
        if (!undoStack.empty()) {
            undoStack.pop(); // Pop the current state
            if (!undoStack.empty()) {
                bookList = undoStack.top(); // Restore the previous state
            }
        }
    }
};

BookManager bookManager;

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void write_book_data(const Request& req, Response& res) {
    string bookNumber = req.get_param_value("bookNumber");
    string bookName = req.get_param_value("bookName");
    string authorName = req.get_param_value("authorName");
    int copies = stoi(req.get_param_value("copies"));

    Book newBook;
    strcpy_s(newBook.books_number, bookNumber.c_str());
    strcpy_s(newBook.Book_Name, bookName.c_str());
    strcpy_s(newBook.Author_Name, authorName.c_str());
    newBook.No_Copies = copies;

    bookManager.AddBook(newBook);

    res.set_content("Book added successfully!", "text/plain");
}

void display_all_books(const Request& req, Response& res) {
    res.set_content(bookManager.DisplayAllBooks(), "text/plain");
}

void search_book(const Request& req, Response& res) {
    string bookNumber = req.get_param_value("bookNumber");
    string result = bookManager.SearchBook(bookNumber);
    if (!result.empty()) {
        res.set_content(result, "text/plain");
    }
    else {
        res.set_content("Book not found!", "text/plain");
    }
}

void update_book(const httplib::Request& req, httplib::Response& res) {
    string bookNumber = req.get_param_value("bookNumber");
    string bookName = req.get_param_value("bookName");
    string authorName = req.get_param_value("authorName");
    int copies = stoi(req.get_param_value("copies"));

    Book updatedBook;
    strcpy_s(updatedBook.books_number, bookNumber.c_str());
    strcpy_s(updatedBook.Book_Name, bookName.c_str());
    strcpy_s(updatedBook.Author_Name, authorName.c_str());
    updatedBook.No_Copies = copies;

    bookManager.UpdateBook(bookNumber, updatedBook);

    res.set_content("Book updated successfully!", "text/plain");
}

void delete_book(const Request& req, Response& res) {
    string bookNumber = req.get_param_value("bookNumber");
    bookManager.DeleteBook(bookNumber);
    res.set_content("Book deleted successfully!", "text/plain");
}




void download_books(const Request& req, Response& res) {
    bookManager.SaveBooksToFile();

    ifstream inFile("book.txt");
    if (inFile.is_open()) {
        stringstream ss;
        ss << inFile.rdbuf();
        inFile.close();

        res.set_content(ss.str(), "text/plain");
        res.set_header("Content-Disposition",
            "attachment; filename=books_download.txt");
    }
    else {
        res.set_content("Error opening file for reading!", "text/plain");
    }
}

void delete_all_books(const Request& req, Response& res) {
    /*std::remove("book.txt");*/

    ofstream outFile("book.txt", ios::trunc); // Open the file with truncation
    outFile.close(); // Close the file to ensure changes take effect

    bookManager = BookManager(); // Reset book manager to clear all books
    res.set_content("All books deleted successfully!", "text/plain");
    ;
}

//void undo_operation(const Request& req, Response& res) {
//    bookManager.Undo();
//    res.set_content("Undo operation performed successfully!", "text/plain");
//}

int main() {
    bookManager.LoadBooksFromFile();

    httplib::Server svr;
    svr.Post("/update", update_book);
    svr.Post("/add", write_book_data);
    svr.Get("/delete", delete_book);
    svr.Get("/display", display_all_books);
    svr.Get("/search", search_book);
    svr.Get("/download", download_books);
    svr.Get("/delete-all", delete_all_books);
    //svr.Get("/undo", undo_operation); // Added endpoint for undo operation
    svr.listen("localhost", 8080);

    return 0;
}