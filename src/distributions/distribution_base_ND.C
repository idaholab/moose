/*
 * distribution.C
 *
 *  Created on: Feb 6, 2014
 *      Author: alfoa
 *
 */

#include "distribution_base_ND.h"
//#include "DistributionContainer.h"
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

std::vector<double> DistributionInverseCdf(BasicDistributionND & dist, double & min, double & max){
 //return dist.InverseCdf(min, max);
 //return std::vector<double>(2,-1.0);
 //return dist.InverseCdf((max-min)/2.0, (max-min), 10);
  return dist.InverseCdf((max-min)/2.0);
}

BasicMultivariateNormal::BasicMultivariateNormal(std::string data_filename, std::vector<double> mu){
  _mu = mu;

  int rows,columns;
  readMatrix(data_filename, rows, columns, _cov_matrix);
  std::vector<std::vector<double> > inverseCovMatrix (rows,std::vector< double >(columns));

  std::cerr<< "Computing inverse and determinant"<< std::endl;

  computeInverse(_cov_matrix, inverseCovMatrix);

  for (int i=0;i<rows;i++){
   std::vector<double> temp;
     for (int j=0;j<columns;j++)
      temp.push_back(inverseCovMatrix[i][j]);
     _inverse_cov_matrix.push_back(temp);
  }

  int dimensions = _mu.size();
  for(int i=0; i<dimensions; i++)
   for(int j=0; j<dimensions; j++)
    std::cerr<<_inverse_cov_matrix[i][j]<<std::endl;

  std::cerr<< "_inverseCovMatrix completed"<< std::endl;

  _determinant_cov_matrix = getDeterminant(_cov_matrix);
  std::cerr<< "_determinantCovMatrix completed";

  if(rows != columns)
          throwError("MultivariateNormal error: covariance matrix in " << data_filename << " is not a square matrix.");
}

BasicMultivariateNormal::BasicMultivariateNormal(const char * data_filename, std::vector<double> mu){
   //std::string str(data_filename);
   //std::cerr<< data_filename << std::endl;

   //std::stringstream ss;
   //ss.str(data_filename);
   //std::string filename = ss.str();
   //std::cerr<< filename << std::endl;
   //std::cerr<< typeof(filename) << std::endl;

   _mu = mu;

   int rows,columns;
   readMatrix(std::string(data_filename), rows, columns, _cov_matrix);
   //readMatrix(data_filename, rows, columns, _cov_matrix);
   std::vector<std::vector<double> > inverseCovMatrix (rows,std::vector< double >(columns));

   std::cerr<< "Computing inverse and determinant"<< std::endl;

   computeInverse(_cov_matrix, inverseCovMatrix);

   for (int i=0;i<rows;i++){
    std::vector<double> temp;
      for (int j=0;j<columns;j++)
       temp.push_back(inverseCovMatrix.at(i).at(j));
      _inverse_cov_matrix.push_back(temp);
   }

   int dimensions = _mu.size();
   for(int i=0; i<dimensions; i++)
    for(int j=0; j<dimensions; j++)
     std::cerr<<_inverse_cov_matrix[i][j]<<std::endl;

   std::cerr<< "_inverseCovMatrix completed"<< std::endl;

   _determinant_cov_matrix = getDeterminant(_cov_matrix);

   std::cerr<< "_determinantCovMatrix completed";

   std::cout<< "choleskyDecomposition initiated";
   _cholesky_C = choleskyDecomposition(_cov_matrix);
   std::cout<< "choleskyDecomposition completed";

   if(rows != columns)
           throwError("MultivariateNormal error: covariance matrix in " << data_filename << " is not a square matrix.");
}


