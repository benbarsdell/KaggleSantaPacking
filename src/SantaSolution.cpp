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

#include <SantaSolution.hpp>

#include <vector>
#include <fstream>
#include <stdexcept>
#include <string>

#include <thrust/transform.h>
#include <thrust/sort.h>
#include <thrust/inner_product.h>
#include <thrust/sequence.h>
#include <thrust/binary_search.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/iterator/permutation_iterator.h>
#include <thrust/iterator/constant_iterator.h>

typedef SantaSolution::dtype       dtype;
typedef SantaSolution::const_diter const_diter;

using std::min;
using std::max;

template<typename T>
inline __host__ __device__
T max8(T z1, T z2, T z3, T z4, T z5, T z6, T z7, T z8) {
	// Note: Hierarchical method maximises instruction-level parallelism
	//         (at the cost of a couple more registers per thread)
	T max_a, max_b, max_c;
	max_a = max(z1, z2);
	max_b = max(z3, z4);
	max_c = max(max_a, max_b);
	max_a = max(z5, z6);
	max_b = max(z7, z8);
	return max( max_c, max(max_a, max_b) );
}

template<typename T>
inline __host__ __device__
T min8(T z1, T z2, T z3, T z4, T z5, T z6, T z7, T z8) {
	// Note: Hierarchical method maximises instruction-level parallelism
	//         (at the cost of a couple more registers per thread)
	T min_a, min_b, min_c;
	min_a = min(z1, z2);
	min_b = min(z3, z4);
	min_c = min(min_a, min_b);
	min_a = min(z5, z6);
	min_b = min(z7, z8);
	return min( min_c, min(min_a, min_b) );
}

struct abs_diff_functor : public thrust::binary_function<dtype, dtype, dtype> {
	inline __host__ __device__
	dtype operator()(dtype id, dtype i) const {
		//return abs(id - i);
		return abs(max(id, i) - min(id, i));
	}
};

size_t SantaSolution::load(std::string filename, size_t count) {
	std::vector<dtype> ids;
	std::vector<dtype> xminima, xmaxima;
	std::vector<dtype> yminima, ymaxima;
	std::vector<dtype> zminima, zmaxima;
	std::ifstream stream(filename.c_str());
	if( !stream ) {
		throw std::runtime_error("Failed to open " + filename);
	}
	std::string value;
	// Read header line
	std::getline(stream, value);
	size_t i = 0;
	for( ; i<count; ++i ) {
		dtype id, x[8], y[8], z[8];
		// First load the ID
		std::getline(stream, value, ',');
		if( !stream ) {
			break;
		}
		// Note: Converts 1-based to 0-based indexing
		id = atoi(value.c_str()) - 1;
		ids.push_back( id );
		// Now load the x,y,z for each vertex
		for( int v=0; v<7; ++v ) {
			std::getline(stream, value, ','); x[v] = atoi(value.c_str());
			std::getline(stream, value, ','); y[v] = atoi(value.c_str());
			std::getline(stream, value, ','); z[v] = atoi(value.c_str());
		}
		// Note: The last column loaded separately due to newline
		std::getline(stream, value, ',');  x[7] = atoi(value.c_str());
		std::getline(stream, value, ',');  y[7] = atoi(value.c_str());
		std::getline(stream, value, '\n'); z[7] = atoi(value.c_str());
		// Convert 8 vertices to 2 extrema for each coordinate
		xminima.push_back( min8(x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7]) );
		xmaxima.push_back( max8(x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7]) );
		yminima.push_back( min8(y[0],y[1],y[2],y[3],y[4],y[5],y[6],y[7]) );
		ymaxima.push_back( max8(y[0],y[1],y[2],y[3],y[4],y[5],y[6],y[7]) );
		zminima.push_back( min8(z[0],z[1],z[2],z[3],z[4],z[5],z[6],z[7]) );
		zmaxima.push_back( max8(z[0],z[1],z[2],z[3],z[4],z[5],z[6],z[7]) );
	}
	size_t n = xminima.size();
	this->resize(n); // Note: Ensures tmp arrays get allocated
	// Copy loaded data to the device
	m_xminima = xminima; m_xmaxima = xmaxima;
	m_yminima = yminima; m_ymaxima = ymaxima;
	m_zminima = zminima; m_zmaxima = zmaxima;
	m_tmp_ids = ids;
	// Note: This defines a strict ordering by ID value and then by order
	//         in file; technically the actual ID values don't matter.
	thrust::stable_sort_by_key(m_tmp_ids.begin(), m_tmp_ids.end(),
	                           this->begin());
	return i;
}
// Saves solution-definition csv file with cols(id,x1,y1,z1,...,x8,y8,z8)
void SantaSolution::save(std::string filename) {
	std::ofstream stream(filename.c_str());
	if( !stream ) {
		throw std::runtime_error("Failed to open " + filename);
	}
	stream << "id,x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,"
	       <<    "x5,y5,z5,x6,y6,z6,x7,y7,z7,x8,y8,z8"
	       << std::endl;
	for( size_t i=0; i<size(); ++i ) {
		dtype id = i+1; // Note: 1-based indexing for IDs
		stream << id;
		// Iterate over all combinations of (xlo|xhi) (ylo|yhi) (zlo|zhi)
		for( int ix=0; ix<2; ++ix ) {
			dtype x = (ix==0) ? m_xminima[i] : m_xmaxima[i];
			for( int iy=0; iy<2; ++iy ) {
				dtype y = (iy==0) ? m_yminima[i] : m_ymaxima[i];
				for( int iz=0; iz<2; ++iz ) {
					dtype z = (iz==0) ? m_zminima[i] : m_zmaxima[i];
					stream << "," << x
					       << "," << y
					       << "," << z;
				}
			}
		}
		stream << std::endl;
	}
}

