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
#include "ND_Interpolation_Functions.h"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <cmath>
#include <ctime>


#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }


double NDInterpolation::interpolateAt(std::vector<double> /* point_coordinate */){
  throw ("Error in interpolateAt: NOT IMPLEMENTED!!!!");
  return -1;
}

//std::vector<double> NDInterpolation::NDinverseFunction(double f_min, double f_max){
//  throw ("Error in NDinverseFunction: NOT IMPLEMENTED!!!!");
//  std::vector<double> a;
//  a.push_back(-1);
//  return a;
//}


double NDInterpolation::getGradientAt(std::vector<double> /* point_coordinate */){
  throw ("Error in getGradientAt: NOT IMPLEMENTED!!!!");
  return -1;
}

void NDInterpolation::fit(std::vector< std::vector<double> > /* coordinates */, std::vector<double> /* values */){
  throw ("Error in fit: NOT IMPLEMENTED!!!!");
}

void NDInterpolation::updateRNGParameters(double tolerance, double initial_divisions){
        _tolerance = tolerance;
        _initial_divisions = (int)initial_divisions;
}


NDInterpolation::NDInterpolation()
{
          _tolerance = 0.1;
          _initial_divisions = 10;
          _random_double = new RandomClass();
          _random_double->seed(0);
}
NDInterpolation::~NDInterpolation()
{
}

bool checkUpperBound(double upper_bound, std::vector<double> values){
 bool check = true;

 for (unsigned int i=0; i<values.size(); i++){
  if (values.at(i) > upper_bound){
   check = check && false;
   //std::cout<< "values.at(i) : " << values.at(i) << std::endl;
  }
  else
   check = check && true;
 }

 return check;
}


bool checkLowerBound(double lower_bound, std::vector<double> values){
 bool check = true;

 for (unsigned int i=0; i<values.size(); i++){
  if (values.at(i) < lower_bound)
   check = check && false;
  else
   check = check && true;
 }

 return check;
}

bool InverseDistanceWeighting::checkUB(double upper_bound){
 return checkUpperBound(upper_bound, _values);
}

bool InverseDistanceWeighting::checkLB(double lower_bound){
 return checkLowerBound(lower_bound, _values);
}

bool NDSpline::checkUB(double upper_bound){
 return checkUpperBound(upper_bound, _values);
}

bool NDSpline::checkLB(double lower_bound){
 return checkLowerBound(lower_bound, _values);
}

double NDInterpolation::minkowskiDistance (std::vector<double> point1, std::vector<double> point2, double p){
 double distance;

 if (point1.size() == point1.size()){
   double pDistance = 0.0;
   for (unsigned int i=0; i<point1.size(); i++){
         // use std::abs() and DON'T use abs()!!!!!!!!!!!!!!!!
     pDistance =  pDistance + pow(std::abs(point1.at(i)-point2.at(i)),p);
   }
   distance = pow(pDistance, 1.0/p);
 }else
    throw ("Error in Minkowski distance: points having different dimensions");

 return distance;
}


double NDInterpolation::vectorNorm(std::vector<double> point, double p){
 double norm = 0;

 for (unsigned int i=1; i<point.size(); i++){
  norm = pow(point[i],p);
 }

 norm = pow(norm, 1/p);

 return norm;
}

