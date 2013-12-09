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

class SantaProblem {
public:
	typedef int                              dtype;
	typedef thrust::device_vector<dtype>     dvector;
	typedef typename dvector::iterator       diter;
	typedef typename dvector::const_iterator const_diter;
	typedef thrust::zip_iterator<thrust::tuple<diter,
	                                           diter,
	                                           diter> > iterator;
	typedef thrust::zip_iterator<thrust::tuple<const_diter,
	                                           const_diter,
	                                           const_diter> > const_iterator;
	typedef typename iterator::reference       reference;
	typedef typename const_iterator::reference const_reference;
private:
	dvector m_widths, m_heights, m_depths;
	dtype   m_sleigh_size;
public:
	inline SantaProblem();
	inline SantaProblem(dtype  sleigh_size,
	                    size_t size,
	                    dtype  val=dtype());
	inline SantaProblem(dtype       sleigh_size,
	                    std::string filename);
	// Loads problem-definition csv file with cols (id,dim1,dim2,dim3)
	// Returns no. loaded
	size_t                load(std::string filename,
	                           size_t      count=size_t(-1));
	inline dtype          sleigh_size() const;
	inline void           set_sleigh_size(dtype size);
	inline size_t         size() const;
	inline void           resize(size_t n, dtype val=dtype());
	inline iterator       begin();
	inline const_iterator begin() const;
	inline iterator       end();
	inline const_iterator end() const;
	inline reference       operator[](size_t i);
	inline const_reference operator[](size_t i) const;
	inline const_diter    widths_begin()  const;
	inline const_diter    heights_begin() const;
	inline const_diter    depths_begin()  const;
};
SantaProblem::SantaProblem() {}
SantaProblem::SantaProblem(dtype sleigh_size, size_t size, dtype val) {
	set_sleigh_size(sleigh_size);
	resize(size, val);
}
SantaProblem::SantaProblem(dtype sleigh_size, 
                           std::string filename) {
	set_sleigh_size(sleigh_size);
	load(filename);
}
SantaProblem::dtype SantaProblem::sleigh_size() const { return m_sleigh_size; }
void     SantaProblem::set_sleigh_size(dtype size) { m_sleigh_size = size; }
size_t   SantaProblem::size() const { return m_widths.size(); }
void     SantaProblem::resize(size_t n, dtype val) {
	m_widths.resize(n, val);
	m_heights.resize(n, val);
	m_depths.resize(n, val);
}
SantaProblem::iterator SantaProblem::begin() {
	using thrust::make_zip_iterator;
	using thrust::make_tuple;
	return make_zip_iterator(make_tuple(m_widths.begin(),
	                                    m_heights.begin(),
	                                    m_depths.begin()));
}
SantaProblem::const_iterator SantaProblem::begin() const {
	using thrust::make_zip_iterator;
	using thrust::make_tuple;
	return make_zip_iterator(make_tuple(m_widths.begin(),
	                                    m_heights.begin(),
	                                    m_depths.begin()));
}
SantaProblem::iterator SantaProblem::end() {
	return begin() + size();
}
SantaProblem::const_iterator SantaProblem::end() const {
	return begin() + size();
}
SantaProblem::reference       SantaProblem::operator[](size_t i)       { return *(begin() + i); }
SantaProblem::const_reference SantaProblem::operator[](size_t i) const { return *(begin() + i); }
SantaProblem::const_diter SantaProblem::widths_begin()  const { return m_widths.begin(); }
SantaProblem::const_diter SantaProblem::heights_begin() const { return m_heights.begin(); }
SantaProblem::const_diter SantaProblem::depths_begin()  const { return m_depths.begin(); }
