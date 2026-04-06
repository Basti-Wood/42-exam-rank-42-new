#include "set.hpp"

namespace {
class null_searchable_bag : public searchable_bag {
public:
  void insert(int) {}
  void insert(int *, int) {}
  void print() const {}
  void clear() {}
  bool has(int) const { return false; }
};

searchable_bag &default_bag() {
  static null_searchable_bag bag;
  return bag;
}
} // namespace

set::set() : searchable_bag(), wrapped_bag(&default_bag()) {}

set::set(searchable_bag &bag) : searchable_bag(), wrapped_bag(&bag) {}

set::set(const set &src) : searchable_bag(src), wrapped_bag(src.wrapped_bag) {}

set &set::operator=(const set &src) {
  if (this != &src)
    wrapped_bag = src.wrapped_bag;
  return *this;
}

set::~set() {}

void set::set_bag(searchable_bag &bag) { wrapped_bag = &bag; }

searchable_bag &set::get_bag() { return *wrapped_bag; }

const searchable_bag &set::get_bag() const { return *wrapped_bag; }

void set::insert(int value) {
  if (!wrapped_bag->has(value))
    wrapped_bag->insert(value);
}

void set::insert(int *array, int size) {
  if (array == 0 || size <= 0)
    return;
  for (int i = 0; i < size; ++i)
    insert(array[i]);
}

void set::print() const { wrapped_bag->print(); }

void set::clear() { wrapped_bag->clear(); }

bool set::has(int value) const { return wrapped_bag->has(value); }
