# Ordered Set

The Ordered Set is a data structure that stores unique elements in a sorted order, doing all operations (inserting, erasing, searching, etc..) in $O(log(n))$.

It's similar to <a href = "https://cplusplus.com/reference/set/set/">C++ set</a>, but adds two functions:

find_by_order($k$): returns an iterator that points to the $k$-th smallest element in the set (0-indexed).<br>
order_of_key($key$): returns the number of elements in the set strictly smaller than $key$.

Based on the <a href = "https://en.wikipedia.org/wiki/Red%E2%80%93black_tree">Red-Black Tree</a>. 

UPD: Just added bidirectional iterators. Might make them random-access later, which may not make much sense but would be nice and easy to implement (find_by_order already implemented).

UPD: Just made iterators random-access.
