#include "Book.h"
#include "Downloader.h"
#include "Parser.h"
#include "Analyzer.h"
#include <iostream>
#include <fstream>

const std::string INPUT_FILE = "input/input.csv";
const std::string OUTPUT_DIR = "result";

std::vector<std::string> urls;
tbb::concurrent_vector<Book> books;

void input_urls() {
    std::ifstream file(INPUT_FILE);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << INPUT_FILE << std::endl;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            urls.push_back(line);
        }
    }
    file.close();
}

int main() {
    input_urls();

	Downloader d(urls);

	std::vector<std::string> pages = d.get_pages();
    
    std::cout << "Fetching pages done\n";

    Parser parser(pages);
    parser.parse_pages(books);

    Analyzer analyzer(books, OUTPUT_DIR);
    analyzer.analyze_books();

	std::cout << "Analyzing done\n";

    analyzer.write_to_file();
	std::cout << "Results written to file\n";

	return 0;
}