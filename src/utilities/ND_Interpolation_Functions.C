#include "ND_Interpolation_Functions.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <math.h>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <ctime>


#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }


double NDInterpolation::interpolateAt(std::vector<double> point_coordinate){
  throw ("Error in interpolateAt: NOT IMPLEMENTED!!!!");
  return -1;
}

std::vector<double> NDInterpolation::NDinverseFunction(double F_min, double F_max){
  throw ("Error in NDinverseFunction: NOT IMPLEMENTED!!!!");
  std::vector<double> a;
  a.push_back(-1);
  return a;
}


double NDInterpolation::getGradientAt(std::vector<double> point_coordinate){
  throw ("Error in getGradientAt: NOT IMPLEMENTED!!!!");
  return -1;
}

void NDInterpolation::fit(std::vector< std::vector<double> > coordinates, std::vector<double> values){
  throw ("Error in fit: NOT IMPLEMENTED!!!!");
}

void NDInterpolation::updateRNGparameters(double tolerance, double initial_divisions){
	_tolerance = tolerance;
	_initial_divisions = initial_divisions;

	std::cout<<"c++ NDInterpolation updateRNGparameter" << _tolerance << _initial_divisions << std::endl;
}


NDInterpolation::NDInterpolation()
{
	  _tolerance = 0.1;
	  _initial_divisions = 10;
}
NDInterpolation::~NDInterpolation()
{
}

bool checkUpperBound(double upper_bound, std::vector<double> values){
 bool check = true;

 for (int i=0; i<values.size(); i++){
  if (values.at(i) > upper_bound){
   check = check && false;
   std::cout<< "values.at(i) : " << values.at(i) << std::endl;
  }
  else
   check = check && true;
 }

 return check;
}


