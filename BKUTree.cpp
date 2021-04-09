#include <iostream>
#include <vector>
#include <queue>

using namespace std;

template <class K, class V>
class BKUTree {
public:
    class AVLTree;
    class SplayTree;

    class Entry {
    public:
        K key;
        V value;
        friend class SplayTree;
        friend class AVLTree;
        friend class BKUTree;
        friend class Node;
        Entry(K key, V value) : key(key), value(value) {}
    };

public:
    AVLTree* avl;
    SplayTree* splay;
    queue<K> keys;
    int maxNumOfKeys;
    friend class Node;

public:
    BKUTree(int maxNumOfKeys = 5)
    {
        this->maxNumOfKeys = maxNumOfKeys;
        this->splay = new SplayTree();
        this->avl = new AVLTree();
    }
    ~BKUTree() { this->clear(); }

    void add(K key, V value)
    {
        if (this->avl->found(key))
        {
            throw "Duplicate key";
        }
        Entry* entry = new Entry(key, value);
        this->avl->add(entry);
        this->splay->add(entry);
        this->splay->head->corr = this->avl->recentNode;
        this->avl->recentNode->corr = this->splay->head;

        int size = keys.size();
        if (size < this->maxNumOfKeys)
            keys.push(key);
        else
        {
            keys.pop();
            keys.push(key);
        }

    }
    void remove(K key)
    {
        if (!this->avl->found(key))
        {
            throw "Not found";
        }
        this->splay->remove(key);
        this->avl->remove(key);
        int range;
        int size = keys.size();

        if (this->maxNumOfKeys > size) range = this->keys.size();
        else range = this->maxNumOfKeys;
        for (int i = 0; i < range; i++)
        {
            if (keys.front() == key)
            {
                keys.pop();
                break;
            }
            keys.push(keys.front());
            keys.pop();
        }
        keys.push(this->splay->head->entry->key);
    }
    V search(K key, vector<K>& traversedList)
    {
        if (key == this->splay->head->entry->key)
            return this->splay->head->entry->value;
        bool seen = false; int range;
        if (this->maxNumOfKeys > this->keys.size()) range = this->keys.size();
        else range = this->maxNumOfKeys;
        for (int i = 0; i < range; i++)
        {
            if (keys.front() == key)
            {
                keys.pop(); seen = true;
                break;
            }
            keys.push(keys.front());
            keys.pop();
        }
        if (seen)
            return this->splay->search(key);
        else
        {
            typename AVLTree::Node* ret = this->avl->SearchBKU(key, this->splay->head->corr, traversedList);
            if (ret)
            {
                if (key == ret->entry->key)
                    return ret->entry->value;
            }
            traversedList.clear();
            typename AVLTree::Node* tmp = this->avl->searchBKU(key, this->avl->head->left, this->splay->head->corr, traversedList);
            if (key == tmp->entry->key)
            {
                if (this->keys.size() < this->maxNumOfKeys)
                    this->keys.push(key);
                else
                {
                    this->keys.pop();
                    this->keys.push(key);
                }
                return this->splay->search(tmp->entry->key);
            }
            else
            {
                throw "Not found";
            }
        }
    }

    void traverseNLROnAVL(void (*func)(K key, V value))
    {
        this->avl->traverseNLR(func);
    }
    void traverseNLROnSplay(void (*func)(K key, V value))
    {
        this->splay->traverseNLR(func);
    }

    void clear()
    {
        this->avl->clear();
        this->splay->clear();
        while (!this->keys.empty()) this->keys.pop();
        delete this->avl;
        delete this->splay;
        this->avl = nullptr;
        this->splay = nullptr;
    }

    class SplayTree {
    public:
        class Node {
            Entry* entry;
            Node* left;
            Node* right;
            friend class SplayTree;
            friend class BKUTree;
            typename AVLTree::Node* corr;