//std::vector<double> InverseDistanceWeighting::NDinverseFunction(double f_min, double f_max){
// // iterative procedure of linear interpolation to determine a nextPoint between firstPoint and secondPoint until CDF(nextPoint) lies between f_min and f_max
//
// double referenceCDF = (f_max-f_min)/2;
//
// boost::random::mt19937 rng;
// rng.seed(time(NULL));
// double range = rng.max() - rng.min();
//
//    // check input values inconsistencies
//
//    if (f_min > f_max)
//        throwError("ND RNG function: invalid param (f_min > f_max)");
//
//    std::vector<double> min_values (_dimensions);
//    std::vector<double> max_values (_dimensions);
//
//    // Find extreme points
//
//    for (int nDim=0; nDim<_dimensions; nDim++){
//        for (int nPoints=0; nPoints<_number_of_points; nPoints++){
//            if (_point_coordinates[nPoints][nDim] < min_values[nDim])
//                min_values[nDim] = _point_coordinates[nPoints][nDim];
//            if (_point_coordinates[nPoints][nDim] > max_values[nDim])
//                max_values[nDim] = _point_coordinates[nPoints][nDim];
//        }
//    }
//
//    std::vector<double> firstPoint  (_dimensions);
//    std::vector<double> secondPoint (_dimensions);
//    std::vector<double> nextPoint   (_dimensions);
//
//
//    // Randomly pick the fistPoint
//    for (int nDim=0; nDim<_dimensions; nDim++){
//     double temp = (rng()-rng.min())/range;;
//        firstPoint[nDim] = min_values[nDim] + (max_values[nDim]-min_values[nDim]) * temp;
//    }
//
//    int min_index, max_index = 0;
//
//    double min_value , max_value = 0.0;
//    for (int n=0; n<_number_of_points; n++){
//        if (_values[n] < min_value){
//         min_index = n;
//            min_value = _values[min_index];
//        }
//        if (_values[n] > max_value){
//         max_index = n;
//            max_value = _values[max_index];
//        }
//    }
//
//    bool convergence;
//
//    double valueFirstPoint = interpolateAt(firstPoint);
//    double valueSecondPoint;
//    double valueNextPoint;
//
//    if (valueFirstPoint < f_min && valueFirstPoint < f_max){
//        secondPoint = max_values;
//        valueSecondPoint = 1.0;
//        convergence = false;
//    } else if (interpolateAt(firstPoint) > f_min && interpolateAt(firstPoint) > f_max){
//        secondPoint = min_values;
//        valueSecondPoint = 0.0;
//        convergence = false;
//    } else {
//     nextPoint = firstPoint;
//        convergence = true;
//    }
//
//    while (convergence){
//        // Determine next point
//
//        for (int nDim=0; nDim<_dimensions; nDim++)
//            nextPoint[nDim] = (firstPoint[nDim]-secondPoint[nDim])*(valueFirstPoint-valueSecondPoint)/(referenceCDF-valueSecondPoint)+secondPoint[nDim];
//        valueNextPoint = interpolateAt(nextPoint);
//
//        if (referenceCDF>valueFirstPoint && referenceCDF<valueSecondPoint){
//            firstPoint = nextPoint;
//            // secondPoint remains
//        } else if (referenceCDF>valueSecondPoint && referenceCDF<valueFirstPoint) {
//            // firstPoint remains;
//            secondPoint = nextPoint;
//        } else
//            throwError("Computational error in NDinverse");
//
//        // test convergence
//        if (valueNextPoint > f_min && valueNextPoint < f_max)
//            convergence = true;
//        else
//            convergence = false;
//    }
//
//    return nextPoint;
//}


double NDInterpolation::integralCellValue(std::vector<std::vector<double> > cell){
  /**
   * This function calculates the integral value of an ND function within a cell
   */

        double value = 0.0;

        int numberOfVerteces = cell.size();
        //double sign = 1.0;

        //int counter = 1;

//        for(int i=numberOfVerteces; i>0; i--){
//                value += interpolateAt(cell.at(i-1)) * sign;
//                //std::cout<<interpolateAt(cell.at(i-1))<<std::endl;
//                sign = sign * (-1.0);
//                counter++;
//                if (counter%2){
//                        sign = sign * (-1.0);
//                }
//        }

        int min=0;
        int max=0;
        for(int i=1; i<numberOfVerteces; i++){
            if (interpolateAt(cell.at(i))>interpolateAt(cell.at(max)))
                   max=i;
            if (interpolateAt(cell.at(i))<interpolateAt(cell.at(min)))
                   min=i;
        }
        value=interpolateAt(cell.at(max)) - interpolateAt(cell.at(min));
        return value;
}


