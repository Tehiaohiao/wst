#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <climits> // for INT_MAX
#include <queue>
#include <string>
#include <utility>   // for std::pair
#include "node.h"

const int DEFAULT_MIN_DEGREE = 2;
const int DEFAULT_MAX_HEIGHT = 10;
const int MAX_NUM_FREE_NODES = 350000;
bool DEBUG = false;

template <class T>
class BTree {
public:
    BTree() : min_degree(DEFAULT_MIN_DEGREE), height(1), max_height(DEFAULT_MAX_HEIGHT) {
        create_tree();
    }
    BTree(int min_deg, int max_hght = DEFAULT_MAX_HEIGHT) : min_degree(min_deg), height(1), max_height(max_hght) {
        create_tree();
    }
    ~BTree();
    std::pair<Node<T>&, int> search(T val);
    int insert(T val);
    void insert_lru(T val);
    bool remove(T val); // returns whether val was found in the tree
    T remove_lru(); // remove the element at the back of the linked list (least recently accessed element)
    T remove_mru(); // remove element at the beginning of the linked list (most recently accessed element)
    int get_height();
    int get_max_height();
    bool is_empty();
    int size();
    std::string to_string();
    std::string print_ordered_mru();
    std::string print_ordered_tail();
private:
    Node<T> *root;
    int min_degree; // each node contains (m-1) to (2*m-1) keys
    int height;
    int max_height;
    int size_;
    Element<T> *head;
    std::vector<Node<T> *> free_nodes;
    void create_tree();
    void destroy_tree(Node<T> *node);
    std::pair<Node<T>&, int> search_node(Node<T> &node, T val);
    void split_child(Node<T> *node, int index);
    int insert_nonfull(Node<T> *node, T element);
    bool remove_helper(Node<T> *node, T val, bool modify_linked_list, Element<T> *new_pos); // returns whether val was found in the tree
                                                                                            // new_pos is the new position of the removed element
    void merge_children(Node<T> *node, int index);
    void steal_from_left_neighbor(Node<T> *node, int index);
    void steal_from_right_neighbor(Node<T> *node, int index);
    Element<T>* find_max_key(Node<T> *node); // implemented recursively. can be implemented iteratively
    Element<T>* find_min_key(Node<T> *node); // implemented recursively. can be implemented iteratively

};

template <class T>
void BTree<T>::create_tree() {
    root = new Node<T>(min_degree);
    head = new Element<T>(); // sentinel
    head->next = head;
    head->prev = head;

    int num_free_nodes = 1; // root plus the one possible extra node when tree exceeds max_height
    int curr_lvl_nodes = 1;
    for (int i=1; i<max_height; ++i) {
        curr_lvl_nodes = curr_lvl_nodes * (min_degree*2);
        num_free_nodes = num_free_nodes + curr_lvl_nodes;
        if (num_free_nodes > MAX_NUM_FREE_NODES) {
            num_free_nodes = MAX_NUM_FREE_NODES;
        }
    }

    for (int j=0; j<num_free_nodes; ++j) {
        free_nodes.push_back(new Node<T>(min_degree));
    }
}

template <class T>
BTree<T>::~BTree() {
    destroy_tree(root);
    delete head;
    while (!free_nodes.empty()) {
        Node<T> *node = free_nodes.back();
        free_nodes.pop_back();
        delete node;
    }
}

template <class T>
void BTree<T>::destroy_tree(Node<T> *node) {

    // recursively destroy all subtrees
    if (!node->is_leaf) {
        for (int i = 0; i <= node->num_keys; ++i) {
            destroy_tree(node->children[i]);
        }
    }
}

template <class T>
std::pair<Node<T>&, int> BTree<T>::search(T val) {
    // call helper function and start search at the root
    return search_node(*root, val);
}

template <class T>
std::pair<Node<T>&, int> BTree<T>::search_node(Node<T> &node, T val) {

    // find the index i of val in node
    // val is at index i, or in the i^th child of node
    int i = 0;
    while (i<node.num_keys && val>node.keys[i].key) {
        i++;
    }

    if (i < node.num_keys && val == node.keys[i].key) { // val is found at index i

        // -------- see if the following can be done without typedef (and in else if)
        typedef std::pair<Node<T>&, int> node_index;
        return node_index(node, i);
    }
    else if (node.is_leaf) { // reached a leaf and val not found
        typedef std::pair<Node<T>&, int> node_index;

        // returned index is -1 when element is not found
        return node_index(node, -1);
    }
    else {
        // recursively search for val in node's i^th child
        return search_node(*node.children[i], val);
    }
}

