//
//  Multi-dimensional array reader
//

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>

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

void readOrderedNDarray(std::string & filename, int & numberOfDimensions, std::vector< std::vector<double> > & discretizationValues, std::vector<double> & values){
    //FILE* pFile = fopen("filename", "rb");
    // file structure
    // - int number of dimensions                                   1
    // - [int] number of discretization points for each dimension   N
    // - [[double]] discretizations points for each dimension       sum (N_i*d_i)
    // - [[double]] CDF value for each discretization point

    // location of first CDF value point = 1 + N + sum (N_i*d_i)

 std::cerr << "readOrderedNDarray" << std::endl;

 std::vector<double> data;
 data = read1Darray(filename);

 int startingPoint = 0;
 if (checkIfdoubleIsInt(data[startingPoint]))
         numberOfDimensions = (int)data[startingPoint];
 else
         throwError("readOrderedNDarray: error in" << filename << "; number of dimensions must be integer");
 //std::cerr << "numberOfDimensions: " << numberOfDimensions << std::endl;

    std::vector<int> discretizations (numberOfDimensions);

    startingPoint++;
    //std::cerr << "discretizations" << std::endl;
    for (int i=0; i<numberOfDimensions; i++){
        if (checkIfdoubleIsInt(data[startingPoint]))
                discretizations[i] = (int)data[startingPoint];
        else
                 throwError("readOrderedNDarray: error in" << filename << "; number of discretizaions must be integer");
        //std::cerr << "discretizations["<< i << "]: " << discretizations[i] << std::endl;
        startingPoint++;
    }

    for (int i=0; i<numberOfDimensions; i++){
     //std::cerr << "Dimension: " << i << std::endl;
        std::vector<double> tempDiscretization;
        for (int j=0; j<discretizations[i]; j++){
            tempDiscretization.push_back(data[startingPoint]);
            //std::cerr << data[startingPoint] << "," ;
            startingPoint++;
        }
        //std::cerr << " - " <<  tempDiscretization.size() << std::endl;
        discretizationValues.push_back(tempDiscretization);
    }

    int numberofValues = 1;
    for (int i=0; i<numberOfDimensions; i++)
     numberofValues *= discretizations[i];

    for (int n=0; n<numberofValues; n++){
     values.push_back(data[startingPoint]);
     startingPoint++;
    }

    std::cerr << "Completed readOrderedNDarray" << std::endl;
}

void readScatteredNDarray(std::string & filename, int & numberOfDimensions, unsigned int & numberOfPoints, std::vector< std::vector<double> > & pointcoordinates, std::vector<double> & values){
 // int - number of dimensions d
 // int - number of points n
 // [[double]]  - point coordinates
 // [double]  - point values

 std::vector<double> data;
 data = read1Darray(filename);

 if (checkIfdoubleIsInt(data[0]))
         numberOfDimensions = (int)data[0];
 else
         throwError("readScatteredNDarray: error in" << filename << "; number of dimensions must be integer");
 numberOfDimensions = (int)data[0];

 if (checkIfdoubleIsInt(data[1]))
         numberOfPoints = (int)data[1];
 else
         throwError("readScatteredNDarray: error in" << filename << "; number of points must be integer");

 int startingPoint = 2;

 for (unsigned int n=0; n<numberOfPoints; n++){
  std::vector<double> tempVector (numberOfDimensions);
  for (int nDim=0; nDim<numberOfDimensions; nDim++){
   tempVector[nDim] = data[startingPoint];
   startingPoint += 1;
  }
  pointcoordinates.push_back(tempVector);
 }

 //startingPoint += numberOfPoints * numberOfDimensions;

 for (unsigned int n=0; n<numberOfPoints; n++){
  values.push_back(data[startingPoint]);
  startingPoint += 1;
 }

 std::cerr << "Completed readScatteredNDarray" << std::endl;

 //check that data info is consistent
 if (values.size() != pointcoordinates.size())
  throwError("Data contained in " << filename << " is not complete: point coordinates and values do not match.");
 if (numberOfPoints != pointcoordinates.size())
  throwError("Data contained in " << filename << " is not complete: expected number of points and point coordinates do not match.");

//for (int n=0; n<numberOfPoints; n++){
//       std::cout<< pointcoordinates[n][0] << " ; " << pointcoordinates[n][1] << " : " << values[n] << std::endl;
// }

}

void readMatrix(const std::string filename, unsigned int & rows, unsigned int & columns, std::vector< std::vector<double> > & matrix){
 // Data format: row1
 //              row2
 //              row3

    std::vector <double> v;

    import_matrix_from_txt_file(filename,v,rows,columns);

    for (unsigned int i=0;i<rows;i++){
     std::vector<double> temp;
        for (unsigned int j=0;j<columns;j++)
         temp.push_back(v[i*columns+j]);
        matrix.push_back(temp);
    }
}


unsigned int ReadNumbers(const std::string & s, std::vector <double> & v ) {
    std::istringstream is( s );
    double n;
    while( is >> n )
        v.push_back( n );

    return v.size();
}




void import_matrix_from_txt_file(const std::string filename_X, std::vector <double>& v, unsigned int& rows, unsigned int& cols){
    std::ifstream file_X;
    std::string line;

    file_X.open(filename_X.c_str());
    if (file_X.is_open())
    {
        unsigned int i=0;
        getline(file_X, line);

        cols =ReadNumbers( line, v );
        //std::cout << "cols:" << cols << std::endl;

        for ( i=1;i<32767;i++){
            if ( getline(file_X, line) == 0 ) break;
            ReadNumbers( line, v );
        }

        rows=i;
        //std::cout << "rows :" << rows << std::endl;
        if(rows >32766) std::cout<< "N must be smaller than MAX_INT";

        file_X.close();
    }
    else{
     throwError("Failure to open file:" << filename_X);
    }
//    for (int i=0;i<rows;i++){
//        for (int j=0;j<cols;j++)
//          std::cout << v[i*cols+j] << "\t" ;
//        std::cout << std::endl;
//    }
}



std::vector<double> read1Darray(std::string filename){
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