// Branchless compare-and-swap
template<typename T>
__host__ __device__
inline void cas(T& a, T& b) {
	T a_ = a;
	a = min(a_, b);
	b = max(a_, b);
}
// Branchless sorting network
template<typename T>
__host__ __device__
inline void sort3(T& a, T& b, T& c) {
	cas(a, b);
	cas(b, c);
	cas(a, b);
}

struct dim_mismatch_functor
	: public thrust::binary_function<void,void,bool> {
	template<typename Tuple1, typename Tuple2>
	inline __host__ __device__
	bool operator()(Tuple1 soln_extrema, Tuple2 prob_dims) const {
		Tuple1& s = soln_extrema;
		Tuple2& p = prob_dims;
		// Note: extrema define *closed* intervals
		dtype s1 = thrust::get<1>(s) - (thrust::get<0>(s)-1);
		dtype s2 = thrust::get<3>(s) - (thrust::get<2>(s)-1);
		dtype s3 = thrust::get<5>(s) - (thrust::get<4>(s)-1);
		dtype p1 = thrust::get<0>(p);
		dtype p2 = thrust::get<1>(p);
		dtype p3 = thrust::get<2>(p);
		
		// Compare after ignoring relative order (to allow arb. permutations)
		sort3(s1, s2, s3);
		sort3(p1, p2, p3);
		return s1 != p1 || s2 != p2 || s3 != p3;
	}
};

template<class BinaryFunction1, class BinaryFunction2>
struct range_reduce_functor
	: public thrust::binary_function<dtype,dtype,
	                                 typename BinaryFunction1::result_type> {
	dtype           init;
	BinaryFunction1 reduce_func;
	BinaryFunction2 transform_func;
	range_reduce_functor(dtype           init_,
	                     BinaryFunction1 reduce_func_,
	                     BinaryFunction2 transform_func_)
		: init(init_), reduce_func(reduce_func_),
		  transform_func(transform_func_) {}
	typedef typename BinaryFunction1::result_type result_type;
	inline __host__ __device__
	result_type operator()(dtype begin, dtype end) const {
		result_type result = init;
		dtype i = begin;
		for( dtype j=i+1; j<end; ++j ) {
			result = reduce_func(result, transform_func(i,j));
		}
		return result;
	}
};
template<class BinaryFunction1, class BinaryFunction2>
range_reduce_functor<BinaryFunction1,BinaryFunction2>
make_range_reduce_functor(dtype init,
                          BinaryFunction1 reduce_func,
                          BinaryFunction2 transform_func) {
	typedef range_reduce_functor<BinaryFunction1,BinaryFunction2> type;
	return type(init, reduce_func, transform_func);
}