template <class T>
int BTree<T>::insert(T val) {

    // levels of the tree traversed to insert val into the tree
    int levels_traversed = 0;

    if (root->num_keys == min_degree * 2 - 1) {

        // root node is full, split into two nodes and move middle
        // 	element up to become the new root
//        Node<T> *new_root = new Node<T>(min_degree);
        Node<T> *new_root;
        if (free_nodes.empty()) {
            new_root = new Node<T>(min_degree);
        }
        else {
            new_root = free_nodes.back();
            free_nodes.pop_back();
        }

        Node<T> *child_ptr = root;

        new_root->is_leaf = false;
        new_root->children[0] = child_ptr;

        root = new_root;
        height++;
        split_child(root, 0);
        levels_traversed = insert_nonfull(root, val);
    }
    else {
        // root not full, call helper function insert_nonfull on root
        levels_traversed = insert_nonfull(root, val);
    }

    size_++;

    return levels_traversed;
}

template <class T>
void BTree<T>::insert_lru(T val) {
    // insert and add element to the back of the linked list
    //  used for element shifting in working set tree

    insert(val);

    // move element containing val from the beginning of the linked list
    //  to the back of the linked list
    Element<T> *element = head->next;
    head->next = head->next->next;
    head->next->prev = head;
    head->prev->next = element;
    element->prev = head->prev;
    head->prev = element;
    element->next = head;

}

template <class T>
void BTree<T>::split_child(Node<T> *node, int index) {

    Node<T> *child1 = node->children[index]; // child to be split
//    Node<T> *child2 = new Node<T>(min_degree); // child splitting into
    Node<T> *child2;
    if (free_nodes.empty()) {
        child2 = new Node<T>(min_degree);
    }
    else {
        child2 = free_nodes.back();
        free_nodes.pop_back();
    }

    child2->is_leaf = child1->is_leaf;
    child2->num_keys = min_degree - 1;

    // move the right half of child1's keys to child2
    for (int i = child2->num_keys - 1; i >= 0; --i) {
        child2->keys[i] = child1->keys[min_degree + i];
        child2->keys[i].next->prev = &(child2->keys[i]);
        child2->keys[i].prev->next = &(child2->keys[i]);
    }

    // move the right half of children from child1 to child2
    if (!child1->is_leaf) {
        for (int i = 0; i < min_degree; ++i) {
            child2->children[i] = child1->children[min_degree + i];
        }
    }

    // update child1 variable num_keys
    child1->num_keys = min_degree - 1;

    // insert child2 into node's vector of children
    for (int i = node->num_keys; i >= index + 1; i--) {
        node->children[i + 1] = node->children[i];
    }
    node->children[index + 1] = child2;

    // shift node's appropriate keys to make room for child1's middle element
    //  to be inserted
    for (int i = node->num_keys - 1; i >= index; i--) {
        node->keys[i + 1] = node->keys[i];
        node->keys[i + 1].prev->next = &(node->keys[i + 1]);
        node->keys[i + 1].next->prev = &(node->keys[i + 1]);
    }

    // insert child1's middle element into node
    node->keys[index] = child1->keys[min_degree - 1];
    node->keys[index].prev->next = &(node->keys[index]);
    node->keys[index].next->prev = &(node->keys[index]);

    node->num_keys++;

}

template <class T>
int BTree<T>::insert_nonfull(Node<T> *node, T element) {

    // find the position i in node to insert element
    int i = node->num_keys - 1;
    while (i >= 0 && element < node->keys[i].key) {
        i--;
    }
    i++;

    if (node->is_leaf) {

        // insert element into position i of the leaf

        // shift keys to the right of i over to make room for element
        for (int j = node->num_keys; j > i; j--) {
            node->keys[j] = node->keys[j - 1];
            node->keys[j].next->prev = &(node->keys[0]) + j;
            node->keys[j].prev->next = &(node->keys[0]) + j;

        }

        // insert element into node's vector of keys at position i
        node->keys[i] = Element<T>(element, head, head->next);
        head->next->prev = &(node->keys[i]);
        head->next = &(node->keys[i]);

        node->num_keys++;

        return 1; // return levels traversed from current node to inserted node
    }
    else { // node is not leaf

        Node<T> *child = node->children[i];

        // if the child to recurse on is full, split the child
        if (child->num_keys == min_degree * 2 - 1) {

            split_child(node, i);

            if (element > node->keys[i].key) {
                i++;
            }
        }

        // recursively insert element on the i^th child
        return 1 + insert_nonfull(node->children[i], element);
    }
}

