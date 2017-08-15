#include <QCoreApplication>

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "node.h"
#include "btree.h"
#include "workingsettree.h"
#include <time.h>
using namespace std;

int insert_file(std::string filename, WorkingSetTree<int> &wst) {
    std::ifstream ifs;
    ifs.open(filename);

    // if file doesn't exist or cannot be read
    // print out error message and return errorlevel of 1
    if (!ifs.is_open()) {
        std::cout << filename << " cannot be opened for reading." << std::endl;
        return 1;
    }

    std::string line;

    // read the file line by line
    while (getline(ifs, line)) {
        wst.insert(stoi(line));
        //cout << "1";
    }
    //cout << endl;
    return 0;
}

int search_file(std::string filename, WorkingSetTree<int> &wst) {
    std::ifstream ifs;
    ifs.open(filename);

    // if file doesn't exist or cannot be read
    // print out error message and return errorlevel of 1
    if (!ifs.is_open()) {
        std::cout << filename << " cannot be opened for reading." << std::endl;
        return 1;
    }

    std::string line;

    // read the file line by line
    while (getline(ifs, line)) {
        wst.search(stoi(line));
    }

    return 0;
}

int insert_file_btree(std::string filename, BTree<int> &btree) {
    std::ifstream ifs;
    ifs.open(filename);

    // if file doesn't exist or cannot be read
    // print out error message and return errorlevel of 1
    if (!ifs.is_open()) {
        std::cout << filename << " cannot be opened for reading." << std::endl;
        return 1;
    }

    std::string line;
    int total_levels_traversed = 0;
    // read the file line by line
    while (getline(ifs, line)) {
        total_levels_traversed += btree.insert(stoi(line));
    }

    cout << "Insert complete. Total levels traversed: " << total_levels_traversed << endl;

    return 0;
}

int search_file_btree(std::string filename, BTree<int> &btree) {
    std::ifstream ifs;
    ifs.open(filename);

    // if file doesn't exist or cannot be read
    // print out error message and return errorlevel of 1
    if (!ifs.is_open()) {
        std::cout << filename << " cannot be opened for reading." << std::endl;
        return 1;
    }

    std::string line;

    // read the file line by line
    while (getline(ifs, line)) {
        btree.search(stoi(line));
    }

    return 0;
}

int delete_file_btree(std::string filename, BTree<int> &btree) {
    std::ifstream ifs;
    ifs.open(filename);

    // if file doesn't exist or cannot be read
    // print out error message and return errorlevel of 1
    if (!ifs.is_open()) {
        std::cout << filename << " cannot be opened for reading." << std::endl;
        return 1;
    }

    std::string line;

    // read the file line by line
    while (getline(ifs, line)) {
        btree.remove(stoi(line));
    }

    return 0;
}

void run_btree_command_line() {
    cout << "B-Tree: assumed to only hold int values for testing via command line" << endl;

    string operations_str = "\nPlease enter number (1) insert\t (2) search\t (3) remove\t (4) print\t";
    operations_str +=
        " (5) print in order of MRU to LRU\t (6) remove LRU\t (7) insert to back of linked list\t (8) quit\nfollowed by numbers to insert/delete";
    cout << "Please enter the minimum degree of the B-Tree" << endl;
    string input = "";
    getline(cin, input);
    BTree<int> btree(stoi(input));

    bool end_program = false;
    while (!end_program) {
        cout << operations_str << endl;

        getline(cin, input);
        istringstream iss(input);
        string option = "";
        if (iss >> option) {
            string num_str;
            switch (stoi(option)) {
            case 1: // insert elements
                while (iss >> num_str) {
                    btree.insert(stoi(num_str));
                }
                break;
            case 2: // search for elements
                while (iss >> num_str) {
                    pair<Node<int>&, int> node_index = btree.search(stoi(num_str));
                    cout << num_str << ": ";
                    if (node_index.second == -1) {
                        cout << "element not found" << endl;
                    }
                    else {
                        cout << "elemnt found at index " << node_index.second << " of the following node..."
                            << node_to_string(node_index.first) << endl;
                    }
                }
                break;
            case 3: // delete elements
                while (iss >> num_str) {
                    if (btree.remove(stoi(num_str))) {
                        cout << num_str << ": deleted successfully" << endl;
                    }
                    else {
                        cout << num_str << ": not found. Not deleted" << endl;
                    }
                }
                break;
            case 4: // print current tree
                cout << "current height: " << btree.get_height() << "; max height: " << btree.get_max_height() << endl;
                cout << btree.to_string() << endl;
                break;
            case 5:
                cout << btree.print_ordered_mru() << endl;
                cout << btree.print_ordered_tail() << endl;
                break;
            case 6:
                cout << "removed LRU: " << to_string(btree.remove_lru()) << endl;
                break;
            case 7:
                while (iss >> num_str) {
                    btree.insert_lru(stoi(num_str));
                }
                break;
            case 8: // quit program
                end_program = true;
                cout << "Bye" << endl;
                break;
            default:
                cout << "Please enter a valid option" << endl;
            }
        }
    }
}

