//
// Created by Eddard on 2023-03-05.
//

#include "ordered_set.h"

#define iterator typename ordered_set<T>::Iterator


template<typename T>
ordered_set<T>::ordered_set(initializer_list<T> values) { for (T value: values) insert(value); }


// Tree Properties
template<typename T>
int ordered_set<T>::size() const { return root_ ? root_->size_ : 0; }

template<typename T>
bool ordered_set<T>::empty() const { return !size(); }

template<typename T>
void ordered_set<T>::clear() { root_.reset(); }


// Getting

// Returns an iterator to the smallest element.
template<typename T>
iterator ordered_set<T>::begin() const { return find_by_order(0); }

// Returns past-the-end (end()) iterator.
template<typename T>
iterator ordered_set<T>::end() const { return Iterator(this); }

// Returns an iterator to the element equal to key, or end() iterator if no such element exists in the set.
template<typename T>
iterator ordered_set<T>::find(T key) const {
    Node *u = root_.get();
    while (u) {
        if (key == u->value_) break;
        if (key < u->value_) u = u->child_[LEFT].get();
        else u = u->child_[RIGHT].get();
    }
    return Iterator(this, u);
}

// Returns an iterator to the least element greater than or equal to key, or end() iterator if no such element exists in the set.
template<typename T>
iterator ordered_set<T>::lower_bound(T key) const {
    Node *u = root_.get();
    Node *ret = nullptr;
    while (u) {
        if (key <= u->value_) ret = u, u = u->child_[LEFT].get();
        else u = u->child_[RIGHT].get();
    }
    return Iterator(this, ret);
}

// Returns an iterator to the least element greater than key, or end() iterator if no such element exists in the set.
template<typename T>
iterator ordered_set<T>::upper_bound(T key) const {
    Node *u = root_.get();
    Node *ret = nullptr;
    while (u) {
        if (key < u->value_) ret = u, u = u->child_[LEFT].get();
        else u = u->child_[RIGHT].get();
    }
    return Iterator(this, ret);
}

// Returns an iterator to the (k+1)-th least element, or end() iterator if the size of the set is less than k+1.
template<typename T>
iterator ordered_set<T>::find_by_order(int k) const {
    if (!root_ || k < 0 || k >= root_->size_) return end();
    
    Node *u = root_.get();
    while (true) {
        int l_size = u->child_[LEFT] ? u->child_[LEFT]->size_ : 0;
        if (l_size == k) return Iterator(this, u);
        if (l_size > k) {
            u = u->child_[LEFT].get();
        } else {
            k -= l_size + 1;
            u = u->child_[RIGHT].get();
        }
    }
    
}

// Returns the number of elements in the set strictly less than key.
template<typename T>
int ordered_set<T>::order_of_key(T key) const {
    Node *u = root_.get();
    int order = 0;
    while (u) {
        int l_size = u->child_[LEFT] ? u->child_[LEFT]->size_ : 0;
        if (key == u->value_) return order + l_size;
        if (key <= u->value_) {
            u = u->child_[LEFT].get();
        } else {
            order += l_size + 1;
            u = u->child_[RIGHT].get();
        }
    }
    return order;
}


// Inserting

template<typename T>
pair<iterator, bool> ordered_set<T>::insert(Node *u) {
    if (!root_) {
        root_.reset(u);
        u->color_ = BLACK;
        return {Iterator(this, u), true};
    }
    Node *current = root_.get(), *parent = nullptr;
    Direction direction;
    while (current) {
        if (u->value_ == current->value_) {
            delete u;
            return {Iterator(this, current), false};
        }
        if (u->value_ < current->value_) {
            parent = current;
            current = current->child_[LEFT].get();
            direction = LEFT;
        } else {
            parent = current;
            current = current->child_[RIGHT].get();
            direction = RIGHT;
        }
    }
    add_child(parent, u, direction);
    update_size(parent);
    insert_fix(u);
    return {iterator(this, u), true};
}

// Inserts key into the set. Returns true if it didn't exist before calling. Otherwise, returns false.
template<typename T>
pair<iterator, bool> ordered_set<T>::insert(T key) { return insert(new Node(key)); }

template<typename T>
void ordered_set<T>::insert_fix(Node *u) {
    // Only called on a red node.
    assert(u->color_ == RED);
    
    Node *parent = u->parent_;
    
    if (!parent) {
        u->color_ = BLACK;
        return;
    }
    
    if (parent->color_ == BLACK)
        return;
    
    
    Direction direction = get_direction(u);
    Direction p_direction = get_direction(parent);
    
    Node *grandparent = parent->parent_;
    Node *uncle = grandparent->child_[!p_direction].get();
    
    // Parent is red(1), and thus not the root(2). So, grandparent exists(2) and is black(1).
    assert(grandparent->color_ == BLACK);
    
    if (!uncle || uncle->color_ == BLACK) {
        // The fix ends here, at most 2 rotations to be done.
        if (direction != p_direction) {
            // Make u the outer child, rotation won't cause violations as both u and parent are red.
            rotate(parent, !direction);
            swap(u, parent);
        }
        // Rotate grandparent to get parent on top and swap colors with it. No more red violations and the black depth is the same for the (now parent's) subtree.
        rotate(grandparent, !p_direction);
        grandparent->color_ = RED;
        parent->color_ = BLACK;
    } else {
        // The recursive part.
        // Propagate black from grandparent down to parent and uncle.
        grandparent->color_ = RED;
        parent->color_ = uncle->color_ = BLACK;
        // Grandparent is red, and we don't know about its parent, call insert_fix on it again.
        insert_fix(grandparent);
    }
    
}


