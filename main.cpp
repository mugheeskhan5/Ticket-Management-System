#include <iostream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <limits>
using namespace std;
using namespace chrono;

class UserBST;

struct User {
    int id;
    string name, department;
    int ticketsSubmitted;

    User(string n, string d, int uid = 0) : name(n), department(d), ticketsSubmitted(0) {
        id = (uid != 0) ? uid : (1000 + rand() % 9000);
    }

    void print() {
        cout << "ID: " << id << " | Name: " << name << " | Dept: " << department << " | Tickets Submitted: " << ticketsSubmitted << endl;
    }
};

struct BSTNode {
    User* user;
    BSTNode* left;
    BSTNode* right;
    BSTNode(User* u) : user(u), left(nullptr), right(nullptr) {}
};

class UserBST {
    BSTNode* root;

    BSTNode* insertNode(BSTNode* node, User* user) {
        if (!node) return new BSTNode(user);
        if (user->id < node->user->id)
            node->left = insertNode(node->left, user);
        else if (user->id > node->user->id)
            node->right = insertNode(node->right, user);
        return node;
    }

    User* searchNode(BSTNode* node, int id) {
        if (!node) return nullptr;
        if (id == node->user->id) return node->user;
        return (id < node->user->id) ? searchNode(node->left, id) : searchNode(node->right, id);
    }

    void inorder(BSTNode* node, vector<User*>& out) {
        if (!node) return;
        inorder(node->left, out);
        out.push_back(node->user);
        inorder(node->right, out);
    }

    void destroyTree(BSTNode* node) {
        if (!node) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node->user;
        delete node;
    }

public:
    UserBST() : root(nullptr) {}
    ~UserBST() { destroyTree(root); }

    void insert(User* user) {
        root = insertNode(root, user);
    }

    User* find(int id) {
        return searchNode(root, id);
    }

    User* findByName(string name) {
        vector<User*> all = getAllUsers();
        for (int i = 0; i < all.size(); i++)
            if (all[i]->name == name) return all[i];
        return nullptr;
    }

    vector<User*> getAllUsers() {
        vector<User*> users;
        inorder(root, users);
        return users;
    }

    void displayAll() {
        cout << "\n=== USER PROFILES (BST Inorder) ===" << endl;
        vector<User*> users = getAllUsers();
        if (users.empty()) { cout << "No users registered." << endl; return; }
        for (int i = 0; i < users.size(); i++) users[i]->print();
    }
};

struct Ticket {
    int id, priority, userId, waitingMinutes;
    string type, description;
    time_point<system_clock> created, closed;
    bool open;

    Ticket(int uid, int p, string t, string desc = "") : priority(p), userId(uid), waitingMinutes(0), type(t), description(desc), open(true) {
        id = 10000 + (rand() % 90000);
        created = system_clock::now();
    }

    void close() {
        open = false;
        closed = system_clock::now();
    }

    void updateWaitingTime() {
        if (open)
            waitingMinutes = (int)duration_cast<minutes>(system_clock::now() - created).count();
    }

    void boostPriority() {
        if (open && waitingMinutes > 30)
            priority = min(10, priority + 2);
    }

    void print(UserBST& userBST) {
        User* user = userBST.find(userId);
        cout << "Ticket ID: " << id << " | User: " << (user ? user->name : "Unknown")
             << " | Priority: " << priority << " | Type: " << type
             << " | Wait: " << waitingMinutes << " min"
             << " | Status: " << (open ? "Open" : "Closed");
        if (!description.empty())
            cout << " | Desc: " << description;
        cout << endl;
    }
};

struct TicketNode {
    Ticket* ticket;
    TicketNode* next;
    TicketNode(Ticket* t) : ticket(t), next(nullptr) {}
};

class TicketLinkedList {
    TicketNode* head;
    int size;

public:
    TicketLinkedList() : head(nullptr), size(0) {}

    ~TicketLinkedList() {
        TicketNode* curr = head;
        while (curr) {
            TicketNode* tmp = curr->next;
            delete curr->ticket;
            delete curr;
            curr = tmp;
        }
    }

    void add(Ticket* t) {
        TicketNode* n = new TicketNode(t);
        n->next = head;
        head = n;
        size++;
    }

    Ticket* find(int id) {
        TicketNode* curr = head;
        while (curr) {
            if (curr->ticket->id == id) return curr->ticket;
            curr = curr->next;
        }
        return nullptr;
    }

