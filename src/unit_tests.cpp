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

#include <iostream>
using std::cout;
using std::endl;
#include <cstdio>
#include <string>
#include <fstream>
#include <cassert>

#include <SantaProblem.hpp>
#include <SantaSolution.hpp>

void test_SantaProblem() {
	cout << "Generating test problem data" << endl;
	std::string presents_filename = tmpnam(0);
	//std::string presents_filename = "test_presents.csv";
	std::ofstream presents_stream(presents_filename.c_str());
	
	int ids[]     = {5,   1,   2,   4,   3};
	int widths[]  = {101, 58,  14,  897, 1};
	int heights[] = {11,  17,  340, 91,  1};
	int depths[]  = {2,   90,  3,   702, 1};
	presents_stream << "id,width,height,depth" << endl;
	for( int i=0; i<5; ++i ) {
		presents_stream << ids[i]
		                << "," << widths[i]
		                << "," << heights[i]
		                << "," << depths[i]
		                << std::endl;
	}
	presents_stream.close();
	
	cout << "Testing class SantaProblem" << endl;
    int sleigh_size = 999;
    SantaProblem problem(sleigh_size, presents_filename);
	assert( problem.sleigh_size() == sleigh_size );
	problem.set_sleigh_size(sleigh_size+1);
	assert( problem.sleigh_size() == sleigh_size+1 );
	assert( problem.size() == 5 );
	// Note: Ordered by ID, not original data order
	assert( *problem.begin() == thrust::make_tuple(widths[1],
	                                               heights[1],
	                                               depths[1]) );
	assert( problem.end() - problem.begin() == 5 );
	problem.begin()[0] = thrust::make_tuple(1, 2, 3);
	problem.begin()[4] = thrust::make_tuple(101, 11, 2);
	assert( *problem.widths_begin()  == 1 );
	assert( *problem.heights_begin() == 2 );
	assert( *problem.depths_begin()  == 3 );
	cout << "  Tests PASSED" << endl;
	remove(presents_filename.c_str());
}

void test_SantaSolution() {
	SantaProblem problem(1000, 3);
	problem[0] = thrust::make_tuple(5,   10,  20);
	problem[1] = thrust::make_tuple(10,  20,  30);
	problem[2] = thrust::make_tuple(100, 100, 100);
	
	cout << "Generating test solution data" << endl;
	std::string solution_filename = tmpnam(0);
	//std::string solution_filename = "test_solution.csv";
	std::ofstream solution_stream(solution_filename.c_str());
	int ids[]     = {2,   1,  3};
	int xminima[] = {6,   1, 21};
	int xmaxima[] = {15,  5,120};
	int yminima[] = {1,   1,  1};
	int ymaxima[] = {20, 10,100};
	int zminima[] = {1,   1,  1};
	int zmaxima[] = {30, 20,100};
	solution_stream << "id,x1,y1,z1,x2,y2,z2,x3,y3,z3,x4,y4,z4,"
	                <<    "x5,y5,z5,x6,y6,z6,x7,y7,z7,x8,y8,z8"
	                << endl;
	for( int i=0; i<3; ++i ) {
		solution_stream << ids[i]
		                << "," << xminima[i] << "," << yminima[i] << "," << zminima[i]
		                << "," << xmaxima[i] << "," << yminima[i] << "," << zminima[i]
		                << "," << xminima[i] << "," << ymaxima[i] << "," << zminima[i]
		                << "," << xmaxima[i] << "," << ymaxima[i] << "," << zminima[i]
		                << "," << xminima[i] << "," << yminima[i] << "," << zmaxima[i]
		                << "," << xmaxima[i] << "," << yminima[i] << "," << zmaxima[i]
		                << "," << xminima[i] << "," << ymaxima[i] << "," << zmaxima[i]
		                << "," << xmaxima[i] << "," << ymaxima[i] << "," << zmaxima[i]
		                << std::endl;
	}
	solution_stream.close();
	
	cout << "Testing class SantaSolution" << endl;
	SantaSolution solution(3);
	solution[0] = thrust::make_tuple(1, 2, 3, 4, 5, 6);
	assert( *solution.begin() == thrust::make_tuple(1, 2, 3, 4, 5, 6) );
	solution.load(solution_filename);
	assert( solution.size() == 3 );
	solution.save(solution_filename);
	assert( solution.load(solution_filename) == 3 );
	assert( solution.end() - solution.begin() == 3 );
	// Note: Ordered by ID, not original data order
	assert( *solution.begin() == thrust::make_tuple(xminima[1],xmaxima[1],
	                                                yminima[1],ymaxima[1],
	                                                zminima[1],zmaxima[1]) );
	
	int size_difference, boundary_violations,
		dimension_mismatches, collisions;
	int valid = solution.validate(problem,
	                              false,
	                              &size_difference,
	                              &boundary_violations,
	                              &dimension_mismatches,
	                              &collisions);
	assert( size_difference == 0 );
	assert( boundary_violations == 0 );
	assert( dimension_mismatches == 0 );
	assert( collisions == 0 );
	assert( valid );
	
	// Modify to induce all types of invalidations
	solution[1] = thrust::make_tuple(0, 9, 1, 2, 1, 30);
	solution.resize(solution.size()-1);
	valid = solution.validate(problem,
	                              false,
	                              &size_difference,
	                              &boundary_violations,
	                              &dimension_mismatches,
	                              &collisions);
	assert( size_difference == -1 );
	assert( boundary_violations == 1 );
	assert( dimension_mismatches == 1 );
	assert( collisions == 1 );
	assert( !valid );
	
	assert( solution.score() == 62 );
	
	cout << "  Tests PASSED" << endl;
	remove(solution_filename.c_str());
}

int main(int argc, char* argv[])
{
	test_SantaProblem();
	test_SantaSolution();
	
	cout << "----------------" << endl;
	cout << "All tests PASSED" << endl;
	cout << "----------------" << endl;
	
	return 0;
}
