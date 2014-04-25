#include "ND_Interpolation_Functions.h"
#include <math.h>
#include <vector>
#include <stdlib.h>
#include "MDreader.h"

microSphere::microSphere(std::string filename, double p, int precision){
	_dataFileName = filename;
	_p = p;
	_precision = precision;

	readScatteredNDarray(_dataFileName, _dimensions, _numberOfPoints, _pointCoordinates, _values);

	MSinitialization();
	_completedInit = true;
}

microSphere::microSphere(double p, int precision)
{
    _p = p;
    _precision = precision;
    _completedInit = false;
    _numberOfPoints = 0;
    _dimensions=0;
}

void microSphere::MSinitialization(){
	srand (time(NULL));

	for (int j=0; j<_precision; j++){
		double sum = 0;
		do{
			for (int i=0; i<_dimensions; i++){
				// x,y,z are uniformly-distributed random numbers in the range (-1,1)
				_unitVector[j][i] = -1+2*rand();
				sum += _unitVector[j][i];
			}
			sum = sqrt(sum);
		}while (sum > 1);

		for (int i=0; i<_dimensions; i++)
			_unitVector[j][i] /= sum;
	}
}


void
microSphere::fit(std::vector< std::vector<double> > coordinates, std::vector<double> values){
  _dimensions=coordinates[0].size();
  _numberOfPoints = coordinates.size();
  _pointCoordinates = coordinates;
  _values = values;
  MSinitialization();
  _completedInit = true;
}


double microSphere::interpolateAt(std::vector<double> point_coordinate){
	double value = 0;
	double weight = 0;
	double cosValue = 0;
	if (not _completedInit)
	{
	  throw ("Error in interpolateAt: the class has not been completely initialized... you can not interpolate!!!!");
	}
	std::vector<double> weights (_values.size());

	for (int n=0; n<_values.size(); n++){
		weight = minkowskiDistance(point_coordinate,_pointCoordinates[n],_p);

		for (int j=0; j<_precision; j++){
			cosValue = cosValueBetweenVectors(_unitVector[j],point_coordinate);

			if (cosValue*weight>0){
				// Do something
			}
		}
	}

	return value;
}

double microSphere::cosValueBetweenVectors(std::vector<double> point1, std::vector<double> point2){
	double cosAngle = 0;
	double point1point2 = 0;

	if (point1.size() == point2.size()){
		for (int nDim=0; nDim<point1.size(); nDim++){
			point1point2 += point1[nDim] * point2[nDim];
		}

		cosAngle = point1point2/(vectorNorm(point1, _p)*vectorNorm(point2, _p));
	}else
		throw ("Error in cosValueBetweenVectors: points having different dimensions");

	return cosAngle;
}

double microSphere::getGradientAt(std::vector<double> point_coordinate){
  if (not _completedInit)
  {
    throw ("Error in getGradientAt: the class has not been completely initialized... you can not interpolate!!!!");
  }
  return -1.0;
}