    vector<Ticket*> getAllTickets() {
        vector<Ticket*> tickets;
        TicketNode* curr = head;
        while (curr) {
            tickets.push_back(curr->ticket);
            curr = curr->next;
        }
        return tickets;
    }

    void printAll(UserBST& userBST) {
        if (!head) { cout << "No tickets in archive." << endl; return; }
        TicketNode* curr = head;
        while (curr) {
            curr->ticket->print(userBST);
            curr = curr->next;
        }
    }

    int getSize() { return size; }
};

struct Agent {
    string name, type;
    Ticket* tickets[5];
    int assigned;
    bool available;

    Agent(string n, string t) : name(n), type(t), assigned(0), available(true) {
        for (int i = 0; i < 5; i++) tickets[i] = nullptr;
    }

    bool assign(Ticket* t) {
        if (available && assigned < 5 && type == t->type) {
            tickets[assigned++] = t;
            if (assigned >= 5) available = false;
            return true;
        }
        return false;
    }

    Ticket* resolve() {
        if (assigned == 0) return nullptr;
        Ticket* t = tickets[0];
        for (int i = 1; i < assigned; i++)
            tickets[i - 1] = tickets[i];
        tickets[--assigned] = nullptr;
        available = true;
        return t;
    }

    void print() {
        cout << "Agent: " << name << " | Type: " << type
             << " | Load: " << assigned << "/5"
             << " | Status: " << (available ? "Available" : "Full") << endl;
    }
};

class AgentManager {
    vector<Agent*> agents;

public:
    ~AgentManager() {
        for (int i = 0; i < agents.size(); i++) delete agents[i];
    }

    void addAgent(Agent* a) { agents.push_back(a); }

    bool assignTicket(Ticket* t) {
        Agent* bestAgent = nullptr;
        int minLoad = 6;

        for (int i = 0; i < agents.size(); i++)
            if (agents[i]->type == t->type && agents[i]->available && agents[i]->assigned < minLoad) {
                minLoad = agents[i]->assigned;
                bestAgent = agents[i];
            }

        if (!bestAgent)
            for (int i = 0; i < agents.size(); i++)
                if (agents[i]->type == t->type && agents[i]->assigned < minLoad) {
                    minLoad = agents[i]->assigned;
                    bestAgent = agents[i];
                }

        if (bestAgent && bestAgent->assign(t)) {
            cout << "  Ticket " << t->id << " assigned to " << bestAgent->name << endl;
            return true;
        }
        cout << "  No " << t->type << " agent available for ticket " << t->id << endl;
        return false;
    }

    Ticket* resolveOne() {
        for (int i = 0; i < agents.size(); i++)
            if (agents[i]->assigned > 0) return agents[i]->resolve();
        return nullptr;
    }

    void printAll() {
        cout << "\n=== AGENTS ===" << endl;
        if (agents.empty()) { cout << "No agents registered." << endl; return; }
        for (int i = 0; i < agents.size(); i++) agents[i]->print();
    }

    void bubbleSort() {
        int n = (int)agents.size();
        if (n < 2) return;
        for (int i = 0; i < n - 1; i++)
            for (int j = 0; j < n - i - 1; j++)
                if (agents[j]->assigned > agents[j + 1]->assigned)
                    swap(agents[j], agents[j + 1]);
    }
};

class PriorityQueue {
    vector<Ticket*> heap;
    string type;

    void heapifyUp(int i) {
        while (i > 0) {
            int parent = (i - 1) / 2;
            if (heap[parent]->priority < heap[i]->priority) {
                swap(heap[i], heap[parent]);
                i = parent;
            } else break;
        }
    }

    void heapifyDown(int i) {
        int n = (int)heap.size();
        int largest = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l < n && heap[l]->priority > heap[largest]->priority) largest = l;
        if (r < n && heap[r]->priority > heap[largest]->priority) largest = r;
        if (largest != i) {
            swap(heap[i], heap[largest]);
            heapifyDown(largest);
        }
    }

public:
    PriorityQueue(string t) : type(t) {}

    void enqueue(Ticket* t) {
        heap.push_back(t);
        heapifyUp((int)heap.size() - 1);
    }

    Ticket* dequeue() {
        if (heap.empty()) return nullptr;
        Ticket* top = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty()) heapifyDown(0);
        return top;
    }

    Ticket* peek() { return heap.empty() ? nullptr : heap[0]; }

    void display(UserBST& userBST) {
        cout << "\n=== " << type << " QUEUE (" << heap.size() << " ticket(s)) ===" << endl;
        if (heap.empty()) { cout << "Empty" << endl; return; }
        vector<Ticket*> tmp = heap;
        sort(tmp.begin(), tmp.end(), [](Ticket* a, Ticket* b) { return a->priority > b->priority; });
        for (int i = 0; i < tmp.size(); i++) {
            cout << i + 1 << ". ";
            tmp[i]->print(userBST);
        }
    }

    string getType() { return type; }
    int getSize() { return (int)heap.size(); }
    bool empty() { return heap.empty(); }
};