            Node(Entry* entry = NULL, Node* left = NULL, Node* right = NULL) {
                this->entry = entry;
                this->left = left;
                this->right = right;
                this->corr = NULL;
            }
        };

    public:
        Node* head;
        friend class AVLTree;
        friend class BKUTree;
        SplayTree() : head(NULL)
        {
            this->head = nullptr;
        }
        ~SplayTree() { this->clear(); };

        void add(K key, V value)
        {
            if (found(key))
            {
                throw "Duplicate key";
            }
            Entry* entry = new Entry(key, value);
            add(entry);
        }
        void add(Entry* entry)
        {
            if (found(entry->key))
            {
                throw "Duplicate key";
            }
            Node* temp = this->head; int save = 0;
            Add(temp, temp, temp, temp, 0, save, entry);
        }
        void Add(Node*& grandparent, Node*& parent, Node*& root, Node*& child, int cost, int& save, Entry* entry)
        {
            if (this->head == nullptr)
            {
                this->head = new Node(entry, NULL, NULL);
                return;
            }
            if (!child)
            {
                child = new Node(entry, NULL, NULL);
            }
            if (entry->key < child->entry->key)
            {
                save += 1;
                Add(parent, root, child, child->left, cost + 1, save, entry);
            }
            else if (entry->key > child->entry->key)
            {
                save += 1;
                Add(parent, root, child, child->right, cost + 1, save, entry);
            }
            if (cost == save)
            {
                Splay(grandparent, parent, root, child, entry->key);
                save -= 2;
            }
        }
        void Splay(Node* grandparent, Node* parent, Node* root, Node* child, K key)
        {
            if (this->head->left != nullptr)
            {
                if (this->head->left->entry->key == key) {
                    Zig_rotation(this->head, this->head, this->head->left);
                    return;
                }
            }
            if (this->head->right != nullptr)
            {
                if (this->head->right->entry->key == key) {
                    Zag_rotation(this->head, this->head, this->head->right);
                    return;
                }
            }
            ComplexSplay(grandparent, parent, root, child);
        }
        void ComplexSplay(Node* grandparent, Node* parent, Node* root, Node* child)
        {
            if (parent->left == root && root->left == child)
                Zig_Zig(grandparent, parent, root, child);
            else if (parent->right == root && root->right == child)
                Zag_Zag(grandparent, parent, root, child);
            else if (parent->right == root && root->left == child)
                Zig_Zag(grandparent, parent, root, child);
            else if (parent->left == root && root->right == child)
                Zag_Zig(grandparent, parent, root, child);
        }
        void Zig_rotation(Node* parent, Node* root, Node* child, bool straight = false)
        {
            Node* temp = child->right;
            child->right = root;
            root->left = temp;
            if (root == this->head) this->head = child;
            else
            {
                if (straight) parent->left = child;
                else parent->right = child;
            }
        }
        void Zag_rotation(Node* parent, Node* root, Node* child, bool straight = false)
        {
            Node* temp = child->left;
            child->left = root;
            root->right = temp;
            if (root == this->head) this->head = child;
            else
            {
                if (straight) parent->right = child;
                else parent->left = child;
            }
        }
        bool checkStraight(Node* parent, Node* root, Node* child)
        {
            if (parent->left == root && root->left == child) return true;
            if (parent->right == root && root->right == child) return true;
            return false;
        }
        void Zig_Zag(Node* grandparent, Node* parent, Node* root, Node* child)
        {
            bool check = checkStraight(parent, root, child);
            Zig_rotation(parent, root, child, check);
            check = checkStraight(grandparent, parent, child);
            Zag_rotation(grandparent, parent, child, check);
        }
        void Zag_Zig(Node* grandparent, Node* parent, Node* root, Node* child)
        {
            bool check = checkStraight(parent, root, child);
            Zag_rotation(parent, root, child);
            check = checkStraight(grandparent, parent, child);
            Zig_rotation(grandparent, parent, child, check);
        }
        void Zig_Zig(Node* grandparent, Node* parent, Node* root, Node* child)
        {
            bool check = checkStraight(grandparent, parent, root);
            Zig_rotation(grandparent, parent, root, check);
            check = checkStraight(grandparent, root, child);
            Zig_rotation(grandparent, root, child, check);
        }
        void Zag_Zag(Node* grandparent, Node* parent, Node* root, Node* child)
        {
            bool check = checkStraight(grandparent, parent, root);
            Zag_rotation(grandparent, parent, root, check);
            check = checkStraight(grandparent, root, child);
            Zag_rotation(grandparent, root, child, check);
        }
        Node* maxLeft(Node* parent)
        {
            if (!parent) return nullptr;
            if (parent->left == nullptr) return nullptr;
            if (parent->left->right == nullptr) return parent->left;
            parent = parent->left;
            while (parent->right->right != nullptr)
                parent = parent->right;
            return parent;
        }
        Node* minRight(Node* parent)
        {
            if (!parent) return nullptr;
            if (parent->right == nullptr) return nullptr;
            if (parent->right->left == nullptr) return parent->right;
            parent = parent->right;
            while (parent->left->left != nullptr)
                parent = parent->left;
            return parent;
        }
        bool found(K key)
        {
            Node* temp = this->head;
            Node* check = Search(temp, temp, temp, temp, key);
            if (!check) return false;
            if (key == check->entry->key) return true;
            return false;
        }
        void remove(K key)
        {
            if (!found(key))
            {
                throw "Not found";
            }
            Node* root = this->head; int save = 0;
            Splaying(root, root, root, root, 0, save, key);
            Node* ptr = this->head; Node* temp;
            if (ptr->left == nullptr && ptr->right == nullptr)
            {
                delete ptr; this->head = nullptr;
            }
            else if (maxLeft(ptr))
            {
                temp = maxLeft(ptr);
                if (temp->right == nullptr)
                {
                    temp->right = this->head->right;
                    Node* t = this->head;
                    this->head = temp;
                    delete t; t = nullptr;
                }
                else
                {
                    save = 0;
                    K t = temp->right->entry->key;
                    Node* p = this->head->left;
                    Node* m = this->head->right;
                    delete this->head; this->head = nullptr;
                    this->head = p;
                    Splaying(p, p, p, p, 0, save, t);
                    this->head->right = m;
                }
            }
            else if (minRight(ptr))
            {
                temp = minRight(ptr);
                if (temp->left == nullptr)
                {
                    temp->left = this->head->left;
                    Node* t = this->head;
                    this->head = temp;
                    delete t; t = nullptr;
                }
                else
                {
                    save = 0;
                    K t = temp->left->entry->key;
                    Node* p = this->head->left;
                    Node* m = this->head->right;
                    delete this->head; this->head = nullptr;
                    this->head = p;
                    Splaying(p, p, p, p, 0, save, t);
                    this->head->left = p;
                }
            }
        }
        void Splaying(Node*& grandparent, Node*& parent, Node*& root, Node*& child, int cost, int& save, K key)
        {
            if (!child)
                return;
            if (key < child->entry->key)
            {
                save += 1;
                Splaying(parent, root, child, child->left, cost + 1, save, key);
            }
            else if (key > child->entry->key)
            {
                save += 1;
                Splaying(parent, root, child, child->right, cost + 1, save, key);
            }
            if (cost == save)
            {
                Splay(grandparent, parent, root, child, key);
                save -= 2;
            }
        }
        bool parentLeft(Node* root, Node* parent)
        {
            if (parent->left == root) return true;
            return false;
        }
        V search(K key)
        {
            if (!found(key))
            {
                throw "Not found";
            }
            Node* temp = this->head;
            return Search(temp, temp, temp, temp, key)->entry->value;
        }
        Node* Search(Node*& grandparent, Node*& parent, Node*& root, Node*& child, K key)
        {
            if (!child) return nullptr;
            if (child->entry->key == key)
            {
                Node* temp = child;
                Splay(grandparent, parent, root, child, key);
                return temp;
            }
            if (key < child->entry->key)
            {
                return Search(parent, root, child, child->left, key);
            }
            else if (key > child->entry->key)
            {
                return Search(parent, root, child, child->right, key);
            }

            return nullptr;
        }
        void traverseNLR(void (*func)(K key, V value))
        {
            Node* ptr = this->head;
            traverseNLR(ptr, func);
        }
        void traverseNLR(Node* ptr, void(*func)(K key, V value))
        {
            if (!ptr) return;
            else
            {
                func(ptr->entry->key, ptr->entry->value);
                traverseNLR(ptr->left, func);
                traverseNLR(ptr->right, func);
            }
        }
        void clear()
        {
            Node* temp = this->head;
            while (temp != nullptr) {
                remove(temp->entry->key);
                temp = this->head;
            }
        }
    };

