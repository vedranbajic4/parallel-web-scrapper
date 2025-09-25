#include "Parser.h"

Parser::Parser(std::vector<std::string> pages) : pages(pages) {
}

void parse_book(GumboNode* node, tbb::concurrent_vector<Book>& result) {
	int rating = 0;			//class: star-rating && Three, Four, One...
	std::string title;		//h3->a
	float price = 0.0f;		//class: product_price -> class: price_color
	bool in_stock = false;	//class: product_price -> class: availability

	GumboVector* children = &node->v.element.children;

	for (unsigned int i = 0; i < children->length; ++i) {
		GumboNode* child = static_cast<GumboNode*>(children->data[i]);
		if (child->type != GUMBO_NODE_ELEMENT) {
			continue;
		}

		GumboElement& el = child->v.element;
		// title extraction
		if (el.tag == GUMBO_TAG_H3) {
			//searching for a tag inside h3
			const GumboVector* h3_children = &el.children;
			for (int j = 0; j < h3_children->length; ++j) {
				GumboNode* h3_child = static_cast<GumboNode*>(h3_children->data[j]);
				if (h3_child->type != GUMBO_NODE_ELEMENT) {
					continue;
				}
				GumboElement& h3_el = h3_child->v.element;
				if (h3_el.tag == GUMBO_TAG_A) {
					GumboAttribute* titleAttr = gumbo_get_attribute(&h3_el.attributes, "title");
					if (titleAttr) {
						title = titleAttr->value;
						break;
					}
				}
			}
		}
		
		// rating extraction
		else if (el.tag == GUMBO_TAG_P) {
			std::string class_value = "";
			auto class_attr = gumbo_get_attribute(&el.attributes, "class");
			if (class_attr) {
				class_value = class_attr->value;
			}
			if (class_value.find("star-rating") != std::string::npos) {
				if (class_value.find("One") != std::string::npos) {
					rating = 1;
				}
				else if (class_value.find("Two") != std::string::npos) {
					rating = 2;
				}
				else if (class_value.find("Three") != std::string::npos) {
					rating = 3;
				}
				else if (class_value.find("Four") != std::string::npos) {
					rating = 4;
				}
				else if (class_value.find("Five") != std::string::npos) {
					rating = 5;
				}
			}
		}

		else if (el.tag == GUMBO_TAG_DIV) {
			auto class_attr = gumbo_get_attribute(&el.attributes, "class");
			std::string class_value = "";
			if (class_attr == nullptr) {
				continue;
			}
			class_value = class_attr->value;

			if (class_value == "product_price") {
				GumboVector* div_children = &child->v.element.children;

				for (int j = 0; j < div_children->length; ++j) {
					GumboNode* div_child = static_cast<GumboNode*>(div_children->data[j]);
					
					if (div_child->type != GUMBO_NODE_ELEMENT) {
						continue;
					}
					
					std::string class_value = "";
					GumboElement& el2 = div_child->v.element;
					auto class_attr = gumbo_get_attribute(&el2.attributes, "class");
					if (class_attr == nullptr) {
						continue;
					}
					class_value = class_attr->value;
					
					// extracting price
					if (class_value == "price_color") {
						if (div_child->type == GUMBO_NODE_ELEMENT &&
						    div_child->v.element.tag == GUMBO_TAG_P) {
							const GumboVector* price_children = &div_child->v.element.children;
							for (int k = 0; k < price_children->length; ++k) {
								GumboNode* price_child = static_cast<GumboNode*>(price_children->data[k]);
								if (price_child->type == GUMBO_NODE_TEXT) {
									std::string price_str = price_child->v.text.text;
									// removing currency symbol and converting to float
									try {
										price = std::stof(price_str.substr(2));
									}
									catch (const std::invalid_argument& e) {
										price = 0.0f;
									}
									catch (const std::out_of_range& e) {
										price = 0.0f;
									}
									break;
								}
							}
						}
					}
					
					// extracting availability
					else if (class_value == "instock availability") {
						if (div_child->type == GUMBO_NODE_ELEMENT &&
							div_child->v.element.tag == GUMBO_TAG_P) {
							const GumboVector* avail_children = &div_child->v.element.children;
							for (int k = 0; k < avail_children->length; ++k) {
								GumboNode* avail_child = static_cast<GumboNode*>(avail_children->data[k]);
								if (avail_child->type == GUMBO_NODE_TEXT) {
									std::string avail_str = avail_child->v.text.text;
									if (avail_str.find("In stock") != std::string::npos) {
										in_stock = true;
									}
									else {
										in_stock = false;
									}
									break;
								}
							}
						}
					}

				}

			}
		}
	}
	result.push_back(Book(rating, title, price, in_stock));
}

// recursive function to search for books in the html tree
void search_books(GumboNode* node, tbb::concurrent_vector<Book>& result) {
	if (node->type != GUMBO_NODE_ELEMENT) {
		return;
	}
	GumboElement* element = &node->v.element;
	// extracting class attribute
	auto attr = gumbo_get_attribute(&element->attributes, "class");
	if (attr) {
		std::string class_value = attr->value;

		// looking for product_pod class and article tag
		if (class_value == "product_pod" && element->tag == GUMBO_TAG_ARTICLE) {
			parse_book(node, result);
		}
	}

	tbb::task_group g;

	// searching html tree recursively
	for(unsigned int i = 0; i < element->children.length; ++i) {
		GumboNode* child = static_cast<GumboNode*>(element->children.data[i]);
		if (child->type == GUMBO_NODE_ELEMENT) {
			g.run([child, &result]() {
				GumboNode* c = child; // copy of pointer child
				search_books(c, result);
			});
		}
	}

	g.wait();

}

void Parser::parse_page(std::string html, tbb::concurrent_vector<Book>& result) const {
	GumboOutput* output = gumbo_parse(html.c_str());
	search_books(output->root, result);
}

void Parser::parse_range(const tbb::blocked_range<std::size_t>& range, tbb::concurrent_vector<Book>& result) const {
	for (int i = range.begin(); i != range.end(); ++i) {
		parse_page(pages[i], result);
	}
}

void Parser::parse_pages(tbb::concurrent_vector<Book>& result) {
	using clock = std::chrono::steady_clock;
	auto start = clock::now();

	tbb::parallel_for(
		tbb::blocked_range<std::size_t>(0, pages.size()),
		[this, &result](const tbb::blocked_range<std::size_t>& range) {
			parse_range(range, result);
		});
	auto end = clock::now();
	std::chrono::duration<double> elapsed = end - start;
	std::cout << "Parsing pages done in: " << elapsed.count() << " seconds.\n";
}