class ResolutionStack {
    stack<Ticket*> stk;

public:
    void push(Ticket* t) { stk.push(t); }

    Ticket* pop() {
        if (stk.empty()) return nullptr;
        Ticket* t = stk.top();
        stk.pop();
        return t;
    }

    Ticket* peek() { return stk.empty() ? nullptr : stk.top(); }

    void printAll(UserBST& userBST) {
        cout << "\n=== RESOLVED TICKETS (Stack - LIFO) ===" << endl;
        if (stk.empty()) { cout << "No resolved tickets yet." << endl; return; }
        stack<Ticket*> tmp = stk;
        vector<Ticket*> tickets;
        while (!tmp.empty()) { tickets.push_back(tmp.top()); tmp.pop(); }
        for (int i = 0; i < tickets.size(); i++) {
            cout << i + 1 << ". ";
            tickets[i]->print(userBST);
        }
    }

    bool empty() { return stk.empty(); }
    int size() { return (int)stk.size(); }
};

class SortingSearchManager {
public:
    static Ticket* binarySearchTickets(vector<Ticket*>& tickets, int ticketId) {
        int left = 0, right = (int)tickets.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (tickets[mid]->id == ticketId) return tickets[mid];
            else if (tickets[mid]->id < ticketId) left = mid + 1;
            else right = mid - 1;
        }
        return nullptr;
    }

    static User* binarySearchUsers(vector<User*>& users, int userId) {
        int left = 0, right = (int)users.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (users[mid]->id == userId) return users[mid];
            else if (users[mid]->id < userId) left = mid + 1;
            else right = mid - 1;
        }
        return nullptr;
    }

    static void quickSort(vector<Ticket*>& tickets, int low, int high, bool byPriority = true) {
        if (low < high) {
            int pi = partition(tickets, low, high, byPriority);
            quickSort(tickets, low, pi - 1, byPriority);
            quickSort(tickets, pi + 1, high, byPriority);
        }
    }

    static void mergeSort(vector<Ticket*>& tickets, int left, int right, bool byPriority = true) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSort(tickets, left, mid, byPriority);
            mergeSort(tickets, mid + 1, right, byPriority);
            mergeParts(tickets, left, mid, right, byPriority);
        }
    }

private:
    static int partition(vector<Ticket*>& tickets, int low, int high, bool byPriority) {
        Ticket* pivot = tickets[high];
        int i = low - 1;
        for (int j = low; j < high; j++) {
            bool cond = byPriority ? tickets[j]->priority > pivot->priority : tickets[j]->id < pivot->id;
            if (cond) swap(tickets[++i], tickets[j]);
        }
        swap(tickets[i + 1], tickets[high]);
        return i + 1;
    }

    static void mergeParts(vector<Ticket*>& tickets, int left, int mid, int right, bool byPriority) {
        int n1 = mid - left + 1, n2 = right - mid;
        vector<Ticket*> L(n1), R(n2);
        for (int i = 0; i < n1; i++) L[i] = tickets[left + i];
        for (int j = 0; j < n2; j++) R[j] = tickets[mid + 1 + j];
        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            bool cond = byPriority ? L[i]->priority >= R[j]->priority : L[i]->id <= R[j]->id;
            tickets[k++] = cond ? L[i++] : R[j++];
        }
        while (i < n1) tickets[k++] = L[i++];
        while (j < n2) tickets[k++] = R[j++];
    }
};

class TicketSystem {
    TicketLinkedList allTickets;
    AgentManager agentMgr;
    ResolutionStack resolvedStack;
    UserBST userBST;
    PriorityQueue* queues[4];
    bool adminMode;

    PriorityQueue* getQueue(string type) {
        if (type == "IT") return queues[0];
        if (type == "Admin") return queues[1];
        if (type == "Accounts") return queues[2];
        if (type == "Academics") return queues[3];
        return nullptr;
    }