    class AVLTree {
    public:
        class Node {
            Entry* entry;
            Node* left;
            Node* right;
            int balance;
            int hL;
            int hR;
            friend class AVLTree;
            friend class BKUTree;
            typename SplayTree::Node* corr;

            Node(Entry* entry = NULL, Node* left = NULL, Node* right = NULL) {
                this->entry = entry;
                this->left = left;
                this->right = right;
                this->balance = 0;
                this->hL = 0;
                this->hR = 0;
                this->corr = NULL;
            }
        };

    public:
        Node* head;
        Node* recentNode;
        friend class SplayTree;
        friend class BKUTree;
        AVLTree() : head(NULL)
        {
            this->head = new Node();
            this->recentNode = new Node();
        }
        ~AVLTree() { this->clear(); };

        void add(K key, V value)
        {
            if (found(key))
            {
                throw "Duplicate key";
            }
            Entry* entry = new Entry(key, value);
            add(entry);
        }
        void add(Entry* entry)
        {
            if (found(entry->key))
            {
                throw "Duplicate key";
            }
            bool h = false;
            Node* temp = this->head;
            Add(temp->left, entry, temp, h);
        }
        void Add(Node*& root, Entry* entry, Node*& parent, bool& h)
        {
            if (this->head->left == nullptr)
            {
                this->head->left = new Node(entry, NULL, NULL);
                this->recentNode = this->head->left;
                return;
            }
            if (!root)
            {
                root = new Node(entry, NULL, NULL);
                this->recentNode = root;
            }
            if (entry->key < root->entry->key)
            {
                root->hL += 1;
                Add(root->left, entry, root, h);
            }
            else if (entry->key > root->entry->key)
            {
                root->hR += 1;
                Add(root->right, entry, root, h);
            }
            edit(parent, root, h);
        }
        void edit(Node* parent, Node* root, bool& h)
        {
            if (h)
                calc_height(root);
            if (root->hL - root->hR > 1)
            {
                Node* child = root->left;
                if (child->hR <= child->hL)
                    LL_case(parent, root, child);
                else
                    LR_case(parent, root, child);
                h = true;
            }
            else if (root->hR - root->hL > 1)
            {
                Node* child = root->right;
                if (child->hL > child->hR)
                    RL_case(parent, root, child);
                else
                    RR_case(parent, root, child);
                h = true;
            }
        }
        void calc_height(Node*& temp)
        {
            if (!temp) return;
            int count = -1;

            Node* a = temp->left;
            if (a == nullptr) temp->hL = 0;
            else
            {
                if (a->hL > a->hR)
                    count = a->hL;
                else
                    count = a->hR;
                temp->hL = count + 1;
            }
            Node* b = temp->right;
            if (b == nullptr) temp->hR = 0;
            else
            {
                if (b->hL > b->hR)
                    count = b->hL;
                else
                    count = b->hR;
                temp->hR = count + 1;
            }
        }
        void LL_case(Node* parent, Node* root, Node* child)
        {
            if (parent->left == root)
                parent->left = child;
            else
                parent->right = child;
            root->left = child->right;
            child->right = root;
            calc_height(root);
            calc_height(child);
        }
        void LR_case(Node* parent, Node* root, Node* child)
        {
            Node* s_child = child->right;
            if (parent->left == root)
                parent->left = s_child;
            else
                parent->right = s_child;

            child->right = s_child->left;
            root->left = s_child->right;
            s_child->left = child;
            s_child->right = root;
            calc_height(child);
            calc_height(root);
            calc_height(s_child);
        }
        void RR_case(Node* parent, Node* root, Node* child)
        {
            if (parent->left == root)
            {
                parent->left = child;
            }
            else parent->right = child;
            root->right = child->left;
            child->left = root;
            calc_height(root);
            calc_height(child);
        }
        void RL_case(Node* parent, Node* root, Node* child)
        {
            Node* s_child = child->left;
            if (parent->left == root)
            {
                parent->left = s_child;
            }
            else parent->right = s_child;
            child->left = s_child->right;
            root->right = s_child->left;
            s_child->right = child;
            s_child->left = root;
            calc_height(child);
            calc_height(root);
            calc_height(s_child);
        }
        Node* maxLeft(Node* parent)
        {
            if (!parent) return nullptr;
            if (parent->left->right == nullptr) return parent->left;
            parent = parent->left;
            while (parent->right->right != nullptr)
                parent = parent->right;
            return parent;
        }
        Node* minRight(Node* parent)
        {
            if (!parent) return nullptr;
            if (parent->right->left == nullptr) return parent->right;
            parent = parent->right;
            while (parent->left->left != nullptr)
                parent = parent->left;
            return parent;
        }
        void remove(K key)
        {
            if (!found(key))
            {
                throw "Not found";
            }
            bool h = false;
            Node* temp = this->head;
            Remove(temp->left, key, temp, h);
        }
        void Remove(Node* root, K key, Node* parent, bool& h)
        {
            if (this->head->left == nullptr) return;
            if (!root) return;
            if (key < root->entry->key)
            {
                Remove(root->left, key, root, h);
            }
            else if (key > root->entry->key)
            {
                Remove(root->right, key, root, h);
            }
            else
            {
                Node* temp = nullptr;
                if (root->hL == 0 && root->hR == 0)
                {
                    if (parent->left == root)
                        parent->left = nullptr;
                    else if (parent->right == root)
                        parent->right = nullptr;
                    temp = parent;
                }
                else if (root->hL != 0)
                {
                    temp = maxLeft(root);
                    if (temp->right == nullptr)
                    {
                        temp->right = root->right;
                        if (parentLeft(root, parent))
                            parent->left = temp;
                        else parent->right = temp;
                        calc_height(temp);
                    }
                    else
                    {
                        temp->hR = temp->right->hL;
                        Node* t = temp->right->left;
                        temp->right->left = root->left;
                        temp->right->right = root->right;
                        calc_height(temp->right);
                        if (parentLeft(root, parent))
                            parent->left = temp->right;
                        else parent->right = temp->right;
                        temp->right = t;
                    }
                }
                else
                {
                    temp = minRight(root);
                    if (temp->left == nullptr)
                    {
                        temp->left = root->left;
                        if (parentLeft(root, parent))
                            parent->left = temp;
                        else parent->right = temp;
                        calc_height(temp);
                    }
                    else
                    {
                        temp->hL = temp->left->hR;
                        Node* t = temp->left->right;
                        temp->left->right = root->right;
                        temp->left->left = root->left;
                        calc_height(temp->left);
                        if (parentLeft(root, parent))
                            parent->left = temp->left;
                        else parent->right = temp->left;
                        temp->left = t;
                    }
                }
                delete root->entry;
                delete root;
                Edit(this->head->left, temp, this->head, h);
            }
        }
        bool parentLeft(Node* root, Node* parent)
        {
            if (parent->left == root) return true;
            return false;
        }
        void Edit(Node* root, Node* temp, Node* parent, bool& h)
        {
            if (!root) return;
            if (temp->entry->key < root->entry->key)
            {
                Edit(root->left, temp, root, h);
            }
            else if (temp->entry->key > root->entry->key)
            {
                Edit(root->right, temp, root, h);
            }
            if (h)
                calc_height(root);
            if (root->hL - root->hR > 1)
            {
                Node* child = root->left;
                if (child->hR <= child->hL)
                    LL_case(parent, root, child);
                else
                    LR_case(parent, root, child);
                h = true;
            }
            else if (root->hR - root->hL > 1)
            {
                Node* child = root->right;
                if (child->hL > child->hR)
                    RL_case(parent, root, child);
                else
                    RR_case(parent, root, child);
                h = true;
            }
        }
        V search(K key)
        {
            if (!found(key))
            {
                throw "Not found";
            }
            Node* temp = this->head->left;
            return Search(key, temp)->entry->value;
        }
        bool found(K key)
        {
            Node* temp = this->head->left;
            Node* check = Search(key, temp);
            if (!check) return false;
            if (key == check->entry->key) return true;
            return false;
        }
        Node* SearchBKU(K key, Node* root, vector<K>& traversedList)
        {
            if (!root) return nullptr;
            if (key == root->entry->key) return root;
            if (key < root->entry->key)
            {
                traversedList.push_back(root->entry->key);
                return SearchBKU(key, root->left, traversedList);
            }
            else if (key > root->entry->key)
            {
                traversedList.push_back(root->entry->key);
                return SearchBKU(key, root->right, traversedList);
            }
        }
        Node* searchBKU(K key, Node* root, Node* brk, vector<K>& traversedList)
        {
            if (root == brk) return nullptr;
            if (!root) return nullptr;
            if (key == root->entry->key) return root;
            if (key < root->entry->key)
            {
                traversedList.push_back(root->entry->key);
                return Search(key, root->left);
            }
            else if (key > root->entry->key)
            {
                traversedList.push_back(root->entry->key);
                return Search(key, root->right);
            }
        }
        Node* Search(K key, Node* root)
        {
            if (!root) return nullptr;
            if (key == root->entry->key) return root;
            if (key < root->entry->key)
            {
                return Search(key, root->left);
            }
            else if (key > root->entry->key)
            {
                return Search(key, root->right);
            }

            return nullptr;
        }
        void traverseNLR(void (*func)(K key, V value))
        {
            Node* ptr = this->head->left;
            traverseNLR(ptr, func);
        }
        void traverseNLR(Node*& ptr, void(*func)(K key, V value))
        {
            if (!ptr) return;
            else
            {
                func(ptr->entry->key, ptr->entry->value);
                traverseNLR(ptr->left, func);
                traverseNLR(ptr->right, func);
            }
        }
        void clear()
        {
            Node* temp = this->head->left;
            while (temp != nullptr) {
                remove(temp->entry->key);
                temp = this->head->left;
            }
            delete this->recentNode;
            delete this->head;
            this->head = nullptr;
            this->recentNode = nullptr;
        }
    };
};
void printKey(int key, int value) {
     cout << key << endl;
}

void test_1()
{
    BKUTree<int, int>* tree = new BKUTree<int, int>();
    int keys[] = {1, 3, 5, 7, 9, 2, 4};
    for (int i = 0; i < 7; i++) tree->add(keys[i], keys[i]);
    tree->traverseNLROnAVL(printKey);
}
void test_2()
{
    BKUTree<int, int>* tree = new BKUTree<int, int>();
    int keys[] = {1, 3, 5, 7, 9, 2, 4};
    for (int i = 0; i < 7; i++) tree->add(keys[i], keys[i]);
    tree->traverseNLROnSplay(printKey);
}
int main()
{
    test_1();
    test_2();
    return 0;
}