#include "searchable_tree_bag.hpp"

searchable_tree_bag::searchable_tree_bag() : tree_bag(), searchable_bag() {}

searchable_tree_bag::searchable_tree_bag(const searchable_tree_bag &src)
    : tree_bag(src), searchable_bag(src) {}

searchable_tree_bag &searchable_tree_bag::operator=(const searchable_tree_bag &src) {
  if (this != &src)
    tree_bag::operator=(src);
  return *this;
}

searchable_tree_bag::~searchable_tree_bag() {}

bool searchable_tree_bag::has(int value) const {
  node *current = tree;

  while (current != 0) {
    if (value == current->value)
      return true;
    if (value < current->value)
      current = current->l;
    else
      current = current->r;
  }
  return false;
}