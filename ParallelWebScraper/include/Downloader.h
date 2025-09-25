#pragma once
#define NOMINMAX
#include <Windows.h>
#include "cpr/cpr.h"
#include <iostream>
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "tbb/concurrent_vector.h"
#include <string>
#include <vector>

class Downloader {
	std::vector<std::string> urls;
	
	// Helper function to fetch a range of URLs3
	// it accepts a blocked_range and a concurrent_vector to store results
	void fetch_range(const tbb::blocked_range<std::size_t>& range, 
					 tbb::concurrent_vector<std::string>& pages,
					 int& fetched) const;

public:
	void operator() (const tbb::blocked_range<std::size_t>& range) const;
	Downloader(const std::vector <std::string> & urls);
	std::vector<std::string> get_pages();
};