void run_workingsettree_command_line() {
    cout << "Working Set Tree: assumed to only hold int values for testing via command line" << endl;

    string operations_str = "\nPlease enter number (1) insert\t (2) search\t (3) remove\t (4) print (5) print list\t";
    operations_str +=
        "(6) quit\t\tfollowed by numbers to insert/delete";
    cout << "Please enter the minimum degree of the b tree" << endl;
    string input = "";
    getline(cin, input);
    WorkingSetTree<int> wst(stoi(input));

    bool end_program = false;
    while (!end_program) {
        cout << operations_str << endl;

        getline(cin, input);
        istringstream iss(input);
        string option = "";
        if (iss >> option) {
            string num_str;
            switch (stoi(option)) {
            case 1: // insert elements
                while (iss >> num_str) {
                    wst.insert(stoi(num_str));
                }
                break;
            /*case 2: // search for elements
                while (iss >> num_str) {
                    pair<Node<int>&, int> node_index = wst.search(stoi(num_str));
                    cout << num_str << ": ";
                    if (node_index.second == -1) {
                        cout << "element not found" << endl;
                    }
                    else {
                        cout << "elemnt found at index " << node_index.second << " of the following node..."
                            << node_to_string(node_index.first) << endl;
                    }
                }
                break;*/
                case 2: // search for elements
                    while (iss >> num_str) {
                        bool is_found = wst.search(stoi(num_str));
                        cout << num_str << ": ";
                        if (is_found) {
                            cout << "element is found" << endl;
                        }
                        else {
                            cout << "element is NOT found" << endl;
                        }
                    }
                    break;
            case 3: // delete elements
                while (iss >> num_str) {
                    if (wst.remove(stoi(num_str))) {
                        cout << num_str << ": deleted successfully" << endl;
                    }
                    else {
                        cout << num_str << ": not found. Not deleted" << endl;
                    }
                }
                break;
            case 4: // print current tree
                cout << wst.to_string() << endl;
                break;
            case 5: // print list
                cout << wst.print_list() << endl;
                break;
            case 6: // quit
                end_program = true;
                cout << "Bye" << endl;
                break;
            default:
                cout << "Please enter a valid option" << endl;
            }
        }
    }
}

void time_btree_sec() {

    time_t start, end;
    double seconds;

    BTree<int> btree;

    time(&start);
    insert_file_btree("data\\original_unique_1_500000", btree);
    time(&end);
    seconds = difftime(end, start);
    cout << "Time taken to insert 500,000 elements into b-tree: " << seconds << endl;
    cout << "btree size: " << btree.size() << endl;
    //string file_string = "data\\unique\\unique";


    // searching
    time(&start);
    search_file_btree("data\\p1_p99_500_50000_0", btree);
    time(&end);

    seconds = difftime(end, start);

    cout << "Time taken to search 50,000 elements in b-tree: " << seconds << endl;


    // deleting
    time(&start);
    delete_file_btree("data\\p1_p99_500_50000_2", btree);
    time(&end);

    seconds = difftime(end, start);

    cout << "Time taken to delete 50,000 elements in b-tree: " << seconds << endl;


    // inserting
    time(&start);
    insert_file_btree("data\\p1_p99_500_50000_1", btree);
    time(&end);

    seconds = difftime(end, start);

    cout << "Time taken to insert 50,000 elements into the b-tree: " << seconds << endl;

}