int NDInterpolation::cdfWeightedPicking(std::vector<std::vector<std::vector<double> > >& vertices,double g){
  /**
   * This function randomly picks a cell weighted by the integral value of the ND function within each cell
   */

        std::vector<double> cellAvgValues (vertices.size());
        double cumulativeValue = 0.0;

        for(unsigned int i=0; i<vertices.size(); i++){
                cellAvgValues.at(i) = integralCellValue(vertices.at(i));
                cumulativeValue += cellAvgValues.at(i);
        }

        for(unsigned int i=0; i<vertices.size(); i++){
                cellAvgValues.at(i) = cellAvgValues.at(i)/cumulativeValue;

        }

        int index=0;
        double cumulativeIndex = 0.0;
        for(unsigned int i=0; i<vertices.size(); i++){
                cumulativeIndex += cellAvgValues.at(i);
                //std::cout<<cumulativeIndex<<std::endl;
                if (cumulativeIndex > g){
                  //std::cout<<"cumulativeIndex: "<<cumulativeIndex<< std::endl;
                  //std::cout<<"g: "<<g<< std::endl;
                  index=i;
                  break;
                }
        }

        return index;
}



bool NDInterpolation::pivotCellCheck(std::vector<std::vector<double> >& cell, double f){
 // This function check if f is contained within cell
 // if the vertex outcome is the same for all vertices then f is not contained within the cell --> outcome = True

 bool outcome = true;

 for (unsigned int n=1; n<cell.size(); n++){
  if (vertexOutcome(cell.at(n-1),f) == vertexOutcome(cell.at(n),f))
   outcome = outcome && true;
  else
   outcome = outcome && false;
 }
 return outcome;
}

int NDInterpolation::vertexOutcome(std::vector<double>& vertex, double f){
 int outcome;
 double value = interpolateAt(vertex);
 if (value <= f)
  outcome = 0;
 else
  outcome = 1;
 return outcome;
}

void NDInterpolation::cellsFilter(std::vector<std::vector<std::vector<double> > >& vertices, double f){
 // This function filter out the cells from vertices that do not contain f
 for (int i=vertices.size(); i>0; i--){
         if (pivotCellCheck(vertices.at(i-1), f) == true){
                 vertices.erase(vertices.begin() + i - 1);
         }
 }
}

void NDInterpolation::refinedCellDivision(std::vector<std::vector<std::vector<double> > >& refined_cell, std::vector<std::vector<double> > & cell, int divisions){
  /**
   * This function partition the original cell into subcells (each dimensions is partitioned into "divisions" intervals)
   */

 std::vector<double> dxs (_dimensions);

 for (int n=0; n<_dimensions; n++){
  int loc = (int)pow(2,n);
  dxs[n] = (double)(cell[loc][n]-cell[0][n])/(double)divisions;
 }

 int numberNewCells = (int)pow(divisions,_dimensions);

 for (int n=0; n<numberNewCells; n++){
  std::vector<int> nd_coordinate = arrayConverter(n,divisions,_dimensions);
  std::vector<std::vector<double> > newCell = generateNewCell(nd_coordinate, cell.at(0), dxs, _dimensions);
  refined_cell.push_back(newCell);
 }
}

std::vector<int> NDInterpolation::arrayConverter(int one_d_coordinate, int divisions, int n_dimensions){
 std::vector<int> nd_coordinates (n_dimensions);
 std::vector<int> weights (n_dimensions);

 weights.at(0)=1;
 for (int nDim=1; nDim<n_dimensions; nDim++)
  weights.at(nDim) = weights.at(nDim-1)*divisions;

 for (int nDim=(n_dimensions-1); nDim>=0; nDim--){
  if (nDim>0){
   nd_coordinates.at(nDim) = one_d_coordinate/weights.at(nDim);
   one_d_coordinate -= nd_coordinates.at(nDim)*weights.at(nDim);
  }
  else{
   nd_coordinates.at(0) = one_d_coordinate;
  }
 }

 return nd_coordinates;
}

