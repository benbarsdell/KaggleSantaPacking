/*
* Copyright 2013 Ben Barsdell
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/*
By Ben Barsdell (2013)
benbarsdell@gmail.com
*/

#include <SantaProblem.hpp>

#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>

#include <thrust/sort.h>

size_t SantaProblem::load(std::string filename, size_t count) {
	std::vector<dtype> ids, widths, heights, depths;
	std::ifstream stream(filename.c_str());
	if( !stream ) {
		throw std::runtime_error("Failed to open " + filename);
	}
	std::string value;
	// Read header line
	std::getline(stream, value);
	size_t i = 0;
	for( ; i<count; ++i ) {
		// First load the ID
		std::getline(stream, value, ',');
		if( !stream ) {
			break;
		}
		// Note: Converts 1-based to 0-based indexing
		dtype id = atoi(value.c_str()) - 1;
		ids.push_back(id);
		// Now load the x,y,z for each vertex
		std::getline(stream, value, ',');  dtype dim1 = atoi(value.c_str());
		std::getline(stream, value, ',');  dtype dim2 = atoi(value.c_str());
		std::getline(stream, value, '\n'); dtype dim3 = atoi(value.c_str());
		widths.push_back(dim1);
		heights.push_back(dim2);
		depths.push_back(dim3);
	}
	// Copy to the device
	m_widths  = widths;
	m_heights = heights;
	m_depths  = depths;
	dvector tmp_ids = ids;
	// Sort dimensions by ID
	// Note: This defines a strict ordering by ID value and then by order
	//         in file; technically the actual ID values don't matter.
	thrust::stable_sort_by_key(tmp_ids.begin(), tmp_ids.end(),
	                           this->begin());
	return i;
}
