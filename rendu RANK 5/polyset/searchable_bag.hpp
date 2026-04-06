#pragma once

#include "bag.hpp"

class searchable_bag : virtual public bag {
public:
	virtual bool has(int) const = 0;
};

#include "searchable_array_bag.hpp"
#include "searchable_tree_bag.hpp"
#include "set.hpp"