std::vector<std::vector<double> > NDInterpolation::generateNewCell(std::vector<int> nd_coordinate, std::vector<double> coordinate_of_point_zero, std::vector<double> dxs, int n_dimensions){
 int numberOfVerteces = (int)pow(2,_dimensions);

 std::vector<std::vector<double> > verteces;

 std::vector<double> vertexRealCoordinate (n_dimensions);

 for (int i=0; i<numberOfVerteces; i++){
  std::vector<int> vertexIntCoordinate = arrayConverter(i,2,n_dimensions);

  for (int nDim=0; nDim<n_dimensions; nDim++)
          vertexRealCoordinate.at(nDim) = coordinate_of_point_zero.at(nDim) + nd_coordinate.at(nDim) * dxs.at(nDim) + vertexIntCoordinate.at(nDim) * dxs.at(nDim);

  verteces.push_back(vertexRealCoordinate);
 }

 return verteces;
}

std::vector<std::vector<double> > NDInterpolation::pickNewCell(std::vector<std::vector<std::vector<double> > > & cells_set, double g){
        /**
        * This function picks a cell given a number in the [0,1] interval
        */

 double numberOfCells = (double)cells_set.size();

 int pickedCell = (int)(g * numberOfCells);

 return cells_set.at(pickedCell);
}

std::vector<double> NDInterpolation::getCellCenter(std::vector<std::vector<double> > & cell){
  /**
   * This function return a random ND point contained into the cell
   */

 std::vector<double> dxs (_dimensions);
 std::vector<double> center (_dimensions);

 for (int i=0; i<_dimensions; i++){
  int vertexLoc = (int)pow(2,i);
  dxs[i] = cell[vertexLoc][i] - cell[0][i];

  double randomDisplacement = _random_double->random();

  center[i] = cell[0][i] + randomDisplacement * dxs[i];
 }
 return center;
}


double NDInterpolation::ndDerivative(std::vector<double> coordinate){
 double value = derivativeStep(coordinate,1);
 return value;
}

double NDInterpolation::oneDDerivative(double fxph, double fx, double fxmh){
 double h = fxph-fx;

 double value=(fxph-2*fx+fxmh)/(2*h);

 return value;
}

double NDInterpolation::derivativeStep(std::vector<double> coordinate, int loop){
 double value;

 double h =_cell_dxs[loop]/100.0;

 std::vector<double> coordinateU = coordinate;
 std::vector<double> coordinateD = coordinate;
 std::vector<double> coordinateC = coordinate;

 coordinateU[loop] = coordinateU[loop] + h;
 coordinateU[loop] = coordinateD[loop] - h;

 if (loop < _dimensions){
  double up     = derivativeStep(coordinateU, loop++);
  double center = derivativeStep(coordinateC, loop++);
  double down   = derivativeStep(coordinateD, loop++);
  value = oneDDerivative(up,center,down);
 }else
  value = oneDDerivative(interpolateAt(coordinateU),interpolateAt(coordinateC),interpolateAt(coordinateD));

 return value;
}


std::vector<double> intToBinary(int value, int size){
        std::vector<double> binary(size);
        for (int i = 0; i < size; i++) {
                binary.at(size-i-1) = value & (1 << i) ? 1 : 0;
        }
        return binary;
}


