/*
 * distribution.C
 *
 *  Created on: Feb 6, 2014
 *      Author: alfoa
 *
 */

#include "distribution_base_ND.h"
#include <stdexcept>
#include <iostream>
#include "MDreader.h"
#include "distributionFunctions.h"
#include <cmath>
#include <math.h>

#include <boost/math/distributions/chi_squared.hpp>

#define _USE_MATH_DEFINES

//#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/lu.hpp>
//#include <boost/numeric/ublas/io.hpp>

//using namespace boost::numeric::ublas;
using namespace std;

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }



BasicDistributionND::BasicDistributionND()
{
}

BasicDistributionND::~BasicDistributionND()
{
}

double
BasicDistributionND::getVariable(const std::string & variableName){
   double res;

   if(_dis_parameters.find(variableName) != _dis_parameters.end())
   {
          res = _dis_parameters.find(variableName) ->second;
   }
   else
   {
     throwError("Parameter " << variableName << " was not found in distribution type " << _type <<".");
   }
   return res;
}

void
BasicDistributionND::updateVariable(const std::string & variableName, double & newValue){
   if(_dis_parameters.find(variableName) != _dis_parameters.end())
   {
     _dis_parameters[variableName] = newValue;
   }
   else
   {
     throwError("Parameter " << variableName << " was not found in distribution type " << _type << ".");
   }
}

std::string &
BasicDistributionND::getType(){
   return _type;
}

double
getDistributionVariable(BasicDistributionND & dist,const std::string & variableName){
  return dist.getVariable(variableName);
}

void
DistributionUpdateVariable(BasicDistributionND & dist,const std::string & variableName, double & newValue){
  dist.updateVariable(variableName, newValue);
}

std::string
getDistributionType(BasicDistributionND & dist) {
  return dist.getType();
}

double DistributionPdf(BasicDistributionND & dist, std::vector<double> & x)
{
  return dist.Pdf(x);
}

double DistributionCdf(BasicDistributionND & dist, std::vector<double> & x)
{
  return dist.Cdf(x);
}

std::vector<double> DistributionInverseCdf(BasicDistributionND & dist, double & min, double & max){
  return dist.InverseCdf(min, max);
}

BasicMultivariateNormal::BasicMultivariateNormal(std::string data_filename, std::vector<double> mu,std::vector<double> sigma){
  _mu = mu;
  _sigma = sigma;

  int rows,columns;
  readMatrix(data_filename, rows, columns, _covMatrix);

  if(rows != columns)
          throwError("MultivariateNormal error: covariance matrix in " << data_filename << " is not a square matrix.");
}

BasicMultivariateNormal::BasicMultivariateNormal(std::vector<std::vector<double> > covMatrix, std::vector<double> mu, std::vector<double> sigma){
  _mu = mu;
  _sigma = sigma;
  _covMatrix = covMatrix;

  computeInverse(_covMatrix, _inverseCovMatrix);

  _determinantCovMatrix = getDeterminant(covMatrix);
}

double BasicMultivariateNormal::Pdf(std::vector<double> x){
        double value = 0;

        if(_mu.size() == x.size()){
                int dimensions = _mu.size();
                double expTerm=0;
                std::vector<double> tempVector (dimensions);
                for(int i=0; i<dimensions; i++){
                        tempVector[i]=0;
                        for(int j=0; j<dimensions; j++)
                                tempVector[i] += _inverseCovMatrix[i][j]*x[j];
                        expTerm += tempVector[i]*x[i];
                }
                value = 1/pow(2*M_PI,dimensions/2)*1/sqrt(_determinantCovMatrix)*exp(expTerm);
        }else
                        throwError("MultivariateNormal PDF error: evaluation point dimensionality is not correct");

        return value;
}

double BasicMultivariateNormal::Cdf(std::vector<double> x){
        double value;

        if(_mu.size() == x.size()){
                int dimensions = _mu.size();
                boost::math::chi_squared chiDistribution(dimensions);

                double mahalanobis;
                std::vector<double> tempVector (dimensions);
                for(int i=0; i<dimensions; i++){
                        tempVector[i]=0;
                        for(int j=0; j<dimensions; j++)
                                tempVector[i] += _inverseCovMatrix[i][j]*x[j];
                        mahalanobis += tempVector[i]*x[i];
                }
                //value = chiDistribution.cdf(mahalanobis);
                value=cdf(chiDistribution,mahalanobis);
        }else
                throwError("MultivariateNormal CDF error: evaluation point dimensionality is not correct");

        return value;
}

BasicMultivariateNormal::~BasicMultivariateNormal(){

}

//template<class T>
//bool InvertMatrix(const matrix<T>& input, matrix<T>& inverse)
//{
//	typedef permutation_matrix<std::size_t> pmatrix;
//
//	// create a working copy of the input
//	matrix<T> A(input);
//
//	// create a permutation matrix for the LU-factorization
//	pmatrix pm(A.size1());
//
//	// perform LU-factorization
//	int res = lu_factorize(A, pm);
//	if (res != 0)
//		return false;
//
//	// create identity matrix of "inverse"
//	inverse.assign(identity_matrix<T> (A.size1()));
//
//	// backsubstitute to get the inverse
//	lu_substitute(A, pm, inverse);
//
//	return true;
//}
//
//void getInverse(std::vector<std::vector<double> > matrix, std::vector<std::vector<double> > inverse_matrix){
//	int dimension = matrix.size();
//	double initialValues[dimension][dimension];
//	matrix<double> A(dimension, dimension), Z(dimension, dimension);
//
//	for(int i=0; i<dimension; i++)
//		for(int j=0; j<dimension; j++)
//			initialValues[i][j]=matrix[i][j];
//	A = make_matrix_from_pointer(initialValues);
//	InvertMatrix(A, Z);
//
//	for(int i=0; i<dimension; i++){
//		std::vector<double> temp;
//		for(int j=0; j<dimension; j++)
//			temp.push_back(Z[i][j]);
//		inverse_matrix.push_back(temp);
//	}
//}