void time_btree_ms() {

    clock_t t;

    BTree<int> btree;

    t = clock();
    insert_file_btree("data\\original_unique_1_500000", btree);
    t = clock() - t;
    cout << "Time taken to insert 500,000 elements into b-tree: " << t << endl;
    cout << "time: " << t << " miliseconds" << endl;
    cout << CLOCKS_PER_SEC << " clocks per second" << endl;
    cout << "time: " << t*1.0 / CLOCKS_PER_SEC << " seconds" << endl;
    cout << "btree size: " << btree.size() << endl;
    //string file_string = "data\\unique\\unique";


    // searching
    t = clock();
    search_file_btree("data\\p1_p99_500_50000_0", btree);
    t = clock() - t;

    cout << "Time taken to search 50,000 elements in b-tree: " << t << endl;
    cout << "time: " << t << " miliseconds" << endl;
    cout << CLOCKS_PER_SEC << " clocks per second" << endl;
    cout << "time: " << t*1.0 / CLOCKS_PER_SEC << " seconds" << endl;


    // deleting
    t = clock();
    delete_file_btree("data\\p1_p99_500_50000_2", btree);
    t = clock() - t;

    cout << "Time taken to delete 50,000 elements in b-tree: " << t << endl;
    cout << "time: " << t << " miliseconds" << endl;
    cout << CLOCKS_PER_SEC << " clocks per second" << endl;
    cout << "time: " << t*1.0 / CLOCKS_PER_SEC << " seconds" << endl;


    // inserting
    t = clock();
    insert_file_btree("data\\p1_p99_500_50000_1", btree);
    t = clock() - t;

    cout << "Time taken to insert 50,000 elements into the b-tree: " << t << endl;
    cout << "time: " << t << " miliseconds" << endl;
    cout << CLOCKS_PER_SEC << " clocks per second" << endl;
    cout << "time: " << t*1.0 / CLOCKS_PER_SEC << " seconds" << endl;

}

void time_wst_sec() {

    time_t start, end;
    double seconds;

    WorkingSetTree<int> wst;

    time(&start);
    //insert_file("data\\original_unique_1_500000", wst);
    insert_file("data\\original_unique_1_500000", wst);
    time(&end);
    seconds = difftime(end, start);
    cout << "Time taken to insert 500,000 elements: " << seconds << endl;
    cout << "insert completed. size: " << wst.size() << endl;

    //string file_string = "data\\unique\\unique0";

    time(&start);  // get current time; same as: now = time(NULL)
                   //search_file("data\\p1_p99_500", wst);
    search_file("data\\p1_p99_500_50000_0", wst);
    time(&end);

    seconds = difftime(end, start);

    //cout << "Time taken to search 500,000 elements: " << seconds << endl;
    cout << "Time taken to search 50,000 elements: " << seconds << endl;

}

void time_wst_ms() {

    clock_t t;

    WorkingSetTree<int> wst;

    t = clock();
    //insert_file("data\\original_unique_1_500000", wst);
    insert_file("data\\original_unique_1_500000", wst);
    t = clock() - t;
    cout << "Time taken to insert 500,000 elements: " << t << endl;
    cout << "insert completed. size: " << wst.size() << endl;
    cout << "time: " << t << " miliseconds" << endl;
    cout << CLOCKS_PER_SEC << " clocks per second" << endl;
    cout << "time: " << t*1.0 / CLOCKS_PER_SEC << " seconds" << endl;

    //string file_string = "data\\unique\\unique0";

    t = clock(); // get current time; same as: now = time(NULL)
                 //search_file("data\\p1_p99_500", wst);
    search_file("data\\p1_p99_500_50000_0", wst);
    t = clock() - t;

    //cout << "Time taken to search 500,000 elements: " << seconds << endl;
    cout << "Time taken to search 50,000 elements: " << t << endl;
    cout << "time: " << t << " miliseconds" << endl;
    cout << CLOCKS_PER_SEC << " clocks per second" << endl;
    cout << "time: " << t*1.0 / CLOCKS_PER_SEC << " seconds" << endl;

}

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);

//    return a.exec();

    //run_btree_command_line();
        run_workingsettree_command_line();
        //time_btree_sec();
        //time_btree_ms();

//        cout << "\n\n" << endl;

        //time_wst_sec();
//        time_wst_ms();

        return 0;
}
