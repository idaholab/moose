/*
 * distribution_base_ND.C
 *
 *  Created on: Feb 6, 2014
 *      Author: alfoa
 *
 */

#include "distribution_base_ND.h"
#include "DistributionContainer.h"
#include <stdexcept>
#include <iostream>
#include <stdio.h>
#include "MDreader.h"
#include "distributionFunctions.h"
#include <cmath>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/math/distributions/chi_squared.hpp>
#include <boost/math/special_functions/gamma.hpp>
#include <boost/math/special_functions/erf.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/math/distributions/normal.hpp>
#include "distribution_1D.h"
using boost::math::normal;

#include <ctime>

#define _USE_MATH_DEFINES

//#include <boost/numeric/ublas/matrix.hpp>
//#include <boost/numeric/ublas/lu.hpp>
//#include <boost/numeric/ublas/io.hpp>

//using namespace boost::numeric::ublas;

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }


BasicDistributionND::BasicDistributionND()
{
	_tolerance = 0.1;
	_initial_divisions = 10;
}

BasicDistributionND::~BasicDistributionND()
{
}

double
BasicDistributionND::getVariable(const std::string & variable_name){
   double res;

   if(_dis_parameters.find(variable_name) != _dis_parameters.end())
   {
          res = _dis_parameters.find(variable_name) ->second;
   }
   else
   {
     throwError("Parameter " << variable_name << " was not found in distribution type " << _type <<".");
   }
   return res;
}

void
BasicDistributionND::updateVariable(const std::string & variable_name, double & new_value){
   if(_dis_parameters.find(variable_name) != _dis_parameters.end())
   {
     _dis_parameters[variable_name] = new_value;
   }
   else
   {
     throwError("Parameter " << variable_name << " was not found in distribution type " << _type << ".");
   }
}

std::string &
BasicDistributionND::getType(){
   return _type;
}

double BasicDistributionND::cellIntegral(std::vector<double> center, std::vector<double> dx){
	  /**
	   * This function calculates the integral of the pdf in a cell region
	   * In the 1D case a cell region is an interval [a,b], thus the integral of the pdf in such interval is
	   * calculated as CDF(b)-CDF(a). This functions perform a similar evolution but for a generic ND cell
	   */

	double value = 0.0;

	int numberOfVerteces = (int)pow(2,center.size());
	double sign = 1.0;

	int counter=1;

	for(int i=numberOfVerteces; i>0; i--){
		std::vector<double> index = int2binary(i-1,center.size());
		std::vector<double> NDcoordinate(center.size());

		for(unsigned int j=0; j<center.size(); j++){
			if (index.at(j)==0)
				NDcoordinate.at(j) = center.at(j) - dx.at(j)/2.0;
			else
				NDcoordinate.at(j) = center.at(j) + dx.at(j)/2.0;
		}
		value += Cdf(NDcoordinate) * sign;

		sign = sign * (-1.0);
		counter++;
		if (counter%2)
			sign = sign * (-1.0);
	}

	return value;
}

//double BasicDistributionND::cellIntegral(std::vector<double> center, std::vector<double> dx){
//	double value=0.0;
//
//	int numberOfVerteces = (int)pow(2,center.size());
//
//	for(int i=0; i<numberOfVerteces; i++){
//		std::vector<double> index = int2binary(i,center.size());
//		std::vector<double> NDcoordinate(center.size());
//
//		for(unsigned int j=0; j<center.size(); j++){
//			if (index[j]==0)
//				NDcoordinate.at(j) = center.at(j) - dx.at(j)/2.0;
//			else
//				NDcoordinate.at(j) = center.at(j) + dx.at(j)/2.0;
//		}
//		value += Cdf(NDcoordinate);
//	}
//
//	value = value/numberOfVerteces;
//
//	return value;
//}


double
getDistributionVariable(BasicDistributionND & dist,const std::string & variable_name){
  return dist.getVariable(variable_name);
}

