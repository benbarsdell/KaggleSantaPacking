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

#include <SantaProblem.hpp>
#include <SantaSolution.hpp>

#include "stopwatch.hpp"

int main(int argc, char* argv[])
{	
#if THRUST_DEVICE_BACKEND == THRUST_DEVICE_BACKEND_CUDA
	cudaSetDevice(0); // Note: This also ensures the device is 'warmed up'
#endif
	
	if( argc <= 2 ) {
		cout << "Usage: " << argv[0] << " presents.csv submissionfile.csv" << endl;
		return -1;
	}
	std::string presents_filename = argv[1];
	std::string solution_filename = argv[2];
	
	int sleigh_size = 1000;
	
	Stopwatch timer;
	timer.start();
	
	SantaProblem  problem(sleigh_size, presents_filename);
	SantaSolution solution(solution_filename);
	
	timer.stop();
	cout << "Load time = " << timer.getTime() << " s" << endl;
	
	timer.reset();
	timer.start();
	
	cout << "Validating solution" << endl;
	//int result = verify_solution(data_cols, soln_cols, sleigh_size);
	int size_difference, boundary_violations,
		dimension_mismatches, collisions;
	bool validated = solution.validate(problem, false,
	                                   &size_difference,
	                                   &boundary_violations,
	                                   &dimension_mismatches,
	                                   &collisions);
	
#if THRUST_DEVICE_BACKEND == THRUST_DEVICE_BACKEND_CUDA
	cudaThreadSynchronize();
#endif
	timer.stop();
	cout << "Validation time = " << timer.getTime() << " s" << endl;
	cout << "                = " << 1. / timer.getTime() << " Hz" << endl;
	
	timer.reset();
	timer.start();
	
	cout << "Evaluating solution" << endl;
	int score = solution.score();
	
#if THRUST_DEVICE_BACKEND == THRUST_DEVICE_BACKEND_CUDA
	cudaThreadSynchronize();
#endif
	timer.stop();
	cout << "Evaluation time = " << timer.getTime() << " s" << endl;
	cout << "                = " << 1. / timer.getTime() << " Hz" << endl;
	
	if( validated ) {
		cout << "Solution VERIFIED" << endl;
		cout << "--------------" << endl;
		cout << "SCORE: " << score << endl;
		cout << "--------------" << endl;
	}
	else {
		if( size_difference != 0 ) {
			cout << "Difference in no. presents: " << size_difference << endl;
		}
		if( boundary_violations != 0 ) {
			cout << "Boundary violations: " << boundary_violations << endl;
		}
		if( dimension_mismatches != 0 ) {
			cout << "Dimension mismatches: " << dimension_mismatches << endl;
		}
		if( collisions != 0 ) {
			cout << "Collisions: " << collisions << endl;
		}
		return -2;
	}
	
	return 0;
}
