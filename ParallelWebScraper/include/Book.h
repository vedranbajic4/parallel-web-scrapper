#pragma once
#include <string>
#include <iostream>

class Book{
public:
	bool operator <(const Book& other) const;

	int rating;
	std::string title;
	float price;
	bool in_stock;
	void debug();
	Book(int rating, std::string title, float price, bool in_stock);
};