void
DistributionUpdateVariable(BasicDistributionND & dist,const std::string & variable_name, double & new_value){
  dist.updateVariable(variable_name, new_value);
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

//std::vector<double> DistributionInverseCdf(BasicDistributionND & dist, double & min, double & max){
std::vector<double> DistributionInverseCdf(BasicDistributionND & dist, double & F, double & g){
	return dist.InverseCdf(F,g);
}

void BasicMultivariateNormal::base10tobaseN(int value_base10, int base, std::vector<int> & value_baseN){
	  /**
	   * This function convert a number in base 10 to a new number in any base N
	   */

	   int index = 0 ;

	   if (value_base10 == 0)
		   value_baseN.push_back(0);
	   else{
		   while ( value_base10 != 0 ){
			   int remainder = value_base10 % base ;  // assume K > 1
			   value_base10  = value_base10 / base ;  // integer division
			   value_baseN.push_back(remainder);
			   index++ ;
			}
	   }
}

//void BasicMultivariateNormal::BasicMultivariateNormal_init(std::string data_filename, std::vector<double> mu){
void BasicMultivariateNormal::BasicMultivariateNormal_init(int &rows, int &columns, std::vector<std::vector<double> > covMatrix, std::vector<double> mu){
	  /**
	   * This is the base function that initializes the Multivariate normal distribution
     * Input Parameter
     * rows: first dimension of covariance matrix
     * columns: second dimension of covariance matrix
     * covMatrix: covariance matrix stored in vector<vector<double> >
     * mu: mean value stored in vector<double>
	   */

   _mu = mu;
   _cov_matrix = covMatrix;
   
   std::vector<std::vector<double> > inverseCovMatrix (rows,std::vector< double >(columns));

   computeInverse(_cov_matrix, inverseCovMatrix);

   for (int i=0;i<rows;i++){
	std::vector<double> temp;
	  for (int j=0;j<columns;j++)
	   temp.push_back(inverseCovMatrix.at(i).at(j));
	  _inverse_cov_matrix.push_back(temp);
   }

   unsigned int dimensions = _mu.size();
   //for(int i=0; i<dimensions; i++)
	//for(int j=0; j<dimensions; j++)
	 //std::cerr<<_inverse_cov_matrix[i][j]<<std::endl;

   _determinant_cov_matrix = getDeterminant(_cov_matrix);

   _cholesky_C = choleskyDecomposition(_cov_matrix);

   if(rows != columns)
	   throwError("MultivariateNormal error: covariance matrix in is not a square matrix.");

   // Creation BasicMultiDimensionalCartesianSpline(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta, bool CDFprovided)
   // number of discretizations in sigma/2 units; plus/minus six sigma

   int numberValues=1;
   std::vector< std::vector<double> > discretizations;
   std::vector<double> alpha (_mu.size());
   std::vector<double> beta (_mu.size());

   for(int i=0; i<dimensions; i++){
	   alpha.at(i) = 0.0;
	   beta.at(i)  = 0.0;

	   numberValues = numberValues * 25;

	   std::vector<double> discretization_temp;
	   double sigma = sqrt(_cov_matrix[i][i]);
	   for(int n=0; n<25; n++){
		   double disc_value = mu.at(i) - 6.0 * sigma + sigma * (double)n /2.0;
		   discretization_temp.push_back(disc_value);
	   }
	   discretizations.push_back(discretization_temp);
   }

   std::vector< double > values (numberValues);
   for(int i=0; i<numberValues; i++){
	   std::vector<int> intCoordinates;
	   base10tobaseN(i,25,intCoordinates);

	   std::vector<double> pointCoordinates(dimensions);
	   std::vector<int> intCoordinatesFormatted(dimensions);

	   for(unsigned int j=0; j<dimensions; j++)
		   intCoordinatesFormatted.at(j) = 0;
	   for(unsigned int j=0; j<intCoordinates.size(); j++)
		   intCoordinatesFormatted.at(j) = intCoordinates.at(j);

	   for(unsigned int j=0; j<intCoordinates.size(); j++)
		   pointCoordinates.at(j) = discretizations.at(j).at(intCoordinatesFormatted.at(j));

	   values.at(i) = getPdf(pointCoordinates, _mu, _inverse_cov_matrix);
   }
   _cartesianDistribution = BasicMultiDimensionalCartesianSpline(discretizations,values,alpha,beta,false);
}

BasicMultivariateNormal::BasicMultivariateNormal(std::string data_filename, std::vector<double> mu){
	  /**
	   * This is the function that initializes the Multivariate normal distribution given:
	   * - data_filename: it specifies the covariance matrix
	   * - mu: the mean value vector
	   */
  int rows,columns;
  std::vector<std::vector<double> > covMatrix;
  readMatrix(data_filename, rows, columns, covMatrix);
  BasicMultivariateNormal_init(rows,columns,covMatrix, mu);
}

BasicMultivariateNormal::BasicMultivariateNormal(const char * data_filename, std::vector<double> mu){
	  /**
	   * This is the function that initializes the Multivariate normal distribution given:
	   * - data_filename: it specifies the covariance matrix
	   * - mu: the mean value vector
	   */
  int rows,columns;
  std::vector<std::vector<double> > covMatrix;
  readMatrix(std::string(data_filename), rows, columns, covMatrix);
	BasicMultivariateNormal_init(rows,columns,covMatrix, mu);
	//BasicMultivariateNormal_init(std::string(data_filename) , mu);
}

BasicMultivariateNormal::BasicMultivariateNormal(std::vector<std::vector<double> > covMatrix, std::vector<double> mu){
  /**
   * This is the function that initializes the Multivariate normal distribution given:
   * - covMatrix: covariance matrix
   * - mu: the mean value vector
   */
  int rows, columns;
  rows = covMatrix.size();
  columns = covMatrix.at(0).size();

	BasicMultivariateNormal_init(rows,columns,covMatrix, mu);
  //_mu = mu;
  //_cov_matrix = covMatrix;

  //computeInverse(_cov_matrix, _inverse_cov_matrix);

  //_determinant_cov_matrix = getDeterminant(_cov_matrix);
}

// Input Parameters: vectors of covariance and mu 
BasicMultivariateNormal::BasicMultivariateNormal(std::vector<double> vecCovMatrix, std::vector<double> mu){
  /**
   * This is the function that initializes the Multivariate normal distribution given:
   * Input Parameters
   * - vecCovMatrix: covariance matrix stored in a vector<double>
   * - mu: the mean value vector
   */

  int rows, columns;
  std::vector<std::vector<double> > covMatrix;
  // convert the vecCovMatrix to covMatrix, output the rows and columns of the covariance matrix
  vectorToMatrix(rows,columns,vecCovMatrix,covMatrix);

	BasicMultivariateNormal_init(rows,columns,covMatrix, mu);
}

BasicMultivariateNormal::BasicMultivariateNormal(std::vector<double> vecCovMatrix, std::vector<double> mu, const char* type, int rank){
  /**
   * This is the function that initializes the Multivariate normal distribution given:
   * This function will compute the svd of given vecCovMatrix
   * Input Parameters
   * - vecCovMatrix: covariance matrix stored in a vector<double>
   * - mu: the mean value vector
   * - rank: the reduced dimension
   * - type: the type of given covariance matrix (vecCovMatrix), it can be 'abs' or 'rel', which means absolute covariance matrix or relative convariance matrix respectively. 
   */
  int rows, columns;
  std::vector<std::vector<double> > covMatrix;
  // convert the vecCovMatrix to covMatrix, output the rows and columns of the covariance matrix
  vectorToMatrix(rows,columns,vecCovMatrix,covMatrix);
  _mu = mu;
  _cov_matrix = covMatrix;
  _rank = (unsigned int) rank;
  _covarianceType = std::string(type);
  if(_rank > _mu.size()) {
    throwError("The  provided rank  is larger than the given problem's dimension, it should be less or equal!" );
  }
  if (_rank == _mu.size()) { 
    std::vector<std::vector<double> > inverseCovMatrix (rows,std::vector< double >(columns));
    computeInverse(_cov_matrix, inverseCovMatrix);
    for (int i=0;i<rows;i++){
	    std::vector<double> temp;
	    for (int j=0;j<columns;j++)
	    temp.push_back(inverseCovMatrix.at(i).at(j));
	    _inverse_cov_matrix.push_back(temp);
    }
    _determinant_cov_matrix = getDeterminant(_cov_matrix);
  }
  //compute the svd
  computeSVD(_cov_matrix, _rank);
}

void BasicMultivariateNormal::computeSVD(std::vector<double> vecCovMatrix) {
  /**
   * This function will compute the svd for given matrix (vecCovMatrix)
   * Input
   * vecCovMatrix: the provided matrix
   */
  int rows, columns;
  std::vector<std::vector<double> > covMatrix;
  // convert the vecCovMatrix to covMatrix, output the rows and columns of the covariance matrix
  vectorToMatrix(rows,columns,vecCovMatrix,covMatrix);
  svdDecomposition(covMatrix,_leftSingularVectors,_rightSingularVectors,_singularValues,_svdTransformedMatrix);
}

void BasicMultivariateNormal::computeSVD(std::vector<double> vecCovMatrix, unsigned int rank) {
  /**
   * This function will compute the truncated svd for given matrix (vecCovMatrix)
   * Input
   * vecCovMatrix: the provided matrix
   * rank: the number of singular values that will be kept for the truncated svd
   */
  int rows, columns;
  std::vector<std::vector<double> > covMatrix;
  // convert the vecCovMatrix to covMatrix, output the rows and columns of the covariance matrix
  vectorToMatrix(rows,columns,vecCovMatrix,covMatrix);
  svdDecomposition(covMatrix,_leftSingularVectors,_rightSingularVectors,_singularValues, _svdTransformedMatrix,rank);
}

void BasicMultivariateNormal::computeSVD(std::vector<std::vector<double> > vecCovMatrix) {
  /**
   * This function will compute the svd for given matrix (vecCovMatrix)
   * Input
   * vecCovMatrix: the provided matrix
   */
  svdDecomposition(vecCovMatrix,_leftSingularVectors,_rightSingularVectors,_singularValues,_svdTransformedMatrix);
}

void BasicMultivariateNormal::computeSVD(std::vector<std::vector<double> > vecCovMatrix, int rank) {
  /**
   * This function will compute the svd for given matrix (vecCovMatrix)
   * Input
   * vecCovMatrix: the provided matrix
   * rank: the number of singular values that will be kept for the truncated svd
   */
  svdDecomposition(vecCovMatrix,_leftSingularVectors,_rightSingularVectors,_singularValues,_svdTransformedMatrix, rank);
}

std::vector<double> BasicMultivariateNormal::getLeftSingularVectors() {
  /**
   * this function returns the left singular vectors
   * input/output
   * row: the row dimension of the left singular vectors
   * col: the column dimension of the left singular vectors
   * output
   * tempVectors: the vector stores the left singular vectors
   */
  std::vector<double> returnVectors;
  for(unsigned int i = 0; i < _leftSingularVectors.size(); ++i) {
    for(unsigned int j = 0; j < _leftSingularVectors.at(0).size(); ++j) {
      returnVectors.push_back(_leftSingularVectors.at(i).at(j));
    }
  }
  return returnVectors;
}

std::vector<double> BasicMultivariateNormal::getRightSingularVectors() {
  /**
   * this function returns the right singular vectors
   * input/output
   * row: the row dimension of the right singular vectors
   * col: the column dimension of the left singular vectors
   * output
   * tempVectors: the vector stores the right singular vectors
   */
  std::vector<double> returnVectors;
  for(unsigned int i = 0; i < _rightSingularVectors.size(); ++i) {
    for(unsigned int j = 0; j < _rightSingularVectors.at(0).size(); ++j) {
      returnVectors.push_back(_rightSingularVectors.at(i).at(j));
    }
  }
  return returnVectors;
}

std::vector<double> BasicMultivariateNormal::getSingularValues() {
  /**
   * this function returns the singular values
   * input/output
   * dim: the dimension of the singular values
   * output
   * _singularValues: the vector stores the singular values
   */
  return _singularValues;
}

std::vector<int> BasicMultivariateNormal::getLeftSingularVectorsDimensions() {
  /**
   * return the row and column of left singular vectors stored in tempVector.at(0) and tempVector.at(1) respectively
   */
  std::vector<int> returnVector;
  returnVector.push_back(_leftSingularVectors.size());
  returnVector.push_back(_leftSingularVectors.at(0).size());
  return returnVector;
}

std::vector<int> BasicMultivariateNormal::getRightSingularVectorsDimensions() {
  /**
   * return the row and column of right singular vectors stored in tempVector.at(0) and tempVector.at(1) respectively
   */
  std::vector<int> returnVector;
  returnVector.push_back(_rightSingularVectors.size());
  returnVector.push_back(_rightSingularVectors.at(0).size());
  return returnVector;
}

int  BasicMultivariateNormal::getSingularValuesDimension() {
  /**
   * return the dimension of  singular value vector stored
   */
  return _singularValues.size();
}

std::vector<double> BasicMultivariateNormal::coordinateInTransformedSpace(int rank) {
  /**
   * This function will return the coordinate in the transformed space
   * rank: the effective dimension of the transformed space
   * This function will generate the coordinate for r (r=rank) random variables, each of them
   * are drew from single normal distribution. We need a random number between 0 and 1 to drew
   * the random variable. In addition, thi function will be used in the input dimensionality reduction
   * application. We will transform the correlated variables into uncorrelated variables, and using this
   * function to draw the samples for the uncorrelated variables, and later transform the samples to correlated
   * variables.
   */
  //std::cout << "BasicMultivariateNormal::coordinateInTransformedSpace" << std::endl;
  std::vector<double> coordinate;
  BasicNormalDistribution * normalDistribution = new BasicNormalDistribution(0,1);
  DistributionContainer *distributionInstance = & DistributionContainer::Instance();
  double randValue = 0.0;
  for(int i = 0; i < rank; ++i) {
    randValue = distributionInstance->random();
    double coordinateValue = normalDistribution->InverseCdf(randValue);
    coordinate.push_back(coordinateValue);
  }
  delete normalDistribution;
  return coordinate;
}

std::vector<double> BasicMultivariateNormal::coordinateInverseTransformed(std::vector<double> & coordinate) {
  /**
   * This function will transform the coordinate back to the original space
   * coordinate: the coordinate in the transformed space
   */
  //std::cout << "BasicMultivariateNormal::coordinateInverseTransformed" << std::endl;
  std::vector<double> originalCoordinate;
  for(unsigned int irow = 0; irow < _svdTransformedMatrix.size(); ++irow) {
    double tempSum = 0.0;
    for(unsigned int icol = 0; icol < _svdTransformedMatrix.at(0).size(); ++icol) {
      tempSum = tempSum + _svdTransformedMatrix.at(irow).at(icol) * coordinate.at(icol);
    }
    originalCoordinate.push_back(tempSum);
  }
  if(_covarianceType == "abs") {
    for(unsigned int idim = 0; idim < originalCoordinate.size(); ++idim) {
      originalCoordinate.at(idim) += _mu.at(idim); 
    } 
  } else if (_covarianceType == "rel") {
    for(unsigned int idim = 0; idim < originalCoordinate.size(); ++idim) {
      originalCoordinate.at(idim) = _mu.at(idim)*(1.0 + originalCoordinate.at(idim));
    } 
  } else {
    throwError("MultivariateNormal Error: covariance type is not available");
  }
  return originalCoordinate;
}

double BasicMultivariateNormal::getPdf(std::vector<double> x, std::vector<double> mu, std::vector<std::vector<double> > inverse_cov_matrix){
  /**
   * This function calculates the pdf values at x of a MVN distribution
   */

	double value = 0;

   if(mu.size() == x.size()){
	   int dimensions = mu.size();
	   double expTerm=0;
	   std::vector<double> tempVector (dimensions);
	   for(int i=0; i<dimensions; i++){
		   tempVector[i]=0;
		   for(int j=0; j<dimensions; j++)
			   tempVector[i] += inverse_cov_matrix[i][j]*(x[j]-mu[j]);
		   expTerm += tempVector[i]*(x[i]-mu[i]);
	   }
	   value = 1/sqrt(_determinant_cov_matrix*pow(2*M_PI,dimensions))*exp(-0.5*expTerm);
   }else
	   throwError("MultivariateNormal PDF error: evaluation point dimensionality is not correct");
   return value;
}

double BasicMultivariateNormal::pdfInTransformedSpace(std::vector<double> x){
	/**
	 * This function calculates the pdf values at x in the PCA transformed space
   * x: the coordinate in the transformed space
	 */
  double value = 1.0;
  BasicNormalDistribution * normalDistribution = new BasicNormalDistribution(0,1);
  if (x.size() == _rank) {
    for (unsigned int i = 0; i < x.size(); ++i) {
      value *=  normalDistribution->Pdf(x.at(i));
    }
  } else {
    throwError("MultivariateNormal PDF in the PCA transformed space error: evaluate point dimensionality is not correct! ")
  }
  delete normalDistribution;
	return value;
}

double BasicMultivariateNormal::Pdf(std::vector<double> x){
	  /**
	   * This function calculates the pdf values at x of a MVN distribution
	   */
	return getPdf(x, _mu, _inverse_cov_matrix);
}

double BasicMultivariateNormal::Cdf(std::vector<double> x){
	  /**
	   * This function calculates the Cdf values at x of a MVN distribution
	   */
	return _cartesianDistribution.Cdf(x);
}

std::vector<double> BasicMultivariateNormal::InverseCdf(double F, double g){
	  /**
	   * This function calculates the inverse CDF values at F of a MVN distribution
	   */
	std::cout<<"BasicMultivariateNormal::InverseCdf"<< std::endl;
	return _cartesianDistribution.InverseCdf(F,g);
  std::cout << "test inverseCdf" << std::endl;
}

double BasicMultivariateNormal::inverseMarginal(double F, int dimension){
	  /**
	   * This function calculates the inverse marginal distribution at F for a specific dimension of a MVN distribution
	   */
	return _cartesianDistribution.inverseMarginal(F,dimension);
}

int BasicMultivariateNormal::returnDimensionality(){
	  /**
	   * This function returns the dimensionality of a MVN distribution
	   */
	return _mu.size();
}

void BasicMultivariateNormal::updateRNGparameter(double tolerance, double initial_divisions){
	  /**
	   * This function updates the random number generator parameters of a MVN distribution
	   */
	return _cartesianDistribution.updateRNGparameter(tolerance,initial_divisions);
}

double BasicMultivariateNormal::Marginal(double x, int dimension){
	  /**
	   * This function calculates the marginal distribution at x for a specific dimension of a MVN distribution
	   */
	return _cartesianDistribution.Marginal(x,dimension);
}

//double BasicMultivariateNormal::Cdf_(std::vector<double> x){
//// if(_mu.size() == x.size()){
////  int dimensions = _mu.size();
////  //boost::math::chi_squared chiDistribution(dimensions);
////
////  double mahalanobis=0.0;
////  std::vector<double> tempVector (dimensions);
////  for(int i=0; i<dimensions; i++)
////   tempVector[i]=0.0;
////
////  for(int i=0; i<dimensions; i++){
////   tempVector[i]=0.0;
////   for(int j=0; j<dimensions; j++)
////    tempVector[i] += _inverseCovMatrix[i][j]*(x[j]-_mu[j]);
////   mahalanobis += tempVector[i]*(x[i]-_mu[i]);
////  }
////  value = boost::math::gamma_p<double,double>(dimensions/2,mahalanobis/2);
//// }else
////  throwError("MultivariateNormal CDF error: evaluation point dimensionality is not correct");
//
// double alpha = 2.5;
// int Nmax = 50;
// double epsilon= 0.01;
// double delta;
//
// int dimensions = _cov_matrix.size();
// double Intsum=0;
// double Varsum=0;
// int N = 0;
// double error=10*epsilon;
// std::vector<double> d (dimensions);
// std::vector<double> e (dimensions);
// std::vector<double> f (dimensions);
//
// d[0] = 0.0;
// e[0] = phi(x[0]/_cholesky_C[0][0]);
// f[0] = e[0] - d[0];
//
// boost::random::mt19937 rng;
// rng.seed(time(NULL));
// double range = rng.max() - rng.min();
//
// while (error>epsilon or N<Nmax){
//  std::vector<double> w (dimensions-1);
//
//  for (int i=0; i<(dimensions-1); i++){
//   w.at(i) = (rng()-rng.min())/range;
//   //std::cout<< "value: " << w.at(i) << std::endl;
//  }
//
//  std::vector<double> y (dimensions-1);
//
//  for (int i=1; i<dimensions; i++){
//   double tempY = d.at(i-1) + w.at(i-1) * (e.at(i-1)-d.at(i-1));
//
//   y.at(i-1) = phi_inv(tempY);
//
//   double tempE = x.at(i);
//
//   for (int j=0; j<(i-1); j++)
//    tempE = tempE - _cholesky_C[i][j] * y.at(j) / _cholesky_C[i][i];
//
//   e.at(i)=phi(tempE);
//   d.at(i)=0.0;
//   f.at(i)=(e.at(i)-d.at(i))*f.at(i-1);
//  }
//
//  N++;
//  delta = (f.at(dimensions-1)-Intsum)/double(N);
//  Intsum = Intsum + delta;
//  Varsum = (double(N-2))*Varsum/double(N) + delta*delta;
//  error = alpha * sqrt(Varsum);
//
//  std::cout << "N " << N << " ; f: " << f.at(dimensions-1) << " ; delta: " << delta << " ; Intsum: " << Intsum << " ; Varsum: " << Varsum << "; error: " << error << std::endl;
// }
//
// return Intsum;
//}

BasicMultivariateNormal::~BasicMultivariateNormal(){

}

double BasicMultivariateNormal::phi(double x){
 double value = 0.5 * (1.0 + boost::math::erf<double>(x/sqrt(2.0)));
 //double value = 0.5 * (boost::math::erf<double>(x/sqrt(2)));
 return value;
}


double BasicMultivariateNormal::phi_inv(double x){
 normal s;
 double value = quantile(s,x);
 return value;
}


//double BasicMultivariateNormal::rn(){
//    boost::random::mt19937 rng;
// rng.seed(time(NULL));
// double range = rng.max() - rng.min();
// double value = (rng()-rng.min())/range;
// std::cout<< "value: " << value << std::endl;
// return value;
//}

//double BasicMultivariateNormal::MVNDST(std::vector<double> a, std::vector<double> b, double alpha, double epsilon, int Nmax){
// int dimensions = _covMatrix.size();
// double Intsum=0;
// double Varsum=0;
// int N = 0;
// double error;
// std::vector<double> d (dimensions);
// std::vector<double> e (dimensions);
// std::vector<double> f (dimensions);
//
// std::vector<std::vector<double> > cholesky_C = choleskyDecomposition(_covMatrix);
//
// d[0] = phi(a[0]/cholesky_C[0][0]);
// e[0] = phi(b[0]/cholesky_C[0][0]);
// f[0] = e[0] - d[0];
//
//    boost::random::mt19937 rng;
// rng.seed(time(NULL));
// double range = rng.max() - rng.min();
//
// do{
//  std::vector<double> w (dimensions-1);
//  for (int i=0; i<(dimensions-1); i++){
//   w.at(i) = (rng()-rng.min())/range;
//   std::cout<< "value: " << rng() << std::endl;
//  }
//
//  std::vector<double> y (dimensions-1);
//  for (int i=1; i<dimensions; i++){
//   double tempY = d.at(i-1) + w.at(i-1)*(e.at(i-1)-d.at(i-1));
//   y.at(i-1) = phi_inv(tempY);
//
//   double tempD = a.at(i);
//   double tempE = b.at(i);
//
//   for (int j=0; j<(i-1); j++){
//    tempD = tempD - cholesky_C[i][j] * y[j] / cholesky_C[i][i];
//    tempE = tempE - cholesky_C[i][j] * y[j] / cholesky_C[i][i];
//   }
//
//   d[i]=phi(tempD);
//   e[i]=phi(tempE);
//   f[i]=(e[i]-d[i])/f[i-1];
//  }
//
//  N++;
//  double delta = (f[dimensions-1]-Intsum)/N;
//  Intsum = Intsum + delta;
//  Varsum = (N-2)*Varsum/N + delta*delta;
//  error = alpha * sqrt(Varsum);
//
// } while (error<epsilon and N<Nmax);
//
// return Intsum;
//}

//http://rosettacode.org/wiki/Cholesky_decomposition#C
double *BasicMultivariateNormal::cholesky(double *A, int n) {
    double *L = (double*)calloc(n * n, sizeof(double));
    if (L == NULL)
        exit(EXIT_FAILURE);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < (i+1); j++) {
            double s = 0;
            for (int k = 0; k < j; k++)
                s += L[i * n + k] * L[j * n + k];
            L[i * n + j] = (i == j) ?
                           sqrt(A[i * n + i] - s) :
                           (1.0 / L[j * n + j] * (A[i * n + j] - s));
        }

    return L;
}

