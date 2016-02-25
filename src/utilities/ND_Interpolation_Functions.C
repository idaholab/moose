#include "ND_Interpolation_Functions.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <cmath>
#include <ctime>


#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }


double NDInterpolation::interpolateAt(std::vector<double> point_coordinate){
  throw ("Error in interpolateAt: NOT IMPLEMENTED!!!!");
  return -1;
}

//std::vector<double> NDInterpolation::NDinverseFunction(double F_min, double F_max){
//  throw ("Error in NDinverseFunction: NOT IMPLEMENTED!!!!");
//  std::vector<double> a;
//  a.push_back(-1);
//  return a;
//}


double NDInterpolation::getGradientAt(std::vector<double> point_coordinate){
  throw ("Error in getGradientAt: NOT IMPLEMENTED!!!!");
  return -1;
}

void NDInterpolation::fit(std::vector< std::vector<double> > coordinates, std::vector<double> values){
  throw ("Error in fit: NOT IMPLEMENTED!!!!");
}

void NDInterpolation::updateRNGparameters(double tolerance, double initial_divisions){
        _tolerance = tolerance;
        _initial_divisions = (int)initial_divisions;
}


NDInterpolation::NDInterpolation()
{
          _tolerance = 0.1;
          _initial_divisions = 10;
          _randomDouble = new RandomClass();
          _randomDouble->seed(0);
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

//std::vector<double> InverseDistanceWeighting::NDinverseFunction(double F_min, double F_max){
// // iterative procedure of linear interpolation to determine a nextPoint between firstPoint and secondPoint until CDF(nextPoint) lies between F_min and F_max
//
// double referenceCDF = (F_max-F_min)/2;
//
// boost::random::mt19937 rng;
// rng.seed(time(NULL));
// double range = rng.max() - rng.min();
//
//    // check input values inconsistencies
//
//    if (F_min > F_max)
//        throwError("ND RNG function: invalid param (F_min > F_max)");
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
//    if (valueFirstPoint < F_min && valueFirstPoint < F_max){
//        secondPoint = max_values;
//        valueSecondPoint = 1.0;
//        convergence = false;
//    } else if (interpolateAt(firstPoint) > F_min && interpolateAt(firstPoint) > F_max){
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
//        if (valueNextPoint > F_min && valueNextPoint < F_max)
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
        double sign = 1.0;

        int counter = 1;

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


int NDInterpolation::CDFweightedPicking(std::vector<std::vector<std::vector<double> > >& vertices,double g){
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



bool NDInterpolation::pivotCellCheck(std::vector<std::vector<double> >& cell, double F){
 // This function check if F is contained within cell
 // if the vertex outcome is the same for all vertices then F is not contained within the cell --> outcome = True

 bool outcome = true;

 for (unsigned int n=1; n<cell.size(); n++){
  if (vertexOutcome(cell.at(n-1),F) == vertexOutcome(cell.at(n),F))
   outcome = outcome && true;
  else
   outcome = outcome && false;
 }
 return outcome;
}

int NDInterpolation::vertexOutcome(std::vector<double>& vertex, double F){
 int outcome;
 double value = interpolateAt(vertex);
 if (value <= F)
  outcome = 0;
 else
  outcome = 1;
 return outcome;
}

void NDInterpolation::cellsFilter(std::vector<std::vector<std::vector<double> > >& vertices, double F){
 // This function filter out the cells from vertices that do not contain F
 for (int i=vertices.size(); i>0; i--){
         if (pivotCellCheck(vertices.at(i-1), F) == true){
                 vertices.erase(vertices.begin() + i - 1);
         }
 }
}

void NDInterpolation::refinedCellDivision(std::vector<std::vector<std::vector<double> > >& refinedCell, std::vector<std::vector<double> > & cell, int divisions){
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
  std::vector<int> NDcoordinate = arrayConverter(n,divisions,_dimensions);
  std::vector<std::vector<double> > newCell = generateNewCell(NDcoordinate, cell.at(0), dxs, _dimensions);
  refinedCell.push_back(newCell);
 }
}

std::vector<int> NDInterpolation::arrayConverter(int oneDcoordinate, int divisions, int n_dimensions){
 std::vector<int> NDcoordinates (n_dimensions);
 std::vector<int> weights (n_dimensions);

 weights.at(0)=1;
 for (int nDim=1; nDim<n_dimensions; nDim++)
  weights.at(nDim) = weights.at(nDim-1)*divisions;

 for (int nDim=(n_dimensions-1); nDim>=0; nDim--){
  if (nDim>0){
   NDcoordinates.at(nDim) = oneDcoordinate/weights.at(nDim);
   oneDcoordinate -= NDcoordinates.at(nDim)*weights.at(nDim);
  }
  else{
   NDcoordinates.at(0) = oneDcoordinate;
  }
 }

 return NDcoordinates;
}

std::vector<std::vector<double> > NDInterpolation::generateNewCell(std::vector<int> NDcoordinate, std::vector<double> coordinateOfPointZero, std::vector<double> dxs, int n_dimensions){
 int numberOfVerteces = (int)pow(2,_dimensions);

 std::vector<std::vector<double> > verteces;

 std::vector<double> vertexRealCoordinate (n_dimensions);

 for (int i=0; i<numberOfVerteces; i++){
  std::vector<int> vertexIntCoordinate = arrayConverter(i,2,n_dimensions);

  for (int nDim=0; nDim<n_dimensions; nDim++)
          vertexRealCoordinate.at(nDim) = coordinateOfPointZero.at(nDim) + NDcoordinate.at(nDim) * dxs.at(nDim) + vertexIntCoordinate.at(nDim) * dxs.at(nDim);

  verteces.push_back(vertexRealCoordinate);
 }

 return verteces;
}

std::vector<std::vector<double> > NDInterpolation::pickNewCell(std::vector<std::vector<std::vector<double> > > & cellsSet, double g){
        /**
        * This function picks a cell given a number in the [0,1] interval
        */

 double numberOfCells = (double)cellsSet.size();

 int pickedCell = (int)(g * numberOfCells);

 return cellsSet.at(pickedCell);
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

  double randomDisplacement = _randomDouble->random();

  center[i] = cell[0][i] + randomDisplacement * dxs[i];
 }
 return center;
}