BasicMultivariateNormal::BasicMultivariateNormal(std::vector<std::vector<double> > covMatrix, std::vector<double> mu){

  _mu = mu;
  _cov_matrix = covMatrix;

  computeInverse(_cov_matrix, _inverse_cov_matrix);

  std::cerr<< "_inverseCovMatrix completed";

  _determinant_cov_matrix = getDeterminant(_cov_matrix);
  std::cerr<< "_determinantCovMatrix completed";
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
     tempVector[i] += _inverse_cov_matrix[i][j]*(x[j]-_mu[j]);
    expTerm += tempVector[i]*(x[i]-_mu[i]);
   }
   value = 1/sqrt(_determinant_cov_matrix*pow(2*M_PI,dimensions))*exp(-0.5*expTerm);
        }else
         throwError("MultivariateNormal PDF error: evaluation point dimensionality is not correct");

        return value;
}

double BasicMultivariateNormal::Cdf(std::vector<double> x){
 double value;

// if(_mu.size() == x.size()){
//  int dimensions = _mu.size();
//  //boost::math::chi_squared chiDistribution(dimensions);
//
//  double mahalanobis=0.0;
//  std::vector<double> tempVector (dimensions);
//  for(int i=0; i<dimensions; i++)
//   tempVector[i]=0.0;
//
//  for(int i=0; i<dimensions; i++){
//   tempVector[i]=0.0;
//   for(int j=0; j<dimensions; j++)
//    tempVector[i] += _inverseCovMatrix[i][j]*(x[j]-_mu[j]);
//   mahalanobis += tempVector[i]*(x[i]-_mu[i]);
//  }
//  value = boost::math::gamma_p<double,double>(dimensions/2,mahalanobis/2);
// }else
//  throwError("MultivariateNormal CDF error: evaluation point dimensionality is not correct");

 double alpha = 2.5;
 int Nmax = 50;
 double epsilon= 0.01;
 double delta;

 int dimensions = _cov_matrix.size();
 double Intsum=0;
 double Varsum=0;
 int N = 0;
 double error=10*epsilon;
 std::vector<double> d (dimensions);
 std::vector<double> e (dimensions);
 std::vector<double> f (dimensions);

 d[0] = 0.0;
 e[0] = phi(x[0]/_cholesky_C[0][0]);
 f[0] = e[0] - d[0];

 boost::random::mt19937 rng;
 rng.seed(time(NULL));
 double range = rng.max() - rng.min();

 while (error>epsilon or N<Nmax){
  std::vector<double> w (dimensions-1);

  for (int i=0; i<(dimensions-1); i++){
   w.at(i) = (rng()-rng.min())/range;
   //std::cout<< "value: " << w.at(i) << std::endl;
  }

  std::vector<double> y (dimensions-1);

  for (int i=1; i<dimensions; i++){
   double tempY = d.at(i-1) + w.at(i-1) * (e.at(i-1)-d.at(i-1));

   y.at(i-1) = phi_inv(tempY);

   double tempE = x.at(i);

   for (int j=0; j<(i-1); j++)
    tempE = tempE - _cholesky_C[i][j] * y.at(j) / _cholesky_C[i][i];

   e.at(i)=phi(tempE);
   d.at(i)=0.0;
   f.at(i)=(e.at(i)-d.at(i))*f.at(i-1);
  }

  N++;
  delta = (f.at(dimensions-1)-Intsum)/double(N);
  Intsum = Intsum + delta;
  Varsum = (double(N-2))*Varsum/double(N) + delta*delta;
  error = alpha * sqrt(Varsum);

  std::cout << "N " << N << " ; f: " << f.at(dimensions-1) << " ; delta: " << delta << " ; Intsum: " << Intsum << " ; Varsum: " << Varsum << "; error: " << error << std::endl;
 }

 return Intsum;
}

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
 //std::cout << "x: " << x << " ; value: " << value << std::endl;
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
 std::cout << "choleskyDecomposition" << std::endl;
 show_matrix(c1,dimensions);

 for (int r=0; r<dimensions; r++){
  std::vector<double> temp;
  for (int c=0; c<dimensions; c++)
   temp.push_back(c1[r*dimensions+c]);
  cholesky_C.push_back(temp);
 }

 return cholesky_C;
}

void BasicMultivariateNormal::show_matrix(double *A, int n) {
 printf("Cholesky Decompostion");
 printf("\n");
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
