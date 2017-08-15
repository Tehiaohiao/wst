#ifndef WORKINGSETTREE_H
#define WORKINGSETTREE_H

#include <string>
#include <utility> // for std::pair
#include "node.h"
#include "btree.h"

const int DEFAULT_MINIMUM_DEGREE = 2;
const int DEFAULT_SCALE_FACTOR = 2;
const int BASE_HEIGHT = 2; // the max height of the smallest b-tree

template <class T>
class WorkingSetTree {
public:
    WorkingSetTree() : min_degree(DEFAULT_MINIMUM_DEGREE), scale_factor(DEFAULT_SCALE_FACTOR) {
        //std::shared_ptr<BTree<T>> tree = std::make_shared<BTree<T>>(min_degree, BASE_HEIGHT);
        BTree<T> *tree = new BTree<T>(min_degree, BASE_HEIGHT);
        trees.push_back(tree);
    }
    WorkingSetTree(int degree, int factor = DEFAULT_SCALE_FACTOR) : min_degree(degree), scale_factor(factor) {
        //std::shared_ptr<BTree<T>> tree = std::make_shared<BTree<T>>(min_degree, BASE_HEIGHT);
        BTree<T> *tree = new BTree<T>(min_degree, BASE_HEIGHT);
        trees.push_back(tree);
    }
    ~WorkingSetTree();
    void insert(T value);
    //std::pair<Node<T>&, int> search(T val);
    bool search(T val);
    bool remove(T val);
    int size();
    std::string to_string();
    std::string print_list();
private:
    int min_degree;
    int scale_factor;
    int size_;
    //std::vector<std::shared_ptr<BTree<T>>> trees;
    std::vector<BTree<T>*> trees;
    void shift_back(int start_tree_index);
    void shift_forward(int tree_index);
};

template <class T>
WorkingSetTree<T>::~WorkingSetTree() {
    int num_trees = trees.size();
    for (int i = 0; i < num_trees; ++i) {
        delete trees[i];
    }
}

template <class T>
void WorkingSetTree<T>::insert(T value) {
    trees[0]->insert(value);
    shift_back(0);
    size_++;
}

/*
template <class T>
std::pair<Node<T>&, int> WorkingSetTree<T>::search(T val) {

    int index = 0;
    int num_trees = trees.size();
    while (index < num_trees) {
        std::pair<Node<T>&, int> node_index = trees[index]->search(val);
        if (node_index.second == -1) { // val not found in this tree
            index++;
        }
        else { // val found. move val to the previous tree (if at tree index 0, move to beginning)
            int new_index = index - 1;
            if (new_index < 0) {
                new_index = 0;
            }
            trees[index]->remove(val);

            trees[new_index]->insert(val);
            shift_back(new_index);

            // TODO move things to tree at index if not enough elements?
            shift_forward(index);

            return trees[new_index]->search(val);
        }
    }
    return trees[0]->search(val);

}
*/

template <class T>
bool WorkingSetTree<T>::search(T val) {

    int index = 0;
    int num_trees = trees.size();
    while (index < num_trees) {
        std::pair<Node<T>&, int> node_index = trees[index]->search(val);
        if (node_index.second == -1) { // val not found in this tree
            index++;
        }
        else { // val found. move val to the previous tree (if at tree index 0, move to beginning)
            int new_index = index - 1;
            if (new_index < 0) {
                new_index = 0;
            }
            trees[index]->remove(val);

            trees[new_index]->insert(val);
            shift_back(new_index);

            // TODO move things to tree at index if not enough elements?
            shift_forward(index);

            return true;
        }
    }
    return false;

}


template <class T>
bool WorkingSetTree<T>::remove(T val) {
    int index = 0;
    int num_trees = trees.size();
    while (index < num_trees) {
        if (trees[index]->remove(val)) {
            shift_forward(index);
            size_--;
            return true;
        }
        index++;
    }
    return false;
}

template <class T>
int WorkingSetTree<T>::size() {
    return size_;
}

template <class T>
void WorkingSetTree<T>::shift_back(int start_tree_index) {

    int index = start_tree_index;
    while (trees[index]->get_height() > trees[index]->get_max_height()) {
        int max_height = trees[index]->get_max_height();
        while (trees[index]->get_height() > max_height) {
            T lru = trees[index]->remove_lru();
            if (trees.size() == index + 1) {
                //trees.push_back(std::make_shared<BTree<T>>(min_degree, trees.back()->get_max_height()*scale_factor));
                BTree<T> *tree = new BTree<T>(min_degree, trees.back()->get_max_height() * scale_factor);
                trees.push_back(tree);
            }
            trees[index + 1]->insert(lru);
        }
        index++;
    }
}

template <class T>
void WorkingSetTree<T>::shift_forward(int tree_index) {
    int index = tree_index;
    int num_trees = trees.size();
    while ((index + 1<num_trees) && trees[index]->get_height() < trees[index]->get_max_height()) { // TODO what is the condition for shifting forward??
        int max_height = trees[index]->get_max_height();
        while (trees[index]->get_height() < max_height) {
            T mru = trees[index + 1]->remove_mru();
            trees[index]->insert_lru(mru);
        }
        index++;
    }
}

template <class T>
std::string WorkingSetTree<T>::to_string() {
    std::string str = "";
    int num_trees = trees.size();
    for (int i = 0; i < num_trees; ++i) {
        str += "Tree " + std::to_string(i) + ":\n" + trees[i]->to_string() + "\n\n";
    }
    return str;
}

template <class T>
std::string WorkingSetTree<T>::print_list() {
    std::string str = "";
    int num_trees = trees.size();
    for (int i = 0; i < num_trees; ++i) {
        str += "Tree " + std::to_string(i) + ":\n";
        str += trees[i]->print_ordered_mru();
        str += "\n";
        str += trees[i]->print_ordered_tail();
        str += "\n\n";
    }
    return str;
}

#endif // WORKINGSETTREE_H
