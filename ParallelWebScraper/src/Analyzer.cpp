#include "Analyzer.h"

Analyzer::Analyzer(tbb::concurrent_vector<Book> books, const std::string& output_dir) : 
	books(books),
	directory(output_dir)
{
	avg_book_price = 0;
	books_1_word = 0;
	books_5_star = 0;
	cheap_books = 0;
}

void Analyzer::write_to_file() {
	std::ofstream out(directory + "/result.txt");
	if (!out.is_open()) {
		std::cerr << "Failed to open file: " << directory + "/result.txt" << std::endl;
		return;
	}
	out << "avg_book_price: " << avg_book_price << std::endl;
	out << "books_1_word: " << books_1_word << std::endl;
	out << "books_5_star: " << books_5_star << std::endl;
	out << "cheap_books: " << cheap_books << std::endl;
	out << "10 cheapest books: \n";
	for (Book b: cheapest_books) {
		out << b.price << " " << b.title << std::endl;
	}

	out.close();
}

std::vector<Book> Analyzer::find_10_cheapest(int l, int r) {
	// stop condition, sort books in range -> return
	if (r - l <= 10) {
		std::vector<Book> cheap_books;

		cheap_books.reserve(r - l);
		for (int i = l; i < r; ++i) {
			cheap_books.emplace_back(books[i]);
		}

		std::sort(cheap_books.begin(), cheap_books.end());
		return cheap_books;
	}

	tbb::task_group g;
	int mid = l + (r - l) / 2;
	std::vector<Book> left_result, right_result;

	g.run([this, &left_result, l, mid]() {
		left_result = find_10_cheapest(l, mid);
		});

	g.run([this, &right_result, mid, r]() {
		right_result = find_10_cheapest(mid, r);
		});
	
	g.wait();

	int lid = 0, rid = 0;
	std::vector<Book> result;

	while(result.size() < 10){
		if(lid >= left_result.size() && rid >= right_result.size()){
			break;
		}

		if(lid >= left_result.size()){
			result.push_back(right_result[rid++]);
		}
		else if(rid >= right_result.size()){
			result.push_back(left_result[lid++]);
		}
		else if(left_result[lid].price < right_result[rid].price){
			result.push_back(left_result[lid++]);
		}
		else{
			result.push_back(right_result[rid++]);
		}
	}

	return result;
}

void Analyzer::analyze_books(){
	// calculating number of books with 5 star rating
	books_5_star = tbb::parallel_reduce(
		tbb::blocked_range<std::size_t>(0, books.size()),
		0,
		[this](const tbb::blocked_range<std::size_t>& range, int local_sum) {
			for (int i = range.begin(); i != range.end(); i++) {
				if (books[i].rating == 5) {
					local_sum++;
				}
			}
			return local_sum;
		},
		std::plus<int>()
	);

	// calculating number of books with 1 word in title
	books_1_word = tbb::parallel_reduce(
		tbb::blocked_range<std::size_t>(0, books.size()),
		0,
		[this](const tbb::blocked_range<std::size_t>& range, int local_sum) {
			for (int i = range.begin(); i != range.end(); i++) {
				if (books[i].title.find(" ") == std::string::npos) {
					local_sum++;
				}
			}
			return local_sum;
		},
		std::plus<int>()
	);

	// calculating average price 
	int sum = tbb::parallel_reduce(
		tbb::blocked_range<std::size_t>(0, books.size()),
		0,
		[this](const tbb::blocked_range<std::size_t>& range, int local_sum) {
			for (int i = range.begin(); i != range.end(); i++) {
				local_sum += books[i].price;
			}
			return local_sum;
		},
		std::plus<int>()
	);
	avg_book_price = 1.0 * sum / books.size();

	// calculating number of cheap books (price < 20$)
	cheap_books = tbb::parallel_reduce(
		tbb::blocked_range<std::size_t>(0, books.size()),
		0,
		[this](const tbb::blocked_range<std::size_t>& range, int local_sum) {
			for (int i = range.begin(); i != range.end(); i++) {
				if (books[i].price < 20) {
					local_sum++;
				}
			}
			return local_sum;
		},
		std::plus<int>()
	);

	//finding 10 cheapest book
	std::vector<Book> result = find_10_cheapest(0, books.size());
	
	for(Book b: result) {
		cheapest_books.push_back(b);
	}
}