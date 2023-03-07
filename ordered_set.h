//
// Created by Eddard on 2023-03-05.
//

#ifndef ORDERED_SET_ORDERED_SET_H
#define ORDERED_SET_ORDERED_SET_H

#include <bits/stdc++.h>
using namespace std;

enum class Direction {
    LEFT = 0, RIGHT = 1
};
using Direction::LEFT;
using Direction::RIGHT;
inline Direction operator!(Direction direction) { return direction == LEFT ? RIGHT : LEFT; }

enum class Color {
    RED = 0, BLACK = 1
};
using Color::RED;
using Color::BLACK;
inline Color operator!(Color color) { return color == RED ? BLACK : RED; }

template<typename T>
class ordered_set {
private:
    class Node;

public:
    [[nodiscard]] int size() const;
    
    [[nodiscard]] bool empty() const;
    
    void clear();
    
    Node *find(T key) const;
    
    Node *lower_bound(T key) const;
    
    Node *upper_bound(T key) const;
    
    Node *find_by_order(int k) const;
    
    int order_of_key(T key) const;
    
    bool insert(T key);
    
    bool erase(T key);

private:
    class Node {
        friend class ordered_set;
    
    public:
        
        Node() = default;
        
        explicit Node(T value, Node *parent = nullptr, Color color = RED);
    
    private:
        class Array {
        public:
            unique_ptr<Node> data[2];
            unique_ptr<Node> &operator[](Direction direction) { return data[direction==RIGHT]; }
            const unique_ptr<Node> &operator[](Direction direction) const { return data[direction==RIGHT]; }
        };
    
        T value_;
        Array child_;
        Node *parent_ = nullptr;
        Color color_ = RED;
        int size_ = 1;
    };
    
    unique_ptr<Node> root_;
    
    // Node functions
    static void add_child(Node *u, Node *child, Direction direction);
    
    static void update_size(Node *u, bool recursive = true);
    
    static Direction get_direction(Node *u);
    
    void rotate(Node *u, Direction direction);
    
    bool insert(Node *u);
    
    void insert_fix(Node *u);
    
    bool erase(Node *u);
    
    void erase_fix(Node *u);
};


#endif //ORDERED_SET_ORDERED_SET_H
