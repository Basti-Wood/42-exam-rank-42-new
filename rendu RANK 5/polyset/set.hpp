#pragma once

#include "searchable_bag.hpp"

class set : public searchable_bag {
private:
  searchable_bag *wrapped_bag;

public:
  set();
  set(searchable_bag &);
  set(const set &);
  set &operator=(const set &);
  ~set();

  void set_bag(searchable_bag &);
  searchable_bag &get_bag();
  const searchable_bag &get_bag() const;

  void insert(int);
  void insert(int *, int);
  void print() const;
  void clear();
  bool has(int) const;
};