template <class T>
bool BTree<T>::remove(T value) {
    return remove_helper(root, value, true, nullptr);
    size_--;
}

// Inputs:
//    node: the root of the subtree from which val is deleted
//    val: key to be deleted
//    modify_linked_list: insert to the front of the linked list when true, maintain current
//							position in the linked list but move element to new_pos when false
// Return value: true if value found and removed, false otherwise
template <class T>
bool BTree<T>::remove_helper(Node<T> *node, T val, bool modify_linked_list, Element<T> *new_pos) { // modify only leaves? for now

    // find the index i of val in node
    int i = 0;
    while (i < node->num_keys && val > node->keys[i].key) {
        i++;
    }

    if (node->is_leaf) { // at leaf of tree
        if (i < node->num_keys && val == node->keys[i].key) { // val found in leaf

            Element<T> *k = &(node->keys[i]);

            if (modify_linked_list) {

                // delete val from linked list by updating next and previous
                //  elements' pointers
                k->next->prev = k->prev;
                k->prev->next = k->next;

            }
            else {

                // do not delete val from linked list. Instead update the pointer
                //  to new_pos
                new_pos->prev = k->prev;
                new_pos->next = k->next;
                k->prev->next = new_pos;
                k->next->prev = new_pos;

            }

            // delete the i^th element from keys by moving all subsequent
            //   elements forward by one position
            for (int j = i; j < node->num_keys - 1; j++) {
                node->keys[j] = node->keys[j + 1];
                node->keys[j].next->prev = &(node->keys[j]);
                node->keys[j].prev->next = &(node->keys[j]);
            }
            node->num_keys--;

            return true;
        }
        else { // val not found in leaf
            return false;
        }
    }
    else if (i < node->num_keys && val == node->keys[i].key) { // val found in internal node

        if (node->children[i]->num_keys >= min_degree) { // 2a
            // 2a: the i^th child has more than the minimum number of keys

            // delete val from linked list
            node->keys[i].next->prev = node->keys[i].prev;
            node->keys[i].prev->next = node->keys[i].next;

            // find the largest element smaller than val
            Element<T> *predecessor = find_max_key(node->children[i]);
            Element<T> pred_copy = Element<T>();
            pred_copy = *predecessor;

            // remove the predecessor from its original position and
            //  replace val with predecessor at its current position
            remove_helper(node, predecessor->key, false, &(node->keys[i]));
            node->keys[i].key = pred_copy.key;

            size_++;

        }
        else if (node->children[i + 1]->num_keys >= min_degree) { //2b
            // 2b: the (i+1)^th child has more than the minimum number of keys

            // delete val from linked list
            node->keys[i].next->prev = node->keys[i].prev;
            node->keys[i].prev->next = node->keys[i].next;

            // find the smallest element larger than val
            Element<T> *successor = find_min_key(node->children[i + 1]);
            Element<T> succ_copy = Element<T>();
            succ_copy = *successor;

            // remove the successor from its original position and
            //  replace val with successor at val's current position
            remove_helper(node, successor->key, false, &(node->keys[i]));
            node->keys[i].key = succ_copy.key;

            size_++;

        }
        else { // 2c
            // 2c: both children have exactly the minimum number of keys

            merge_children(node, i);

            // print current tree if in debug mode
            if (DEBUG) {
                std::cout << to_string() << std::endl;
            }

            // recursively delete val on the left child
            remove_helper(node->children[i], val, modify_linked_list, new_pos);

        }

        return true;

    }
    else { // val not found in internal node

        // if child node to recurse on has the minimum allowed number of keys
        //  adjust tree such that the node has at least min_degree number of elements
        if (node->children[i]->num_keys == min_degree - 1) {

            // if one of its siblings has at least t keys, steal from sibling
            if (i > 0 && node->children[i - 1]->num_keys >= min_degree) { //3a
                // 3a: its left neighbor has more than the minimum number of elements

                steal_from_left_neighbor(node, i);

            }
            else if (i < node->num_keys && node->children[i + 1]->num_keys >= min_degree) { //3b
                // 3b: its right neighbor has more than the minimum number of elements

                steal_from_right_neighbor(node, i);

            }
            else { // 3c
                // 3c: both siblings have t-1 keys, merge with a sibling
                if (i < node->num_keys) { // merge with right sibling

                    merge_children(node, i);

                }
                else { // i = node.num_keys, merge with left sibling

                    merge_children(node, i-1);
                    i--;

                }
            }

        }

        if (DEBUG) {
            std::cout << to_string() << std::endl;
        }

        // recurse
        return remove_helper(node->children[i], val, modify_linked_list, new_pos);
    }
}