double NDInterpolation::averageCellValue(std::vector<double> center, std::vector<double> dx){
        double value=0.0;

        int numberOfVerteces = (int)pow(2,center.size());

        for(int i=0; i<numberOfVerteces; i++){
                std::vector<double> index = intToBinary(i,center.size());
                std::vector<double> nd_coordinate;
                for(unsigned int j=0; j<center.size(); j++){
                        if (index[j]==0)
                                nd_coordinate[j] = center[j];
                        else
                                nd_coordinate[j] = center[j] + dx[j];
                }
                value += interpolateAt(nd_coordinate);
        }

        value = value/numberOfVerteces;

        return value;
}

std::vector<double> NDInterpolation::ndInverseFunctionGrid(double f, double g){
 std::cout<<"ndInverseFunctionGrid f: " << f << std::endl;
 std::cout<<"ndInverseFunctionGrid G: " << g << std::endl;

 int last_divisions = (int)round(1.0/_tolerance);

 std::vector<double> pointMax(_dimensions);
 for(int i=0; i< _dimensions; i++)
         pointMax.at(i) = _cell_point_0.at(i)+_cell_dxs.at(i);


 std::vector<std::vector<double> > basic_cell;
 std::vector<int> nd_coordinate (_dimensions);

 for (int n=0; n<_dimensions; n++){
  nd_coordinate.at(n) = 0;
 }

 basic_cell = generateNewCell(nd_coordinate, _cell_point_0, _cell_dxs, _dimensions);

 std::vector<std::vector<std::vector<double> > > coarseCell;
 refinedCellDivision(coarseCell, basic_cell, _initial_divisions);

 // ==> cellsFilter(coarseCell, f);

 int pickedCell = cdfWeightedPicking(coarseCell,g);

 std::cout<<"pickedCell: " << pickedCell << std::endl;

 std::vector<std::vector<std::vector<double> > > refined_cell;

 refinedCellDivision(refined_cell, coarseCell.at(pickedCell), last_divisions);

 /// ===>cellsFilter(refined_cell,f);

 //std::vector<std::vector<double> > pivotSubcell = pickNewCell(refined_cell,g);
 int pickedSubCell = cdfWeightedPicking(refined_cell,f);
 std::cout<<"pickedSubCell: " << pickedSubCell << std::endl;
 std::vector<double> randomVector = getCellCenter(refined_cell.at(pickedSubCell));

 return randomVector;
}



//std::vector<double> NDInterpolation::ndInverseFunctionGrid(double f, double g){
//
// int last_divisions = (int)round(1.0/_tolerance);
//
// std::vector<double> pointMax(_dimensions);
// for(int i=0; i< _dimensions; i++){
//       pointMax.at(i) = _cell_point_0.at(i)+_cell_dxs.at(i);
// }
// f = interpolateAt(_cell_point_0) + f * (interpolateAt(pointMax) - interpolateAt(_cell_point_0));
//
// // Create basic cell
// std::vector<std::vector<double> > basic_cell;
// std::vector<int> nd_coordinate (_dimensions);
//
// for (int n=0; n<_dimensions; n++){
//  nd_coordinate.at(n) = 0;
// }
//
// basic_cell = generateNewCell(nd_coordinate, _cell_point_0, _cell_dxs, _dimensions);
//       // Divide input space into cells
// std::vector<std::vector<std::vector<double> > > coarseCell;
// refinedCellDivision(coarseCell, basic_cell, _initial_divisions);
//       // Select pivot cells
// cellsFilter(coarseCell, f);
//       // Subdivide pivot cells into sub-cells
// std::vector<std::vector<std::vector<double> > > refined_cell;
// for (unsigned int i=0; i<coarseCell.size(); i++){
//       refinedCellDivision(refined_cell, coarseCell[i], last_divisions);
// }
//       // Select pivot sub-cells
// cellsFilter(refined_cell, f);
//       // Randomly pick a pivot sub-cell
// std::vector<std::vector<double> > pivotSubcell = pickNewCell(refined_cell,g);
//       // Get center of the picked cell
// std::vector<double> randomVector = getCellCenter(pivotSubcell);
// return randomVector;
//}
