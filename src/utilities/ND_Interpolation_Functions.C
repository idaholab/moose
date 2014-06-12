#include "ND_Interpolation_Functions.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <fstream>
#include <vector>
#include <boost/random/mersenne_twister.hpp>

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

NDInterpolation::NDInterpolation()
{
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
		double pDistance = 0;
		for (int i=0; i<point1.size(); i++){
			pDistance += pow(abs(point1[i]-point2[i]),p);
		}
		distance = pow(pDistance, 1/p);
		//distance = sqrt(pDistance);
	}else
		throw ("Error in minkowski distance: points having different dimensions");

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


double NDInterpolation::NDderivative(std::vector<double> x){
	double value;
	return value;
}
