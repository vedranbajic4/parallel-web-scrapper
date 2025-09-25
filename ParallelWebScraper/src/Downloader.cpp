#include "Downloader.h"

Downloader::Downloader(const std::vector<std::string>& urls) : urls(urls) {
}

cpr::Response fetch_with_retry(const std::string& url, int max_retries = 5, int base_delay_ms = 300) {
	for (int attempt = 1; attempt <= max_retries; ++attempt) {
		auto response = cpr::Get(cpr::Url{ url }, cpr::Timeout{ 5000 }); // 5s timeout

		if (response.status_code == 200) {
			return response; // success
		}

		std::cerr << "Fetch failed (" << response.status_code
			<< ") attempt " << attempt << " of " << max_retries << "\n";

		if (attempt < max_retries) {
			int delay = base_delay_ms * (1 << (attempt - 1)); // exponential backoff: 500ms, 1s, 2s...
			std::cerr << "Retrying in " << delay << " ms...\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		}
	}

	return cpr::Response{}; // empty response if failed all attempts
}

void Downloader::fetch_range(const tbb::blocked_range<std::size_t>& range,
							tbb::concurrent_vector<std::string>& pages,
							int& fetched) const {
	
	for (int i = range.begin(); i != range.end(); ++i){
		cpr::Response r = fetch_with_retry(urls[i]);
		if (r.status_code == 200) {
			pages.push_back(r.text);
			fetched++;
		}
		else {
			std::cout << "Nisam fetchovao : " << urls[i] << " Status code: " << r.status_code << std::endl;
		}
	}
}

std::vector<std::string> Downloader::get_pages() {
	tbb::concurrent_vector<std::string> pages;
	int fetched = 0;
	using clock = std::chrono::steady_clock;
	auto start = clock::now();

	tbb::parallel_for(
		tbb::blocked_range<std::size_t>(0, urls.size()),
		[this, &pages, &fetched](const tbb::blocked_range<std::size_t>& range) {
			fetch_range(range, pages, fetched);
		});

	std::vector<std::string> ret(pages.begin(), pages.end());
	std::cout << "Fetched pages: " << fetched << std::endl;
	std::cout << "Number of pages that were unsuccesfuly fetched: " << urls.size() - fetched << std::endl << std::endl;
	
	auto end = clock::now();
	std::chrono::duration<double> elapsed = end - start;

	double throughput = fetched / elapsed.count(); // stranica/sec
	std::cout << "Time taken: " << elapsed.count() << " seconds\n";
	std::cout << "Throughput: " << throughput << " pages/second\n\n";

	return ret;
}