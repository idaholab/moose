#ifndef DISTRIBUTIONFUNCTIONS_H
#define DISTRIBUTIONFUNCTIONS_H

#include <sstream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>
#include <iostream>
#include <stdio.h>
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
void inverseMatrix(double* A, int N);
void computeInverse(const std::vector<std::vector<double> > & matrix, std::vector<std::vector<double> > & inverse);
// convert a matrix stored in a vector to a matrix stored in a vector of vector
void vectorToMatrix(unsigned int &rows, unsigned int & columns, std::vector<double> &vecMatrix, std::vector<std::vector<double> > &_cov_matrix);
double getDeterminant(std::vector<std::vector<double> > matrix);

// functions for singular value decomposition
void svdDecomposition(const std::vector<std::vector<double> > &matrix, std::vector<std::vector<double> > &leftSingularVectors, std::vector<std::vector<double> > &rightSingularVectors, std::vector<double> &singularValues, std::vector<std::vector<double> > &transformedMatrix);
void svdDecomposition(const std::vector<std::vector<double> > &matrix, std::vector<std::vector<double> > &leftSingularVectors, std::vector<std::vector<double> > &rightSingularVectors, std::vector<double> &singularValues, std::vector<std::vector<double> > &transformedMatrix, unsigned int rank);
void matrixConversionToEigenType(std::vector<std::vector<double> > original, Eigen::MatrixXd &converted);
void matrixConversionToCxxVVectorType(const Eigen::MatrixXd & original, std::vector<std::vector<double> > &converted);
void vectorConversionToCxxVectorType(const Eigen::VectorXd & original, std::vector<double>  &converted);
void computeNearestSymmetricMatrix(const std::vector<std::vector<double> > &matrix, std::vector<std::vector<double> > &symmetricMatrix); 
void resetSingularValues(std::vector<std::vector<double> > &leftSingularVectors, std::vector<std::vector<double> > &rightSingularVectors, std::vector<double> &singularValues, std::vector<std::vector<double> > &transformedMatrix);

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

void LoadData(double** data, int dimensionality, int cardinality, std::string filename);

double calculateCustomPdf(double position, double fitting, double** dataSet, int numberSamples);

double calculateCustomCDF(double position, double fitting, double** dataSet, int numberSamples);

double rk_gauss();

double STDgammaRNG(double shape);

double gammaRNG(double shape, double scale);

double betaRNG(double alpha, double beta);

double ModifiedLogFunction(double x);

double AbramStegunApproximation(double t);


#endif /* DISTRIBUTIONFUNCTIONS_H */
