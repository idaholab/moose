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
#ifndef DISTRIBUTIONFUNCTIONS_H
#define DISTRIBUTIONFUNCTIONS_H

#include <sstream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>
#include <iostream>
#include <cmath>   // to use erfc error function
#include <ctime>   // for rand() and srand()

//#include <Eigen/Dense>
#include <Eigen/SVD>

/*
 *  distributionFunctions
 *      source: Numerical Recipes in C++ 3rd edition
 */

//typedef boost::numeric::ublas::matrix<double> matrixDouble;

void matrixConversion(std::vector<std::vector<double> > original, double converted[]);
void matrixBackConversion(double original[], std::vector<std::vector<double> > converted);
//void inverseMatrix(double* a, int n);
void computeInverse(const std::vector<std::vector<double> > & matrix, std::vector<std::vector<double> > & inverse);
// convert a matrix stored in a vector to a matrix stored in a vector of vector
void vectorToMatrix(unsigned int &rows, unsigned int & columns, std::vector<double> &vec_matrix, std::vector<std::vector<double> > &cov_matrix);
double getDeterminant(std::vector<std::vector<double> > matrix);

// functions for singular value decomposition
void getInverseTransformedMatrix(const std::vector<std::vector<double> > &left_singular_vectors, std::vector<double> &singular_values, std::vector<std::vector<double> > &inverse_transformed_matrix);
void svdDecomposition(const std::vector<std::vector<double> > &matrix, std::vector<std::vector<double> > &left_singular_vectors, std::vector<std::vector<double> > &right_singular_vectors, std::vector<double> &singular_values, std::vector<std::vector<double> > &transformed_matrix);
void svdDecomposition(const std::vector<std::vector<double> > &matrix, std::vector<std::vector<double> > &left_singular_vectors, std::vector<std::vector<double> > &right_singular_vectors, std::vector<double> &singular_values, std::vector<std::vector<double> > &transformed_matrix, unsigned int rank);
void matrixConversionToEigenType(std::vector<std::vector<double> > original, Eigen::MatrixXd &converted);
void matrixConversionToCxxVVectorType(const Eigen::MatrixXd & original, std::vector<std::vector<double> > &converted);
void vectorConversionToCxxVectorType(const Eigen::VectorXd & original, std::vector<double>  &converted);
void computeNearestSymmetricMatrix(const std::vector<std::vector<double> > &matrix, std::vector<std::vector<double> > &symmetric_matrix);
void resetSingularValues(std::vector<std::vector<double> > &left_singular_vectors, std::vector<std::vector<double> > &right_singular_vectors, std::vector<double> &singular_values, std::vector<std::vector<double> > &transformed_matrix);

void nrerror(const char error_text[]);

double gammp(double a, double x);

double loggam(double xx);


#define ITMAX 100
#define EPSW 3.0e-7

void gser(double *gamser,double a,double x,double *gln);

#define FPMIN 1.0e-30

void gcf(double *gammcf,double a,double x,double *gln);

double gammaFunc(double x);

double betaFunc(double alpha, double beta);

double logGamma(double x);

double betaContFrac(double a, double b, double x);

double betaInc(double a, double b, double x);

double normRNG(double mu, double sigma, double RNG);

//void loadData(double** data, int dimensionality, int cardinality, std::string filename);

//double calculateCustomPdf(double position, double fitting, double** data_set, int number_samples);

//double calculateCustomCDF(double position, double fitting, double** data_set, int number_samples);

//double rkGauss();

//double stdGammaRNG(double shape);

double gammaRNG(double shape, double scale);

double betaRNG(double alpha, double beta);

//double modifiedLogFunction(double x);

double abramStegunApproximation(double t);


#endif /* DISTRIBUTIONFUNCTIONS_H */