std::vector<std::vector<double> > BasicMultivariateNormal::choleskyDecomposition(std::vector<std::vector<double> > matrix){
 std::vector<std::vector<double> > cholesky_C;

 int dimensions = matrix.size();
 double m1[dimensions*dimensions];

 for (int r=0; r<dimensions; r++)
  for (int c=0; c<dimensions; c++)
   m1[r*dimensions+c] = matrix[r][c];

 double *c1 = cholesky(m1, dimensions);
 //std::cout << "choleskyDecomposition" << std::endl;
 //show_matrix(c1,dimensions);

 for (int r=0; r<dimensions; r++){
  std::vector<double> temp;
  for (int c=0; c<dimensions; c++)
   temp.push_back(c1[r*dimensions+c]);
  cholesky_C.push_back(temp);
 }
 return cholesky_C;
}

void BasicMultivariateNormal::show_matrix(double *A, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            printf("%2.5f ", A[i * n + j]);
        printf("\n");
    }
}

//template<class T>
//bool InvertMatrix(const matrix<T>& input, matrix<T>& inverse)
//{
// typedef permutation_matrix<std::size_t> pmatrix;
//
// // create a working copy of the input
// matrix<T> A(input);
//
// // create a permutation matrix for the LU-factorization
// pmatrix pm(A.size1());
//
// // perform LU-factorization
// int res = lu_factorize(A, pm);
// if (res != 0)
//  return false;
//
// // create identity matrix of "inverse"
// inverse.assign(identity_matrix<T> (A.size1()));
//
// // backsubstitute to get the inverse
// lu_substitute(A, pm, inverse);
//
// return true;
//}
//
//void getInverse(std::vector<std::vector<double> > matrix, std::vector<std::vector<double> > inverse_matrix){
// int dimension = matrix.size();
// double initialValues[dimension][dimension];
// matrix<double> A(dimension, dimension), Z(dimension, dimension);
//
// for(int i=0; i<dimension; i++)
//  for(int j=0; j<dimension; j++)
//   initialValues[i][j]=matrix[i][j];
// A = make_matrix_from_pointer(initialValues);
// InvertMatrix(A, Z);
//
// for(int i=0; i<dimension; i++){
//  std::vector<double> temp;
//  for(int j=0; j<dimension; j++)
//   temp.push_back(Z[i][j]);
//  inverse_matrix.push_back(temp);
// }
//}
