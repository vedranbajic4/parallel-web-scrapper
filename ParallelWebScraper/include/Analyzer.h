#pragma once
#include <vector>
#include "Book.h"
#include "tbb/parallel_reduce.h"
#include "tbb/blocked_range.h"
#include "tbb/task_group.h"
#include "tbb/concurrent_vector.h"
#include <algorithm>
#include <fstream>

class Analyzer {
	tbb::concurrent_vector<Book> books;
	std::string directory;
	int books_5_star;							//number of 5 star books
	float avg_book_price;						//average price of book
	int books_1_word;							//books with title have 1 word
	int cheap_books;							//books with price below 20
	std::vector<Book> cheapest_books;	//10 cheapest books

	std::vector<Book> find_10_cheapest(int l, int r);
public:
	Analyzer(tbb::concurrent_vector<Book> books, const std::string& output_dir);
	void analyze_books();
	void write_to_file();
};