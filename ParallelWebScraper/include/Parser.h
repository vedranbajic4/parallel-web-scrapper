#pragma once
#include "Book.h"
#include <vector>
#include <string>
#include <gumbo.h>
#include "tbb/tbb.h"
#include "tbb/concurrent_vector.h"
#include <iostream>

class Parser {
	std::vector<std::string> pages;

	void parse_page(std::string html, tbb::concurrent_vector<Book>& result) const;

	//this will parse a range of html string and extract books from it
	void parse_range(const tbb::blocked_range<std::size_t>& range, tbb::concurrent_vector<Book>& result) const;
public:
	Parser(std::vector<std::string> pages);

	//this will return books from given html strings representing pages
	void parse_pages(tbb::concurrent_vector<Book>& result);
};