/*
 * element.h
 *
 * template struct that stores a key of type T, a prev pointer that points to
 * the element inserted into the tree after the given element, and a next pointer
 * that points to the element inserted into the tree before the given element.
 */

#ifndef ELEMENT_H
#define ELEMENT_H

#include <vector>

template <class T>
struct Element {
    T key;
    Element<T> *prev;
    Element<T> *next;

    Element() {
        key = NULL;
        prev = nullptr;
        next = nullptr;
    }

    Element(T k, Element *p, Element *n) {
        key = k;
        prev = p;
        next = n;
    }

};

/*
representing the element as a string, in the format "#key,next_key,prev_key#"
*/
template <class T>
std::string key_to_string(const Element<T> &element) {
    std::string str = "#";
    str += std::to_string(element.key) + "," + std::to_string(element.next->key)
        + "," + std::to_string(element.prev->key) + "#";
    return str;
}

#endif // ELEMENT_H