    void clearInput() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    void pauseForUser() {
        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void displayMenu(string title, vector<string> options) {
        cout << "\n" << string(50, '=') << "\n" << title << "\n" << string(50, '=') << "\n";
        for (int i = 0; i < options.size(); i++)
            cout << "[" << i + 1 << "] " << options[i] << "\n";
        cout << "[0] " << (adminMode ? "Back" : "Exit") << "\nChoice: ";
    }

    void addTicket(int userId, int priority, string type, string desc = "") {
        if (priority < 1 || priority > 10) {
            cout << "Invalid priority. Setting to 5." << endl;
            priority = 5;
        }
        PriorityQueue* q = getQueue(type);
        if (!q) {
            cout << "Invalid type. Must be IT, Admin, Accounts, or Academics." << endl;
            return;
        }
        Ticket* t = new Ticket(userId, priority, type, desc);
        allTickets.add(t);
        q->enqueue(t);
        User* user = userBST.find(userId);
        if (user) user->ticketsSubmitted++;
        cout << "Ticket created! ID: " << t->id << " | Type: " << type << " | Priority: " << priority << endl;
    }

    void assignTickets() {
        cout << "\n=== ASSIGNING TICKETS ===" << endl;
        bool any = false;
        for (int i = 0; i < 4; i++) {
            Ticket* t = queues[i]->peek();
            if (t && agentMgr.assignTicket(t)) {
                queues[i]->dequeue();
                any = true;
            }
        }
        if (!any) cout << "No tickets could be assigned." << endl;
    }

    void resolveTicket() {
        Ticket* t = agentMgr.resolveOne();
        if (t) {
            t->close();
            resolvedStack.push(t);
            cout << "Ticket " << t->id << " resolved." << endl;
        } else {
            cout << "No assigned tickets to resolve." << endl;
        }
    }

    void searchTicket() {
        cout << "\nSearch by:\n[1] Ticket ID\n[2] User ID\n[3] Binary Search\nChoice: ";
        int choice;
        if (!(cin >> choice)) { clearInput(); return; }

        if (choice == 1) {
            int id; cout << "Ticket ID: "; cin >> id;
            Ticket* t = allTickets.find(id);
            if (t) { cout << "\nFound:\n"; t->print(userBST); }
            else cout << "Ticket not found." << endl;

        } else if (choice == 2) {
            int uid; cout << "User ID: "; cin >> uid;
            User* u = userBST.find(uid);
            if (u) {
                cout << "\nUser:\n"; u->print();
                cout << "\nTickets:\n";
                vector<Ticket*> tickets = allTickets.getAllTickets();
                bool found = false;
                for (int i = 0; i < tickets.size(); i++)
                    if (tickets[i]->userId == uid) { tickets[i]->print(userBST); found = true; }
                if (!found) cout << "(none)" << endl;
            } else {
                cout << "User not found." << endl;
            }

        } else if (choice == 3) {
            vector<Ticket*> tickets = allTickets.getAllTickets();
            if (tickets.empty()) { cout << "No tickets available." << endl; return; }
            SortingSearchManager::quickSort(tickets, 0, (int)tickets.size() - 1, false);
            int id; cout << "Ticket ID: "; cin >> id;
            Ticket* r = SortingSearchManager::binarySearchTickets(tickets, id);
            if (r) { cout << "\nFound via Binary Search:\n"; r->print(userBST); }
            else cout << "Ticket not found." << endl;
        }
    }

    void sortAndDisplay() {
        vector<Ticket*> tickets = allTickets.getAllTickets();
        if (tickets.empty()) { cout << "No tickets to sort." << endl; return; }

        cout << "\nSort algorithm:\n[1] Quick Sort\n[2] Merge Sort\n[3] Sort by Wait Time\nChoice: ";
        int alg; cin >> alg;

        if (alg == 3) {
            sort(tickets.begin(), tickets.end(), [](Ticket* a, Ticket* b) {
                return a->waitingMinutes > b->waitingMinutes;
            });
            cout << "Sorted by wait time." << endl;
        } else {
            cout << "Sort by:\n[1] Priority\n[2] Ticket ID\nChoice: ";
            int by; cin >> by;
            bool byPriority = (by != 2);
            int n = (int)tickets.size();
            if (alg == 1) {
                SortingSearchManager::quickSort(tickets, 0, n - 1, byPriority);
                cout << "Quick Sorted." << endl;
            } else if (alg == 2) {
                SortingSearchManager::mergeSort(tickets, 0, n - 1, byPriority);
                cout << "Merge Sorted." << endl;
            } else {
                cout << "Invalid option." << endl;
                return;
            }
        }

        cout << "\n=== SORTED TICKETS ===" << endl;
        for (int i = 0; i < tickets.size(); i++) tickets[i]->print(userBST);
    }

    void demoBinarySearchUsers() {
        vector<User*> users = userBST.getAllUsers();
        if (users.empty()) { cout << "No users registered." << endl; return; }

        sort(users.begin(), users.end(), [](User* a, User* b) { return a->id < b->id; });

        cout << "\nUsers (sorted by ID):" << endl;
        for (int i = 0; i < users.size(); i++) users[i]->print();

        int id; cout << "\nUser ID to search: "; cin >> id;
        User* r = SortingSearchManager::binarySearchUsers(users, id);
        if (r) { cout << "\nFound via Binary Search:\n"; r->print(); }
        else cout << "User not found." << endl;
    }

    void updatePriorities() {
        cout << "\n=== UPDATING PRIORITIES ===" << endl;
        int boosted = 0;
        vector<Ticket*> tickets = allTickets.getAllTickets();
        for (int i = 0; i < tickets.size(); i++) {
            if (!tickets[i]->open) continue;
            tickets[i]->updateWaitingTime();
            int old = tickets[i]->priority;
            tickets[i]->boostPriority();
            if (tickets[i]->priority > old) {
                cout << "Ticket " << tickets[i]->id << " boosted: " << old << " -> " << tickets[i]->priority << endl;
                boosted++;
            }
        }
        cout << boosted << " ticket(s) boosted." << endl;
    }

    void adminMenu() {
        adminMode = true;
        while (adminMode) {
            displayMenu("ADMIN PANEL", {
                "Add Ticket",
                "Assign Tickets to Agents",
                "Resolve a Ticket",
                "View All Tickets",
                "View Sorted Tickets",
                "Search Ticket / User",
                "View Priority Queues",
                "View Resolved Stack",
                "View Agents",
                "Sort Agents by Workload",
                "Update Priorities",
                "View Users (BST)",
                "Add New User",
                "Binary Search Users",
                "Stack Operations"
            });

            int choice;
            if (!(cin >> choice)) { clearInput(); continue; }
            clearInput();

            if (choice == 0) { adminMode = false; break; }

            switch (choice) {
                case 1: {
                    int uid, p;
                    string t, d;
                    cout << "User ID: "; cin >> uid;
                    cout << "Priority: "; cin >> p;
                    cout << "Type (IT/Admin/Accounts/Academics): "; cin >> t;
                    cout << "Description: "; clearInput(); getline(cin, d);
                    addTicket(uid, p, t, d);
                    break;
                }
                case 2: assignTickets(); break;
                case 3: resolveTicket(); break;
                case 4: cout << "\n=== ALL TICKETS ===" << endl; allTickets.printAll(userBST); break;
                case 5: sortAndDisplay(); break;
                case 6: searchTicket(); break;
                case 7:
                    for (int i = 0; i < 4; i++) queues[i]->display(userBST);
                    break;
                case 8: resolvedStack.printAll(userBST); break;
                case 9: agentMgr.printAll(); break;
                case 10: agentMgr.bubbleSort(); cout << "Agents sorted by workload." << endl; break;
                case 11: updatePriorities(); break;
                case 12: userBST.displayAll(); break;
                case 13: {
                    string n, d;
                    cout << "Name: "; getline(cin, n);
                    cout << "Department: "; getline(cin, d);
                    if (n.empty()) { cout << "Name cannot be empty." << endl; break; }
                    userBST.insert(new User(n, d));
                    cout << "User added." << endl;
                    break;
                }
                case 14: demoBinarySearchUsers(); break;
                case 15: {
                    cout << "Stack operation:\n[1] Peek\n[2] Pop\nChoice: ";
                    int op; cin >> op;
                    if (op == 1) {
                        Ticket* t = resolvedStack.peek();
                        if (t) { cout << "Top: "; t->print(userBST); }
                        else cout << "Stack is empty." << endl;
                    } else if (op == 2) {
                        Ticket* t = resolvedStack.pop();
                        if (t) { cout << "Popped: "; t->print(userBST); }
                        else cout << "Stack is empty." << endl;
                    }
                    break;
                }
                default: cout << "Invalid choice." << endl;
            }

            clearInput();
            pauseForUser();
        }
    }

    void userMenu() {
        adminMode = false;
        while (true) {
            displayMenu("USER PANEL", {
                "Submit a Ticket",
                "Check My Tickets",
                "View a Queue",
                "View All Open Tickets",
                "View My Profile"
            });

            int choice;
            if (!(cin >> choice)) { clearInput(); continue; }
            clearInput();

            if (choice == 0) break;

            switch (choice) {
                case 1: {
                    string name, type, desc;
                    int priority;
                    cout << "Your Name: "; getline(cin, name);
                    cout << "Priority: "; cin >> priority; clearInput();
                    cout << "Type (IT/Admin/Accounts/Academics): "; cin >> type; clearInput();
                    cout << "Description: "; getline(cin, desc);
                    User* u = userBST.findByName(name);
                    if (!u) {
                        u = new User(name, "General");
                        userBST.insert(u);
                        cout << "New user registered. ID: " << u->id << endl;
                    }
                    addTicket(u->id, priority, type, desc);
                    break;
                }
                case 2: {
                    string name;
                    cout << "Your Name: "; getline(cin, name);
                    User* u = userBST.findByName(name);
                    if (u) {
                        cout << "\nYour Tickets:" << endl;
                        vector<Ticket*> ts = allTickets.getAllTickets();
                        bool found = false;
                        for (int i = 0; i < ts.size(); i++)
                            if (ts[i]->userId == u->id) { ts[i]->print(userBST); found = true; }
                        if (!found) cout << "(none)" << endl;
                    } else {
                        cout << "User not found." << endl;
                    }
                    break;
                }
                case 3: {
                    string type;
                    cout << "Queue type (IT/Admin/Accounts/Academics): "; cin >> type; clearInput();
                    PriorityQueue* q = getQueue(type);
                    if (q) q->display(userBST);
                    else cout << "Invalid queue type." << endl;
                    break;
                }
                case 4: {
                    cout << "\n=== OPEN TICKETS ===" << endl;
                    vector<Ticket*> ts = allTickets.getAllTickets();
                    bool found = false;
                    for (int i = 0; i < ts.size(); i++)
                        if (ts[i]->open) { ts[i]->print(userBST); found = true; }
                    if (!found) cout << "No open tickets." << endl;
                    break;
                }
                case 5: {
                    string name;
                    cout << "Your Name: "; getline(cin, name);
                    User* u = userBST.findByName(name);
                    if (u) u->print();
                    else cout << "User not found." << endl;
                    break;
                }
                default: cout << "Invalid choice." << endl;
            }

            clearInput();
            pauseForUser();
        }
    }

public:
    TicketSystem() : adminMode(false) {
        srand(time(nullptr));

        queues[0] = new PriorityQueue("IT");
        queues[1] = new PriorityQueue("Admin");
        queues[2] = new PriorityQueue("Accounts");
        queues[3] = new PriorityQueue("Academics");

        agentMgr.addAgent(new Agent("Mughees", "IT"));
        agentMgr.addAgent(new Agent("Anees", "Admin"));
        agentMgr.addAgent(new Agent("Rafay", "Accounts"));
        agentMgr.addAgent(new Agent("Hamza", "IT"));
        agentMgr.addAgent(new Agent("Ali", "Academics"));

        userBST.insert(new User("Tommy", "IT", 1001));
        userBST.insert(new User("Arif", "Admin", 1002));
        userBST.insert(new User("Umer", "Accounts", 1003));
        userBST.insert(new User("Abdullah", "Academics", 1004));

        addTicket(1001, 8, "IT", "Network connectivity issue");
        addTicket(1002, 5, "Admin", "Document request");
        addTicket(1003, 9, "Accounts", "Salary query");
        addTicket(1004, 3, "Academics", "Course material missing");
    }

    ~TicketSystem() {
        for (int i = 0; i < 4; i++) delete queues[i];
    }

    void run() {
        cout << "\n=== TICKET MANAGEMENT SYSTEM ===" << endl;
        cout << "BST | Heap | Linked List | Stack | QuickSort | MergeSort | BinarySearch" << endl;
        cout << string(50, '=') << endl;

        while (true) {
            displayMenu("MAIN MENU", { "Admin Mode", "User Mode" });

            int choice;
            if (!(cin >> choice)) { clearInput(); continue; }
            clearInput();

            if (choice == 0) break;
            else if (choice == 1) adminMenu();
            else if (choice == 2) userMenu();
            else cout << "Invalid choice." << endl;
        }

        cout << "\nGoodbye!" << endl;
    }
};

int main() {
    TicketSystem system;
    system.run();
    return 0;
}
