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

#pragma once

#include <thrust/device_vector.h>
#include <thrust/iterator/zip_iterator.h>

#include <SantaProblem.hpp>

class SantaSolution {
public:
	typedef int                              dtype;
	typedef thrust::device_vector<dtype>     dvector;
	typedef typename dvector::iterator       diter;
	typedef typename dvector::const_iterator const_diter;
	typedef thrust::zip_iterator<thrust::tuple<diter, diter,
	                                           diter, diter,
	                                           diter, diter> > iterator;
	typedef thrust::zip_iterator<thrust::tuple<const_diter,
	                                           const_diter,
	                                           const_diter,
	                                           const_diter,
	                                           const_diter,
	                                           const_diter> > const_iterator;
	typedef typename iterator::reference       reference;
	typedef typename const_iterator::reference const_reference;
private:
	// Note: These are always to be kept in ID order
	dvector m_xminima, m_xmaxima;
	dvector m_yminima, m_ymaxima;
	dvector m_zminima, m_zmaxima;
	// Temporary spaces required for some of the algorithms
	mutable dvector m_tmp_ids;
	mutable dvector m_tmp_sorted;
	mutable dvector m_tmp_indices;
public:
	inline SantaSolution();
	inline SantaSolution(size_t size, dtype val=dtype());
	inline SantaSolution(std::string filename);
	// Loads solution-definition csv file with cols(id,x1,y1,z1,...,x8,y8,z8)
	// Returns no. loaded
	size_t                load(std::string filename,
	                           size_t      count=size_t(-1));
	// Saves solution-definition csv file with cols(id,x1,y1,z1,...,x8,y8,z8)
	void                  save(std::string filename);
	inline size_t         size() const;
	inline void           resize(size_t size, dtype val=dtype());
	inline iterator       begin();
	inline const_iterator begin() const;
	inline iterator       end();
	inline const_iterator end() const;
	inline reference       operator[](size_t i);
	inline const_reference operator[](size_t i) const;
	int validate(const SantaProblem& problem_def,
	             bool                quick=false,
	             int*                size_difference=0,
	             int*                boundary_violations=0,
	             int*                dimension_mismatches=0,
	             int*                collisions=0) const;
	int score() const;
};
SantaSolution::SantaSolution() {}
SantaSolution::SantaSolution(size_t n, dtype val) { resize(n, val); }
SantaSolution::SantaSolution(std::string filename) {
	this->load(filename);
}
size_t SantaSolution::size() const {
	return m_xminima.size();
}
void SantaSolution::resize(size_t n, dtype val) {
	m_xminima.resize(n, val);
	m_xmaxima.resize(n, val);
	m_yminima.resize(n, val);
	m_ymaxima.resize(n, val);
	m_zminima.resize(n, val);
	m_zmaxima.resize(n, val);
	
	m_tmp_ids.resize(n);
	m_tmp_sorted.resize(n);
	m_tmp_indices.resize(n);
}
SantaSolution::iterator SantaSolution::begin() {
	using thrust::make_zip_iterator;
	using thrust::make_tuple;
	return make_zip_iterator(make_tuple(m_xminima.begin(),
	                                    m_xmaxima.begin(),
	                                    m_yminima.begin(),
	                                    m_ymaxima.begin(),
	                                    m_zminima.begin(),
	                                    m_zmaxima.begin()));
}
SantaSolution::const_iterator SantaSolution::begin() const {
	using thrust::make_zip_iterator;
	using thrust::make_tuple;
	return make_zip_iterator(make_tuple(m_xminima.begin(),
	                                    m_xmaxima.begin(),
	                                    m_yminima.begin(),
	                                    m_ymaxima.begin(),
	                                    m_zminima.begin(),
	                                    m_zmaxima.begin()));
}
SantaSolution::iterator SantaSolution::end() {
	return begin() + size();
}
SantaSolution::const_iterator SantaSolution::end() const {
	return begin() + size();
}
SantaSolution::reference       SantaSolution::operator[](size_t i)       { return *(begin() + i); }
SantaSolution::const_reference SantaSolution::operator[](size_t i) const { return *(begin() + i); }
