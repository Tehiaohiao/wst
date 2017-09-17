/*
 * node.h
 *
 * template struct that represents a node in the b-tree. Stores data
 * of type Element<T>. Keys are stored using a vector of size (m*2-1)
 * allocated during construction, and pointers to children are stored
 * using a vector of pre-allocated sizes 2m, where m is the minimum
 * degree of the b-tree.
*/

#ifndef NODE_H
#define NODE_H

#include "element.h"
#include <vector>

const int DEFAULT_MIN_DEGR = 2;

template <class T>
struct Node {
    int num_keys;
    bool is_leaf;
    int min_degree;
    Node<T> *parent;
    int index_in_parent; // its index in the parent's children vector
    std::vector<Element<T> > keys;
    std::vector<Node<T>*> children;

    Node() : num_keys(0), is_leaf(true), min_degree(DEFAULT_MIN_DEGR), index_in_parent(-1) {
        keys.resize(min_degree * 2 - 1);
        children.resize(min_degree * 2);
        parent = nullptr;
    }

    Node(int md) : num_keys(0), is_leaf(true), min_degree(md), index_in_parent(-1) {
        keys.resize(min_degree * 2 - 1);
        children.resize(min_degree * 2);
        parent = nullptr;
    }
};

/* string format: "( *num_keys, max_num_keys, max_num_children, is_leaf* list_of_elements ) */
template <class T>
std::string node_to_string(const Node<T> &node) {
    std::string str = "( ";
    str += "*" + std::to_string(node.num_keys) + "," + std::to_string(node.keys.size()) + "," + std::to_string(node.children.size()) + "," + std::to_string(node.is_leaf) + "* ";
    for (int i = 0; i < node.num_keys; ++i) {
        str += key_to_string(node.keys[i]) + " ";
    }
    return str + ")";
}

#endif // NODE_H