bool checkLowerBound(double lower_bound, std::vector<double> values){
 bool check = true;

 for (int i=0; i<values.size(); i++){
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
   for (int i=0; i<point1.size(); i++){
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

 for (int i=1; i<point.size(); i++){
  norm = pow(point[i],p);
 }

 norm = pow(norm, 1/p);

 return norm;
}

std::vector<double> InverseDistanceWeighting::NDinverseFunction(double F_min, double F_max){
 // iterative procedure of linear interpolation to determine a nextPoint between firstPoint and secondPoint until CDF(nextPoint) lies between F_min and F_max

 double referenceCDF = (F_max-F_min)/2;

 boost::random::mt19937 rng;
 rng.seed(time(NULL));
 double range = rng.max() - rng.min();

    // check input values inconsistencies

    if (F_min > F_max)
        throwError("ND RNG function: invalid param (F_min > F_max)");

    std::vector<double> min_values (_dimensions);
    std::vector<double> max_values (_dimensions);

    // Find extreme points

    for (int nDim=0; nDim<_dimensions; nDim++){
        for (int nPoints=0; nPoints<_number_of_points; nPoints++){
            if (_point_coordinates[nPoints][nDim] < min_values[nDim])
                min_values[nDim] = _point_coordinates[nPoints][nDim];
            if (_point_coordinates[nPoints][nDim] > max_values[nDim])
                max_values[nDim] = _point_coordinates[nPoints][nDim];
        }
    }

    std::vector<double> firstPoint  (_dimensions);
    std::vector<double> secondPoint (_dimensions);
    std::vector<double> nextPoint   (_dimensions);


    // Randomly pick the fistPoint
    for (int nDim=0; nDim<_dimensions; nDim++){
     double temp = (rng()-rng.min())/range;;
        firstPoint[nDim] = min_values[nDim] + (max_values[nDim]-min_values[nDim]) * temp;
    }

    int min_index, max_index = 0;

    double min_value , max_value = 0.0;
    for (int n=0; n<_number_of_points; n++){
        if (_values[n] < min_value){
         min_index = n;
            min_value = _values[min_index];
        }
        if (_values[n] > max_value){
         max_index = n;
            max_value = _values[max_index];
        }
    }

    bool convergence;

    double valueFirstPoint = interpolateAt(firstPoint);
    double valueSecondPoint;
    double valueNextPoint;

    if (valueFirstPoint < F_min && valueFirstPoint < F_max){
        secondPoint = max_values;
        valueSecondPoint = 1.0;
        convergence = false;
    } else if (interpolateAt(firstPoint) > F_min && interpolateAt(firstPoint) > F_max){
        secondPoint = min_values;
        valueSecondPoint = 0.0;
        convergence = false;
    } else {
     nextPoint = firstPoint;
        convergence = true;
    }

    while (convergence){
        // Determine next point

        for (int nDim=0; nDim<_dimensions; nDim++)
            nextPoint[nDim] = (firstPoint[nDim]-secondPoint[nDim])*(valueFirstPoint-valueSecondPoint)/(referenceCDF-valueSecondPoint)+secondPoint[nDim];
        valueNextPoint = interpolateAt(nextPoint);

        if (referenceCDF>valueFirstPoint && referenceCDF<valueSecondPoint){
            firstPoint = nextPoint;
            // secondPoint remains
        } else if (referenceCDF>valueSecondPoint && referenceCDF<valueFirstPoint) {
            // firstPoint remains;
            secondPoint = nextPoint;
        } else
            throwError("Computational error in NDinverse");

        // test convergence
        if (valueNextPoint > F_min && valueNextPoint < F_max)
            convergence = true;
        else
            convergence = false;
    }

    return nextPoint;
}



std::vector<double> NDInterpolation::NDinverseFunctionGrid(double F, double g){

 //initial_divisions = 10;
 //tolerance = 0.1;

 //std::cout<<"c++ initial_divisions: "<< _initial_divisions << std::endl;
 //std::cout<<"c++ tolerance: "<< _tolerance << std::endl;
 std::cout<<"c++ F: "<< F << std::endl;
 //std::cout<<"c++ _dimensions: "<< _dimensions << std::endl;

 int last_divisions = (int)round(1.0/_tolerance*_initial_divisions);

 // Create basic cell
 std::vector<std::vector<double> > basic_cell;
 std::vector<int> NDcoordinate (_dimensions);

 for (int n=0; n<_dimensions; n++)
  NDcoordinate.at(n) = 0;
 //std::cout<<"NDinverseFunctionGrid: here1 "<< _dimensions <<std::endl;
 basic_cell = generateNewCell(NDcoordinate, _cellPoint0, _cellDxs, _dimensions);
 //std::cout<<"NDinverseFunctionGrid: here2 "<<std::endl;
 // Divide input space into cells
 std::vector<std::vector<std::vector<double> > > coarseCell;
 refinedCellDivision(coarseCell, basic_cell, _initial_divisions);
 //std::cout<<"NDinverseFunctionGrid: here3 "<<std::endl;
 // Select pivot cells
 cellsFilter(coarseCell, F);
 //std::cout<<"NDinverseFunctionGrid: here4 "<<std::endl;
 // Subdivide pivot cells into sub-cells
 std::vector<std::vector<std::vector<double> > > refinedCell;
 for (int i=0; i<coarseCell.size(); i++)
	 refinedCellDivision(refinedCell, coarseCell[i], last_divisions);
 //std::cout<<"NDinverseFunctionGrid: here5 "<<std::endl;
 // Select pivot sub-cells
 cellsFilter(refinedCell, F);
 //std::cout<<"NDinverseFunctionGrid: here6 "<<std::endl;
 // Randomly pick a pivot sub-cell
 std::vector<std::vector<double> > pivotSubcell = pickNewCell(refinedCell,g);
 //std::cout<<"NDinverseFunctionGrid: here7"<<std::endl;
 // Get center of the picked cell
 std::vector<double> randomVector = getCellCenter(pivotSubcell);
 //std::cout<<"NDinverseFunctionGrid: here8"<<std::endl;

 return randomVector;
}


bool NDInterpolation::pivotCellCheck(std::vector<std::vector<double> > cell, double F){
 // This function check if F is contained within cell
 // if the vertex outcome is the same for all vertices then F is not contained within the cell --> outcome = True

 bool outcome = true;

 for (int n=1; n<cell.size(); n++){
  if (vertexOutcome(cell[n-1],F) == vertexOutcome(cell[n],F))
   outcome = outcome && true;
  else
   outcome = outcome && false;
 }
 return outcome;
}

int NDInterpolation::vertexOutcome(std::vector<double> vertex, double F){
 int outcome;

 if (interpolateAt(vertex) <= F)
  outcome = 0;
 else
  outcome = 1;

 return outcome;
}

void NDInterpolation::cellsFilter(std::vector<std::vector<std::vector<double> > >& vertices, double F){
 // This function filter out the cells from vertices that do not contain F
 //std::cout<<"vertices size: "<< vertices.size() << std::endl;

 for (int i=vertices.size(); i<0; i--)
  if (pivotCellCheck(vertices[i], F) == false){
   vertices.erase(vertices.begin() + i);
  }
}

void NDInterpolation::refinedCellDivision(std::vector<std::vector<std::vector<double> > >& refinedCell, std::vector<std::vector<double> > cell, int divisions){

 std::vector<double> dxs (_dimensions);

 for (int n=0; n<_dimensions; n++){
  int loc = (int)pow(2,n);
  dxs[n] = (double)(cell[loc][n]-cell[0][n])/(double)divisions;
  //std::cout<< "cell[loc][n]: "<< cell[loc][n] << std::endl;
  //std::cout<< "cell[0][n]: "<< cell[0][n] << std::endl;
  //std::cout<< "dx: "<< dxs[n] << std::endl;
 }

 int numberNewCells = (int)pow(divisions,_dimensions);

 for (int n=0; n<numberNewCells; n++){
  std::vector<int> NDcoordinate = arrayConverter(n,divisions,_dimensions);
  std::vector<std::vector<double> > newCell = generateNewCell(NDcoordinate, cell[0], dxs, _dimensions);
  refinedCell.push_back(newCell);
 }

 //std::cout<< "refinedCell.size: "<< refinedCell.size() << std::endl;
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
 //std::cout<<" " << std::endl;
 return verteces;
}

std::vector<std::vector<double> > NDInterpolation::pickNewCell(std::vector<std::vector<std::vector<double> > > cellsSet, double g){
 int numberOfCells = cellsSet.size();

 // generate RN in [0,1]
// int seed = time(0);
// boost::random::mt19937 rng;
// rng.seed(seed);
// boost::random::uniform_int_distribution<> dist(1, numberOfCells);
//
// int pickedCell = dist(rng)-1;

 int pickedCell = floor(g * numberOfCells);

 return cellsSet[pickedCell];
}

std::vector<double> NDInterpolation::getCellCenter(std::vector<std::vector<double> > cell){
 std::vector<double> dxs (_dimensions);
 std::vector<double> center (_dimensions);

 for (int i=0; i<_dimensions; i++){
  int vertexLoc = (int)pow(2,i);
  dxs[i] = (cell[vertexLoc][i] - cell[0][i])/2.0;

  center[i] = cell[0][i] + dxs[i];

  std::cout << "center: "<< center[i]<< std::endl;
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


double NDInterpolation::integral(std::vector<double> coordinate, int samples){
 double value;
 double volume=1;
 double sum=0;
 std::vector<double> cellrange (_dimensions);
 std::vector<double> sampleCoordinate (_dimensions);

 boost::random::mt19937 rng;
 boost::random::uniform_real_distribution<> zeroOne(0,1);

 for (int i=0; i<_dimensions; i++){
  cellrange[i] = coordinate[i]-_cellPoint0[i];
  volume = volume * cellrange[i];
 }

 for (int n=0; n<samples; n++){
  for (int i=0; i<_dimensions; i++)
   sampleCoordinate[i] = _cellPoint0[i] + zeroOne(rng)  * cellrange[i];

  sum = sum + interpolateAt(sampleCoordinate);
 }
 return value = volume/samples * sum;
}


std::vector<double> int2binary(int value, int size){
	std::vector<double> binary(size);
	for (int i = 0; i < size; i++) {
		binary[i] = value & (1 << i) ? 1 : 0;
	}
	return binary;
}


double NDInterpolation::averageCellValue(std::vector<double> center, std::vector<double> dx){
	double value=0.0;

	int numberOfVerteces = (int)pow(2,center.size());

	for(int i=0; i<numberOfVerteces; i++){
		std::vector<double> index = int2binary(i,center.size());
		std::vector<double> NDcoordinate;
		for(int j=0; j<center.size(); j++){
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







