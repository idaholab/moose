/* Copyright 2017 Battelle Energy Alliance, LLC

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#ifndef _MDreader_h
#define _MDreader_h

#include <vector>
#include <string>


void readOrderedNDArray(std::string & filename, int & number_of_dimensions, std::vector< std::vector<double> > & discretization_values, std::vector<double> & values);
void readScatteredNDArray(std::string & filename, int & number_of_dimensions,unsigned int & number_of_points, std::vector< std::vector<double> > & point_coordinates, std::vector<double> & values);
//double returnCDFvalue(std::vector<double> coordinates);
//int findIndex(double pivot, std::vector<double> discretizations);
std::vector<double> read1DArray(std::string filename);
double getPointAtCoordinate(std::vector<double> coordinates);
void readMatrix(const std::string filename, unsigned int & rows, unsigned int & columns, std::vector< std::vector<double> > & matrix);
void importMatrixFromTxtFile(const std::string filename_x, std::vector <double>& v, unsigned int& rows, unsigned int& cols);
unsigned int readNumbers(const std::string & s, std::vector <double> & v );


#endif