double NDInterpolation::NDderivative(std::vector<double> coordinate){
 double value = derivativeStep(coordinate,1);
 return value;
}

double NDInterpolation::OneDderivative(double fxph, double fx, double fxmh){
 double h = fxph-fx;

 double value=(fxph-2*fx+fxmh)/(2*h);

 return value;
}

double NDInterpolation::derivativeStep(std::vector<double> coordinate, int loop){
 double value;

 double h =_cellDxs[loop]/100.0;

 std::vector<double> coordinateU = coordinate;
 std::vector<double> coordinateD = coordinate;
 std::vector<double> coordinateC = coordinate;

 coordinateU[loop] = coordinateU[loop] + h;
 coordinateU[loop] = coordinateD[loop] - h;

 if (loop < _dimensions){
  double up     = derivativeStep(coordinateU, loop++);
  double center = derivativeStep(coordinateC, loop++);
  double down   = derivativeStep(coordinateD, loop++);
  value = OneDderivative(up,center,down);
 }else
  value = OneDderivative(interpolateAt(coordinateU),interpolateAt(coordinateC),interpolateAt(coordinateD));

 return value;
}


std::vector<double> int2binary(int value, int size){
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
                std::vector<double> index = int2binary(i,center.size());
                std::vector<double> NDcoordinate;
                for(unsigned int j=0; j<center.size(); j++){
                        if (index[j]==0)
                                NDcoordinate[j] = center[j];
                        else
                                NDcoordinate[j] = center[j] + dx[j];
                }
                value += interpolateAt(NDcoordinate);
        }

        value = value/numberOfVerteces;

        return value;
}

std::vector<double> NDInterpolation::NDinverseFunctionGrid(double F, double g){
 std::cout<<"NDinverseFunctionGrid F: " << F << std::endl;
 std::cout<<"NDinverseFunctionGrid G: " << g << std::endl;

 int last_divisions = (int)round(1.0/_tolerance);

 std::vector<double> pointMax(_dimensions);
 for(int i=0; i< _dimensions; i++)
         pointMax.at(i) = _cellPoint0.at(i)+_cellDxs.at(i);


 std::vector<std::vector<double> > basic_cell;
 std::vector<int> NDcoordinate (_dimensions);

 for (int n=0; n<_dimensions; n++){
  NDcoordinate.at(n) = 0;
 }

 basic_cell = generateNewCell(NDcoordinate, _cellPoint0, _cellDxs, _dimensions);

 std::vector<std::vector<std::vector<double> > > coarseCell;
 refinedCellDivision(coarseCell, basic_cell, _initial_divisions);

 // ==> cellsFilter(coarseCell, F);

 int pickedCell = CDFweightedPicking(coarseCell,g);

 std::cout<<"pickedCell: " << pickedCell << std::endl;

 std::vector<std::vector<std::vector<double> > > refinedCell;

 refinedCellDivision(refinedCell, coarseCell.at(pickedCell), last_divisions);

 /// ===>cellsFilter(refinedCell,F);

 //std::vector<std::vector<double> > pivotSubcell = pickNewCell(refinedCell,g);
 int pickedSubCell = CDFweightedPicking(refinedCell,F);
 std::cout<<"pickedSubCell: " << pickedSubCell << std::endl;
 std::vector<double> randomVector = getCellCenter(refinedCell.at(pickedSubCell));

 return randomVector;
}



//std::vector<double> NDInterpolation::NDinverseFunctionGrid(double F, double g){
//
// int last_divisions = (int)round(1.0/_tolerance);
//
// std::vector<double> pointMax(_dimensions);
// for(int i=0; i< _dimensions; i++){
//       pointMax.at(i) = _cellPoint0.at(i)+_cellDxs.at(i);
// }
// F = interpolateAt(_cellPoint0) + F * (interpolateAt(pointMax) - interpolateAt(_cellPoint0));
//
// // Create basic cell
// std::vector<std::vector<double> > basic_cell;
// std::vector<int> NDcoordinate (_dimensions);
//
// for (int n=0; n<_dimensions; n++){
//  NDcoordinate.at(n) = 0;
// }
//
// basic_cell = generateNewCell(NDcoordinate, _cellPoint0, _cellDxs, _dimensions);
//       // Divide input space into cells
// std::vector<std::vector<std::vector<double> > > coarseCell;
// refinedCellDivision(coarseCell, basic_cell, _initial_divisions);
//       // Select pivot cells
// cellsFilter(coarseCell, F);
//       // Subdivide pivot cells into sub-cells
// std::vector<std::vector<std::vector<double> > > refinedCell;
// for (unsigned int i=0; i<coarseCell.size(); i++){
//       refinedCellDivision(refinedCell, coarseCell[i], last_divisions);
// }
//       // Select pivot sub-cells
// cellsFilter(refinedCell, F);
//       // Randomly pick a pivot sub-cell
// std::vector<std::vector<double> > pivotSubcell = pickNewCell(refinedCell,g);
//       // Get center of the picked cell
// std::vector<double> randomVector = getCellCenter(pivotSubcell);
// return randomVector;
//}