// find the largest element in the subtree rooted at node
template <class T>
Element<T>* BTree<T>::find_max_key(Node<T> *node) {
    if (node->is_leaf) {
        return &node->keys[node->num_keys - 1];
    }
    else {
        return find_max_key(node->children[node->num_keys]);
    }
}

// find the smallest element in the subtree rooted at node
template <class T>
Element<T>* BTree<T>::find_min_key(Node<T> *node) {
    if (node->is_leaf) {
        return &node->keys[0];
    }
    else {
        return find_min_key(node->children[0]);
    }
}

// merge node key at index and its right child at index (i+1)
//  to the left child at index i
template <class T>
void BTree<T>::merge_children(Node<T> *node, int index) {
    Node<T> *left_child = node->children[index];
    Node<T> *right_child = node->children[index + 1];

    // merge val and right child to left child

    // move val to left child
    left_child->keys[min_degree - 1] = node->keys[index];
    left_child->keys[min_degree - 1].prev->next = &(left_child->keys[min_degree - 1]);
    left_child->keys[min_degree - 1].next->prev = &(left_child->keys[min_degree - 1]);

    // move all keys from right_child to left child and update linked list pointers
    for (int j = 0; j < right_child->num_keys; ++j) {

        int index = j + min_degree;
        left_child->keys[index] = right_child->keys[j];
        left_child->keys[index].next->prev = &(left_child->keys[index]);
        left_child->keys[index].prev->next = &(left_child->keys[index]);

    }

    // append all right child's children to left child's children vector
    if (!right_child->is_leaf) {
        for (int j = 0; j <= right_child->num_keys; ++j) {
            left_child->children[min_degree + j] = right_child->children[j];
        }
    }

    // remove val from node by shifting all subsequent keys one position to the left
    for (int j = index + 1; j < node->num_keys; j++) {
        node->keys[j - 1] = node->keys[j];
        node->keys[j - 1].next->prev = &(node->keys[j - 1]);
        node->keys[j - 1].prev->next = &(node->keys[j - 1]);
    }

    // remove right child by shifting all subsequent children one position to the left
    //  in the children vector
    for (int j = index + 1; j < node->num_keys; ++j) {
        node->children[j] = node->children[j + 1];
    }

    left_child->num_keys = min_degree * 2 - 1;
    node->num_keys--;

    // free memory used by right_child
//    delete right_child;
    right_child->num_keys = 0;
    free_nodes.push_back(right_child);

    // decrease height variable if height of b-tree has decremented
    if (node == root && node->num_keys == 0) {
        root = left_child;
        height--;
    }
}

// rotate the tree by stealing an element from the left neighbor and
//  moving node key at i down to the right child such that the right child
//  now has one more key
template <class T>
void BTree<T>::steal_from_left_neighbor(Node<T> *node, int index) {

    Node<T> *left_child = node->children[index - 1];
    Node<T> *child = node->children[index];

    // shift all elements in child one position to the right
    for (int j = child->num_keys; j>0; j--) {
        child->keys[j] = child->keys[j - 1];
        child->keys[j].next->prev = &(child->keys[j]);
        child->keys[j].prev->next = &(child->keys[j]);
    }

    // insert the i^th key in node to the beginning of its right child
    child->keys[0] = node->keys[index - 1];
    child->keys[0].next->prev = &(child->keys[0]);
    child->keys[0].prev->next = &(child->keys[0]);

    // set the i^th key in node to last element of left child
    node->keys[index - 1] = left_child->keys[left_child->num_keys - 1];
    node->keys[index - 1].next->prev = &(node->keys[index - 1]);
    node->keys[index - 1].prev->next = &(node->keys[index - 1]);

    // move the left node's rightmost child pointer to the beginning of the right child node's children vector
    if (!child->is_leaf) {

        for (int j = child->num_keys; j >= 0; j--) {
            child->children[j + 1] = child->children[j];
        }
        child->children[0] = left_child->children[left_child->num_keys];

    }

    left_child->num_keys--;
    child->num_keys++;

}

