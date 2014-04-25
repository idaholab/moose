#include <vector>
#include <string>

using namespace std;

#ifndef _MDreader_h
#define _MDreader_h

void readOrderedNDarray(std::string & filename, int & numberOfDimensions, std::vector< std::vector<double> > & discretizationValues, vector<double> & values);
void readScatteredNDarray(std::string & filename, int & numberOfDimensions, int & numberOfPoints, std::vector< std::vector<double> > & pointcoordinates, std::vector<double> & values);
//double returnCDFvalue(std::vector<double> coordinates);
//int findIndex(double pivot, std::vector<double> discretizations);
std::vector<double> read1Darray(std::string filename);
double getPointAtCoordinate(std::vector<double> coordinates);
void readMatrix(std::string filename, int & rows, int & columns, std::vector< std::vector<double> > & matrix);
void import_matrix_from_txt_file(std::string filename_X, vector <double>& v, int& rows, int& cols);
int ReadNumbers( const string & s, vector <double> & v );

#endif
