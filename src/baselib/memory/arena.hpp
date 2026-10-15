#pragma once

#include "types.hpp"
#include <cstdlib>

class arena_allocator {
private:
	void* mem_;
	usize cap_;
	usize offset_;
public:
	arena_allocator();
	~arena_allocator();
	

};