// rotate the tree by stealing an element from the right neighbor and
//  moving the node key at i down to the left child such that the left
//  child now has one more key
template <class T>
void BTree<T>::steal_from_right_neighbor(Node<T> *node, int index) {

    Node<T> *child = node->children[index];
    Node<T> *right_child = node->children[index + 1];

    // push node key at index i to the back of left child
    child->keys[child->num_keys] = node->keys[index];
    child->keys[child->num_keys].next->prev = &(child->keys[child->num_keys]);
    child->keys[child->num_keys].prev->next = &(child->keys[child->num_keys]);

    // move right child's first key to node
    node->keys[index] = right_child->keys[0];
    node->keys[index].next->prev = &(node->keys[index]);
    node->keys[index].prev->next = &(node->keys[index]);

    // shift all keys in right child one position to the left
    for (int j = 0; j<right_child->num_keys - 1; ++j) {
        right_child->keys[j] = right_child->keys[j + 1];
        right_child->keys[j].next->prev = &(right_child->keys[j]);
        right_child->keys[j].prev->next = &(right_child->keys[j]);
    }

    // move right child's first child pointer to be left child's last child pointer
    if (!child->is_leaf) {

        child->children[child->num_keys + 1] = right_child->children[0];

        for (int j = 0; j < right_child->num_keys; ++j) {
            right_child->children[j] = right_child->children[j + 1];
        }

    }

    child->num_keys++;
    right_child->num_keys--;
}

template <class T>
int BTree<T>::get_height() {
    return height;
}

template <class T>
int BTree<T>::get_max_height() {
    return max_height;
}

template <class T>
bool BTree<T>::is_empty() {
    return (root->num_keys == 0);
}

template <class T>
int BTree<T>::size() {
    return size_;
}

// string representation of the tree
template <class T>
std::string BTree<T>::to_string() {

    // reference: https://codereview.stackexchange.com/questions/35656/printing-out-a-binary-tree-level-by-level

    std::string str = "";
    if (root == NULL) {
        return str;
    }
    Node<T> *n_ptr = root;
    int level = 0;

    // Use a queue for breadth-first traversal of the tree.  The pair is
    // to keep track of the depth of each node (Depth of root node is 1)
    typedef std::pair<Node<T> *, int> node_level;
    std::queue<node_level> q;
    q.push(node_level(n_ptr, 1));

    while (!q.empty()) {
        node_level nl = q.front();
        q.pop();
        if (nullptr != (n_ptr = nl.first)) {
            if (level != nl.second) {
                str += "\nLevel " + std::to_string(nl.second) + ": ";
                level = nl.second;
            }
            str += node_to_string(*n_ptr);
            if (!n_ptr->is_leaf) {
                for (int i = 0; i <= n_ptr->num_keys; ++i) {
                    q.push(node_level(n_ptr->children[i], level + 1));
                }
            }
        }
    }

    return str;
}

// remove the least recently accessed element (tail) from the
//  b-tree and the linked list
template <class T>
T BTree<T>::remove_lru() {
    if (head->next == head) {
        return NULL;
    }
    T lru = head->prev->key;
    remove(lru);
    return lru;
}

// remove the most recently accessed element (head) from
//  the b-tree and the linked list
template <class T>
T BTree<T>::remove_mru() {
    if (head->next == head) {
        return NULL;
    }
    T mru = head->next->key;
    remove(mru);
    return mru;
}

// print linked list from the most recently accessed (head) to
//  the least recently accessed element (tail)
template <class T>
std::string BTree<T>::print_ordered_mru() {

    Element<T> *elmt = head->next;
    std::string str = "MRU-> ";
    while (elmt != head) {
        str += key_to_string(*elmt) + " ";
        elmt = elmt->next;
    }
    str += " <-LRU";
    return str;

}

// print linked list from the least recently accessed (tail) to
//   the most recently accessed element (head)
template <class T>
std::string BTree<T>::print_ordered_tail() {

    Element<T> *elmt = head->prev;
    std::string str = "(tail) LRU-> ";
    while (elmt != head) {
        str += key_to_string(*elmt) + " ";
        elmt = elmt->prev;
    }
    str += " <-MRU";
    return str;

}

#endif // BTREE_H
