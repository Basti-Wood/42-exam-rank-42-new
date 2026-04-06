#include "tree_bag.hpp"
#include <iostream>

tree_bag::tree_bag() {
	tree = 0;
}

tree_bag::tree_bag(const tree_bag &src) {
	tree = copy_node(src.tree);
}

tree_bag::~tree_bag() {
	destroy_tree(tree);
}

tree_bag &tree_bag::operator=(const tree_bag &src) {
	if (this != &src) {
		destroy_tree(tree);
		tree = copy_node(src.tree);
	}
	return *this;
}

tree_bag::node *tree_bag::extract_tree() {
	node *temp = tree;
	tree = 0;
	return temp;
}

void tree_bag::set_tree(node *new_tree) {
	destroy_tree(tree);
	tree = new_tree;
}

void tree_bag::insert(int item) {
	node *new_node = new node;
	new_node->value = item;
	new_node->l = 0;
	new_node->r = 0;

	if (tree == 0) {
		tree = new_node;
	} else {
		node *current = tree;
		while (true) {
			if (item < current->value) {
				if (current->l == 0) {
					current->l = new_node;
					break;
				} else {
					current = current->l;
				}
			} else if (item > current->value) {
				if (current->r == 0) {
					current->r = new_node;
					break;
				} else {
					current = current->r;
				}
			} else {
				delete new_node;
				break;
			}
		}
	}
}

void tree_bag::insert(int *items, int count) {
	for (int i = 0; i < count; i++) {
		insert(items[i]);
	}
}

void tree_bag::print() const {
	print_node(tree);
	std::cout << std::endl;
}

void tree_bag::clear() {
	destroy_tree(tree);
	tree = 0;
}

// defined as static functions in the class
void tree_bag::destroy_tree(node *current) {
	if (current != 0) {
		destroy_tree(current->l);
		destroy_tree(current->r);
		delete current;
	}
}

void tree_bag::print_node(node *current) {
	if (current != 0) {
		print_node(current->l);
		std::cout << current->value << " ";
		print_node(current->r);
	}
}

tree_bag::node *tree_bag::copy_node(node *current) {
	if (current == 0) {
		return 0;
	} else {
		node *new_node = new node;
		new_node->value = current->value;
		new_node->l = copy_node(current->l);
		new_node->r = copy_node(current->r);
		return new_node;
	}
}