struct collision_functor
	: public thrust::binary_function<dtype,dtype,dtype> {
	const dtype* ids;
	const dtype* xminima;
	const dtype* xmaxima;
	const dtype* yminima;
	const dtype* ymaxima;
	collision_functor(const dtype* ids_,
	                  const dtype* xminima_,
	                  const dtype* xmaxima_,
	                  const dtype* yminima_,
	                  const dtype* ymaxima_)
		: ids(ids_),
		  xminima(xminima_), xmaxima(xmaxima_),
		  yminima(yminima_), ymaxima(ymaxima_) {}
	inline __host__ __device__
	dtype operator()(dtype i, dtype j) const {
		// Determine whether presents i and j collide (in the x-y plane)
		dtype ixb = xminima[ids[i]];
		dtype ixe = xmaxima[ids[i]];
		dtype iyb = yminima[ids[i]];
		dtype iye = ymaxima[ids[i]];
		dtype jxb = xminima[ids[j]];
		dtype jxe = xmaxima[ids[j]];
		dtype jyb = yminima[ids[j]];
		dtype jye = ymaxima[ids[j]];
		return !(ixe < jxb || jxe < ixb ||
		         iye < jyb || jye < iyb );
	}
};

int SantaSolution::validate(const SantaProblem& problem,
                            bool quick,
                            int* _size_difference,
                            int* _boundary_violations,
                            int* _dimension_mismatches,
                            int* _collisions) const {
	// Check that sizes match
	int size_difference = int(this->size()) - int(problem.size());
	if( _size_difference ) {
		*_size_difference = size_difference;
	}
	if( quick && _size_difference != 0 ) {
		return false;
	}
	
	// Check sleigh bounds in each dimension
	using thrust::make_constant_iterator;
	dtype sleigh_size = problem.sleigh_size();
	int boundary_violations = 0;
	// sum(xminima <= 0)
	int xmin_violations =
		thrust::inner_product(m_xminima.begin(),
		                      m_xminima.end(),
		                      make_constant_iterator<dtype>(0),
		                      dtype(0),
		                      thrust::plus<dtype>(),
		                      thrust::less_equal<dtype>());
	boundary_violations += xmin_violations;
	if( _boundary_violations ) {
		*_boundary_violations = boundary_violations;
	}
	if( quick && xmin_violations > 0 ) {
		return false;
	}
	// sum(xmaxima > sleigh_size)
	int xmax_violations =
		thrust::inner_product(m_xmaxima.begin(),
		                      m_xmaxima.end(),
		                      make_constant_iterator<dtype>(sleigh_size),
		                      dtype(0),
		                      thrust::plus<dtype>(),
		                      thrust::greater<dtype>());
	boundary_violations += xmax_violations;
	if( _boundary_violations ) {
		*_boundary_violations = boundary_violations;
	}
	if( quick && xmax_violations > 0 ) {
		return false;
	}
	int ymin_violations =
		thrust::inner_product(m_yminima.begin(),
		                      m_yminima.end(),
		                      make_constant_iterator<dtype>(0),
		                      dtype(0),
		                      thrust::plus<dtype>(),
		                      thrust::less_equal<dtype>());
	boundary_violations += ymin_violations;
	if( _boundary_violations ) {
		*_boundary_violations = boundary_violations;
	}
	if( quick && ymin_violations > 0 ) {
		return false;
	}
	int ymax_violations =
		thrust::inner_product(m_ymaxima.begin(),
		                      m_ymaxima.end(),
		                      make_constant_iterator<dtype>(sleigh_size),
		                      dtype(0),
		                      thrust::plus<dtype>(),
		                      thrust::greater<dtype>());
	boundary_violations += ymax_violations;
	if( _boundary_violations ) {
		*_boundary_violations = boundary_violations;
	}
	if( quick && ymax_violations > 0 ) {
		return false;
	}
	int zmin_violations =
		thrust::inner_product(m_zminima.begin(),
		                      m_zminima.end(),
		                      make_constant_iterator<dtype>(0),
		                      dtype(0),
		                      thrust::plus<dtype>(),
		                      thrust::less_equal<dtype>());
	boundary_violations += zmin_violations;
	if( _boundary_violations ) {
		*_boundary_violations = boundary_violations;
	}
	if( quick && zmin_violations > 0 ) {
		return false;
	}
	// Note: No upper bound on z
	
	// Check for dimension mismatches
	// sum(soln_dims != prob_dims)
	int dimension_mismatches = thrust::inner_product(this->begin(),
	                                                 this->end(),
	                                                 problem.begin(),
	                                                 dtype(0),
	                                                 thrust::plus<dtype>(),
	                                                 dim_mismatch_functor());
	if( _dimension_mismatches ) {
		*_dimension_mismatches = dimension_mismatches;
	}
	if( quick && dimension_mismatches > 0 ) {
		return false;
	}
	
	// Check for any collisions between presents
	// This starts by finding all intersections between presents in the z
	//   dimension using an O(NlogN) algorithm, and then directly checks each
	//   z-intersecting pair for a full collision in x and y as well.
	dvector& ids = m_tmp_ids;
	thrust::sequence(ids.begin(), ids.end());
	dvector& zminima = m_tmp_sorted;
	zminima = m_zminima;
	//dvector& range_ends = ids; // Note: We can re-use ids when this is needed
	dvector& range_ends = m_tmp_indices;
	// Sort interval starts, keeping track of ordering. These form the
	//   starts of the collision ranges.
	thrust::stable_sort_by_key(zminima.begin(), zminima.end(), // Keys
	                           ids.begin());                   // Values
	// Find where corresponding interval ends would be inserted into
	//   sorted starts. These form the ends of the collision ranges.
	thrust::upper_bound(zminima.begin(), zminima.end(),
	                    make_permutation_iterator(m_zmaxima.begin(),
	                                              ids.begin()),
	                    make_permutation_iterator(m_zmaxima.begin(),
	                                              ids.end()),
	                    range_ends.begin());
	// For each interval, iterate through all z-collisions and compute
	//   whether the collision also occurred in x and y dims.
	using thrust::raw_pointer_cast;
	collision_functor collision_func(raw_pointer_cast(&ids[0]),
	                                 raw_pointer_cast(&m_xminima[0]),
	                                 raw_pointer_cast(&m_xmaxima[0]),
	                                 raw_pointer_cast(&m_yminima[0]),
	                                 raw_pointer_cast(&m_ymaxima[0]));
	using thrust::make_counting_iterator;
	// sum(count_collisions(index))
	int collisions =
		thrust::inner_product(make_counting_iterator<dtype>(0),
		                      make_counting_iterator<dtype>(size()),
		                      range_ends.begin(),
		                      dtype(0),
		                      thrust::plus<dtype>(),
		                      make_range_reduce_functor(dtype(0),
		                                                thrust::plus<dtype>(),
		                                                collision_func));
	if( _collisions ) {
		*_collisions = collisions;
	}
	if( quick && collisions > 0 ) {
		return false;
	}
	
	return (size_difference      == 0 &&
	        boundary_violations  == 0 &&
	        dimension_mismatches == 0 &&
	        collisions           == 0);
}

int SantaSolution::score() const {
	dtype zmax = *thrust::max_element(m_zmaxima.begin(), m_zmaxima.end());
	
	// Initialise temporary data spaces
	dvector& ids = m_tmp_ids;
	thrust::sequence(ids.begin(), ids.end());
	dvector& zmaxima = m_tmp_sorted;
	zmaxima = m_zmaxima;
	
	// Produce IDs sorted primarily by zmax, secondarily by ID
	thrust::sort_by_key(ids.begin(),
	                    ids.end(),
	                    zmaxima.begin());
	thrust::stable_sort_by_key(zmaxima.rbegin(),
	                           zmaxima.rend(),
	                           ids.rbegin());
	
	// Compute ordering metric
	// sum(abs(IDs - index))
	using thrust::make_counting_iterator;
	dtype sigma = thrust::inner_product(ids.begin(),
	                                    ids.end(),
	                                    make_counting_iterator<dtype>(0),
	                                    dtype(0),
	                                    thrust::plus<dtype>(),
	                                    abs_diff_functor());
	dtype score = 2 * zmax + sigma;
	return score;
}