// Erasing

template<typename T>
bool ordered_set<T>::erase(Node *u) {
    if (!u) return false; // Doesn't exist
    
    if (u->child_[LEFT] && u->child_[RIGHT]) {
        // Go to the successor of the node, which is the one with the least value in the right child subtree.
        // It can't have a left child, so it will have at most one child.
        Node *v = u->child_[RIGHT].get();
        while (v->child_[LEFT]) v = v->child_[LEFT].get();
        swap(u->value_, v->value_);
        u = v;
    }
    
    if (!u->child_[LEFT] && !u->child_[RIGHT]) {
        if (!u->parent_) {
            clear();
            return true;
        }
        if (u->color_ == RED) {
            Node *p = u->parent_;
            p->child_[get_direction(u)].reset();
            update_size(p);
            return true;
        }
        // Black non-root leaf.
        // Replace u with a black dummy node and start fixing from dummy
        Node *dummy = new Node();
        dummy->size_ = 0;
        dummy->color_ = BLACK;
        
        Node *p = u->parent_;
        Direction direction = get_direction(u);
        
        p->child_[direction].reset();
        add_child(p, dummy, direction);
        
        update_size(dummy->parent_);
        erase_fix(dummy);
        dummy->parent_->child_[get_direction(dummy)].reset();
        return true;
    }
    Direction c_direction = u->child_[RIGHT] ? RIGHT : LEFT;
    
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NullDereference"
    Node *child = u->child_[c_direction].release();
#pragma clang diagnostic pop
    
    // Non-leaf with one child. So, child is a red leaf, and u has to be black.
    assert(child->color_ == RED && child->size_ == 1 && u->color_ == BLACK);
    // Replace u with child and color it black. No fixes needed.
    child->parent_ = u->parent_;
    child->color_ = BLACK;
    unique_ptr<Node> &ptr = u->parent_ ? u->parent_->child_[get_direction(u)] : root_;
    ptr.reset(child);
    
    update_size(child);
    return true;
}

// Erases key from the set. Returns true if it didn't exist before calling. Otherwise, returns false.
template<typename T>
bool ordered_set<T>::erase(T key) {
    Node *u = root_.get();
    while (u) {
        if (key == u->value_) break;
        if (key < u->value_) u = u->child_[LEFT].get();
        else u = u->child_[RIGHT].get();
    }
    return erase(u);
}

template<typename T>
void ordered_set<T>::erase_fix(Node *u) {
    // Only called on a black node
    assert(u->color_ == BLACK);
    Node *parent = u->parent_;
    
    if (!parent)
        return;
    
    Direction direction = get_direction(u);
    Node *sibling = parent->child_[!direction].get();
    Node *close_nephew = sibling->child_[direction].get();
    Node *distant_nephew = sibling->child_[!direction].get();
    
    // Sibling must exist, since if it's nullptr, its black depth is less than u's subtree.
    // Nephews may not exist if erase_fix is called on the dummy node (first call)
    
    if (parent->color_ == BLACK && sibling->color_ == BLACK && (!close_nephew || close_nephew->color_ == BLACK) && (!distant_nephew || distant_nephew->color_ == BLACK)) {
        // Go up.
        sibling->color_ = RED;
        return erase_fix(parent);
    }
    
    if (sibling->color_ == RED) {
        // Now nephews must both exist and be black, rotate the parent to u's direction and swap its color with the (previously) sibling, then continue fixing from u.
        swap(parent->color_, sibling->color_);
        rotate(parent, direction);
        sibling = close_nephew;
        close_nephew = sibling->child_[direction].get();
        distant_nephew = sibling->child_[!direction].get();
        // close nephew is now the sibling, so sibling is black.
    }
    
    // u and sibling are black. at least one of the involved nodes is red
    
    if ((!close_nephew || close_nephew->color_ == BLACK) && (!distant_nephew || distant_nephew->color_ == BLACK)) {
        // parent is currently the only unchecked node, so it's red.
        // push the sibling's black to the parent, now both subtrees have the same black depth.
        swap(parent->color_, sibling->color_);
        return;
    }
    
    // at least one nephew exists and is red, we need it to be the distant one for the final fix.
    
    if (!distant_nephew || distant_nephew->color_ == BLACK) {
        // close nephew is the red one, some rotations and changing colors will do.
        swap(sibling->color_, close_nephew->color_);
        rotate(sibling, !direction);
        sibling = close_nephew;
        close_nephew = sibling->child_[direction].get();
        distant_nephew = sibling->child_[!direction].get();
        // Now sibling is still black, but distant nephew is red.
    }
    
    // Parent's color is x, sibling is black, distant nephew is red, close nephew's color doesn't matter.
    // u's subtree needs to pass through an extra black node to the root.
    distant_nephew->color_ = BLACK;
    swap(parent->color_, sibling->color_);
    rotate(parent, direction);
    // close nephew's subtree passes through the same nodes (parent and sibling with black and x colors), no changes in black depth.
    // Distant nephew's subtree passes through the black added to distant nephew instead of the sibling's black (swapped to parent), no changes in black depth.
    // u's subtree passes through an extra black from the sibling, fixed.
    
}


