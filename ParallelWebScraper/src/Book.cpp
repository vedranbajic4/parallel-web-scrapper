#include "Book.h"

Book::Book(int r, std::string t, float p, bool b) : rating(r), title(t), price(p), in_stock(b) {

};

bool Book::operator <(const Book& other) const {
	return price < other.price;
}

void Book::debug() {
	std::cout << "Title: " << title << "\n";
	std::cout << "Rating: " << rating << "\n";
	std::cout << "Price: " << price << "\n";
	std::cout << "In_stock: " << in_stock << "\n\n";
}