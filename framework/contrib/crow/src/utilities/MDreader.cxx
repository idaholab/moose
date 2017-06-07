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
//
//  Multi-dimensional array reader
//

#include <cstdio>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <iso646.h>

#include "MDreader.h"

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

bool checkIfdoubleIsInt(double value){
        double intPart;
        double decPart = std::modf(value, &intPart);

        if (decPart == 0.0)
                return true;
        else
                return false;
}

void readOrderedNDArray(std::string & filename, int & number_of_dimensions, std::vector< std::vector<double> > & discretization_values, std::vector<double> & values){
    //FILE* pFile = fopen("filename", "rb");
    // file structure
    // - int number of dimensions                                   1
    // - [int] number of discretization points for each dimension   N
    // - [[double]] discretizations points for each dimension       sum (N_i*d_i)
    // - [[double]] CDF value for each discretization point

    // location of first CDF value point = 1 + N + sum (N_i*d_i)

 std::cerr << "readOrderedNDArray" << std::endl;

 std::vector<double> data;
 data = read1DArray(filename);

 int startingPoint = 0;
 if (checkIfdoubleIsInt(data[startingPoint]))
         number_of_dimensions = (int)data[startingPoint];
 else
         throwError("readOrderedNDArray: error in" << filename << "; number of dimensions must be integer");
 //std::cerr << "number_of_dimensions: " << number_of_dimensions << std::endl;

    std::vector<int> discretizations (number_of_dimensions);

    startingPoint++;
    //std::cerr << "discretizations" << std::endl;
    for (int i=0; i<number_of_dimensions; i++){
        if (checkIfdoubleIsInt(data[startingPoint]))
                discretizations[i] = (int)data[startingPoint];
        else
                 throwError("readOrderedNDArray: error in" << filename << "; number of discretizaions must be integer");
        //std::cerr << "discretizations["<< i << "]: " << discretizations[i] << std::endl;
        startingPoint++;
    }

    for (int i=0; i<number_of_dimensions; i++){
     //std::cerr << "Dimension: " << i << std::endl;
        std::vector<double> tempDiscretization;
        for (int j=0; j<discretizations[i]; j++){
            tempDiscretization.push_back(data[startingPoint]);
            //std::cerr << data[startingPoint] << "," ;
            startingPoint++;
        }
        //std::cerr << " - " <<  tempDiscretization.size() << std::endl;
        discretization_values.push_back(tempDiscretization);
    }

    int numberofValues = 1;
    for (int i=0; i<number_of_dimensions; i++)
     numberofValues *= discretizations[i];

    for (int n=0; n<numberofValues; n++){
     values.push_back(data[startingPoint]);
     startingPoint++;
    }

    std::cerr << "Completed readOrderedNDArray" << std::endl;
}

void readScatteredNDArray(std::string & filename, int & number_of_dimensions, unsigned int & number_of_points, std::vector< std::vector<double> > & point_coordinates, std::vector<double> & values){
 // int - number of dimensions d
 // int - number of points n
 // [[double]]  - point coordinates
 // [double]  - point values

 std::vector<double> data;
 data = read1DArray(filename);

 if (checkIfdoubleIsInt(data[0]))
         number_of_dimensions = (int)data[0];
 else
         throwError("readScatteredNDArray: error in" << filename << "; number of dimensions must be integer");
 number_of_dimensions = (int)data[0];

 if (checkIfdoubleIsInt(data[1]))
         number_of_points = (int)data[1];
 else
         throwError("readScatteredNDArray: error in" << filename << "; number of points must be integer");

 int startingPoint = 2;

 for (unsigned int n=0; n<number_of_points; n++){
  std::vector<double> tempVector (number_of_dimensions);
  for (int nDim=0; nDim<number_of_dimensions; nDim++){
   tempVector[nDim] = data[startingPoint];
   startingPoint += 1;
  }
  point_coordinates.push_back(tempVector);
 }

 //startingPoint += number_of_points * number_of_dimensions;

 for (unsigned int n=0; n<number_of_points; n++){
  values.push_back(data[startingPoint]);
  startingPoint += 1;
 }

 std::cerr << "Completed readScatteredNDArray" << std::endl;

 //check that data info is consistent
 if (values.size() != point_coordinates.size())
  throwError("Data contained in " << filename << " is not complete: point coordinates and values do not match.");
 if (number_of_points != point_coordinates.size())
  throwError("Data contained in " << filename << " is not complete: expected number of points and point coordinates do not match.");

//for (int n=0; n<number_of_points; n++){
//       std::cout<< point_coordinates[n][0] << " ; " << point_coordinates[n][1] << " : " << values[n] << std::endl;
// }

}

void readMatrix(const std::string filename, unsigned int & rows, unsigned int & columns, std::vector< std::vector<double> > & matrix){
 // Data format: row1
 //              row2
 //              row3

    std::vector <double> v;

    importMatrixFromTxtFile(filename,v,rows,columns);

    for (unsigned int i=0;i<rows;i++){
     std::vector<double> temp;
        for (unsigned int j=0;j<columns;j++)
         temp.push_back(v[i*columns+j]);
        matrix.push_back(temp);
    }
}


unsigned int readNumbers(const std::string & s, std::vector <double> & v ) {
    std::istringstream is( s );
    double n;
    while( is >> n )
        v.push_back( n );

    return v.size();
}




void importMatrixFromTxtFile(const std::string filename_x, std::vector <double>& v, unsigned int& rows, unsigned int& cols){
    std::ifstream file_X;
    std::string line;

    file_X.open(filename_x.c_str());
    if (file_X.is_open())
    {
        unsigned int i=0;
        getline(file_X, line);

        cols =readNumbers( line, v );
        //std::cout << "cols:" << cols << std::endl;

        for ( i=1;i<32767;i++){
            if (!getline(file_X, line) ) break;
            readNumbers( line, v );
        }

        rows=i;
        //std::cout << "rows :" << rows << std::endl;
        if(rows >32766) std::cout<< "N must be smaller than MAX_INT";

        file_X.close();
    }
    else{
     throwError("Failure to open file:" << filename_x);
    }
//    for (int i=0;i<rows;i++){
//        for (int j=0;j<cols;j++)
//          std::cout << v[i*cols+j] << "\t" ;
//        std::cout << std::endl;
//    }
}



std::vector<double> read1DArray(std::string filename){
 //numbers are separated by newlines

        std::vector<double> data;
        std::ifstream in(filename.c_str());
 if(not in)
  throwError("The filename " << filename << " does not exist!!!!");
 if(in)
  std::cerr << "The file " << filename << " was successfully found" << std::endl;

 double number;
    while (in >> number){
     data.push_back(number);
    }
 in.close();

 std::cerr << "Done reading file" << std::endl;

 return data;
}



//double returnCDFvalue(vector<double> coordinates){
// double value;
// std::vector<double> indices;
//
// for (i=0; i<N_dimensions; i++)
//  if (coordinates[i] > discretizations[i][discretizations.size()])
//   mooseError("Error in CDF evaluation for custom distribution");
//  else
//   indices[i] = findIndex(coordinates[i], discretizations[i]);
//
// return value;
//}
//
//int findIndex(double pivot, vector<double> discretizations){
// // given an arrays of values and a pivot point, this function returns
// int index=discretizations.size();
//
// for (i=1; i<index; i++){
//  if ((pivot<discretizations[i]) && (pivot>discretizations[i-1]))
//   index= i-1;
// }
//
// return index;
//}