// Node Functions

template<typename T>
void ordered_set<T>::add_child(Node *u, Node *child, Direction direction) {
    assert(!u->child_[direction]);
    u->child_[direction].reset(child);
    child->parent_ = u;
}

template<typename T>
void ordered_set<T>::update_size(Node *u, bool recursive) {
    if (!u) return;
    u->size_ = 1;
    for (auto &c: u->child_.data) if (c) u->size_ += c->size_;
    if (recursive) update_size(u->parent_);
}

template<typename T>
Direction ordered_set<T>::get_direction(Node *u) {
    assert(u->parent_);
    return u->parent_->child_[RIGHT].get() == u ? RIGHT : LEFT;
}

template<typename T>
void ordered_set<T>::rotate(Node *u, Direction direction) {
    
    Direction u_direction;
    
    // Create pointers to parent, child, and inner grandchild, while releasing unique_ptrs from u, child, and inner grandchild.
    Node *parent = u->parent_;
    if (parent) parent->child_[u_direction = get_direction(u)].release(); else root_.release();
    Node *child = u->child_[!direction].release();
    Node *grandchild = child->child_[direction].release();
    
    // Reattach unique_ptrs to u, -previously- child, and inner grandchild
    add_child(child, u, direction);
    if (parent) add_child(parent, child, u_direction); else root_.reset(child), child->parent_ = nullptr;
    if (grandchild) add_child(u, grandchild, !direction);
    
    // Non-recursively update the sizes of the 2 nodes involved in the rotation, no other node sizes are affected.
    update_size(u, false);
    update_size(child, false);
    
}


// Iterator Functions

template<typename T>
ordered_set<T>::Iterator::Iterator(const ordered_set *tree, Node *ptr):tree_(tree), ptr_(ptr) {}

template<typename T>
const T &ordered_set<T>::Iterator::operator*() {
    if (!ptr_) throw out_of_range("Dereferencing end iterator.");
    return ptr_->value_;
}

template<typename T>
T const *ordered_set<T>::Iterator::operator->() {
    if (!ptr_) throw out_of_range("Dereferencing end iterator.");
    return &ptr_->value_;
}

template<typename T>
bool ordered_set<T>::Iterator::operator==(const Iterator &other) const { return ptr_ == other.ptr_; }

template<typename T>
bool ordered_set<T>::Iterator::operator!=(const Iterator &other) const { return ptr_ != other.ptr_; }

template<typename T>
iterator &ordered_set<T>::Iterator::operator++() {
    
    if (!ptr_) throw out_of_range("Incrementing end iterator.");
    
    if (ptr_->child_[RIGHT]) {
        ptr_ = ptr_->child_[RIGHT].get();
        while (ptr_->child_[LEFT]) ptr_ = ptr_->child_[LEFT].get();
    } else {
        while (ptr_->parent_ && get_direction(ptr_) == RIGHT) ptr_ = ptr_->parent_;
        ptr_ = ptr_->parent_; // In case of nullptr, it means no element is greater than current element, so it becomes an end iterator.
    }
    
    return *this;
}

template<typename T>
iterator ordered_set<T>::Iterator::operator++(int) {
    Iterator tmp(*this);
    ++(*this);
    return tmp;
}

template<typename T>
iterator &ordered_set<T>::Iterator::operator--() {
    
    if (!ptr_) {
        *this = tree_->find_by_order(tree_->size() - 1);
    } else if (ptr_->child_[LEFT]) {
        ptr_ = ptr_->child_[LEFT].get();
        while (ptr_->child_[RIGHT]) ptr_ = ptr_->child_[RIGHT].get();
    } else {
        while (ptr_->parent_ && get_direction(ptr_) == LEFT) ptr_ = ptr_->parent_;
        ptr_ = ptr_->parent_; // In case of nullptr, it means no element is smaller than current element, so it was a begin iterator.
        if (!ptr_) throw out_of_range("Decrementing begin iterator.");
    }
    
    return *this;
}

template<typename T>
iterator ordered_set<T>::Iterator::operator--(int) {
    Iterator tmp(*this);
    --(*this);
    return tmp;
}
