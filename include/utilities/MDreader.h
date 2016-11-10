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
