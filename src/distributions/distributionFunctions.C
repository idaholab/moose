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
/*
 *
 *  Created on: April 28, 2012
 *      Author: MANDD
 *
 *      Tests  : None for the custom
 *
 *      Problems : None
 *      Issues  : None
 *      Complaints : None
 *      Compliments : None
 *
 *      source: Numerical Recipes in C++ 3rd edition
 *
 */

#include <sstream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>
#include <iostream>
#include <cmath> // to use erfc error function
#include <ctime> // for rand() and srand()
#include <cstdio>

#include "distribution_1D.h"
#include "distributionFunctions.h"

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/lu.hpp>

//#include <Eigen/Dense>
#include <Eigen/SVD>
#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

#define _USE_MATH_DEFINES

/*
extern "C" {
    // LU decomoposition of a general matrix
    void dgetrf_(int* M, int *N, double* A, int* lda, int* IPIV, int* INFO);

    // generate inverse of a matrix given its LU decomposition
    void dgetri_(int* N, double* A, int* lda, int* IPIV, double* WORK, int* lwork, int* INFO);
}
*/

typedef boost::numeric::ublas::matrix<double> matrixDouble;

void matrixConversionBoost(const std::vector<std::vector<double> > & original, matrixDouble & converted)
{
  converted.resize(original.size(),original.at(0).size());
  for(unsigned int r=0; r < original.size(); r++)
  {
    for(unsigned int c=0; c < original.at(r).size(); c++)
    {
      converted(r,c)  = original.at(r).at(c);
    }
  }
}

/*
void matrixConversion(std::vector<std::vector<double> > original, double converted[]){
        if (original.size() == original[0].size()){
                int dimensions = original.size();
                for (int r=0; r<dimensions; r++)
                        for (int c=0; c<dimensions; c++){
                                converted[r*dimensions+c] = original[r][c];
                        }
        }else
                throwError("Error in matrixConversion: matrix is not squared.");
}
*/

void matrixBackConversionBoost(const matrixDouble & original, std::vector<std::vector<double> > & converted)
{
  for(unsigned int r=0; r < original.size1(); r++)
  {
    for(unsigned int c = 0; c < original.size2(); c++)
    {
      converted.at(r).at(c) = original(r,c);
    }
  }
}

/*
void matrixBackConversion(double original[], std::vector<std::vector<double> > converted){
        int dimensions = int(sizeof(original)/sizeof(double));
        dimensions = sqrt(dimensions);

        for (int r=0; r<dimensions; r++)
                for (int c=0; c<dimensions; c++)
                        converted[r][c] = original[r*dimensions+c];
}
*/

/*
//http://stackoverflow.com/questions/3519959/computing-the-inverse-of-a-matrix-using-lapack-in-c
void inverseMatrix(double* a, int n)
{
    int *IPIV = new int[n+1];
    int LWORK = n*n;
    double *WORK = new double[LWORK];
    int INFO;

    dgetrf_(&n,&n,a,&n,IPIV,&INFO);
    dgetri_(&n,a,&n,IPIV,WORK,&LWORK,&INFO);

    delete IPIV;
    delete WORK;
}
*/

//Roughly based on http://savingyoutime.wordpress.com/2009/09/21/c-matrix-inversion-boostublas/ and libs/numeric/ublas/test/test_lu.cpp
void invertMatrixBoost(matrixDouble & a, matrixDouble & aInverted)
{
  boost::numeric::ublas::permutation_matrix<std::size_t> pm(a.size1());

  boost::numeric::ublas::lu_factorize<matrixDouble,
                                      boost::numeric::ublas::permutation_matrix<> >(a, pm);

  aInverted.assign(boost::numeric::ublas::identity_matrix<double>(a.size1()));

  boost::numeric::ublas::lu_substitute(a, pm, aInverted);
}

void computeInverse(const std::vector<std::vector<double> > & matrix, std::vector<std::vector<double> > & inverse){
        int dimensions = matrix.size();
        matrixDouble A(dimensions,dimensions),inverted(dimensions,dimensions);
        matrixConversionBoost(matrix, A);
        invertMatrixBoost(A,inverted);
        matrixBackConversionBoost(inverted, inverse);
}

// Convert the vector of covariance to vector of vector of covariance
void  vectorToMatrix(unsigned int &rows,unsigned int &columns,std::vector<double> &vec_matrix, std::vector<std::vector<double> > &cov_matrix) {
        /** Input Parameter
         * vec_matrix: covariance matrix stored in a vector
         * Output Parameter
         * rows: the first dimension of the covariance matrix
         * columns: the second dimension of the covariance matrix
         * cov_matrix: covariance matrix stored in vector<vector<double> >
         */
        unsigned int dimensions = vec_matrix.size();
        dimensions = std::lround(std::sqrt(dimensions));
        rows = dimensions;
        columns = dimensions;
        if(rows*columns != vec_matrix.size())
                      throwError("MultivariateNormal error: covariance matrix in is not a square matrix.");
        for (unsigned int row = 0; row < rows; ++row) {
                std::vector<double> temp;
                for (unsigned int colm = 0; colm < columns; ++colm) {
                        temp.push_back(vec_matrix.at(colm+row*columns));
                }
                cov_matrix.push_back(temp);
        }
}

//See for example http://en.wikipedia.org/wiki/LU_decomposition or
// http://programmingexamples.net/wiki/CPP/Boost/Math/uBLAS/determinant
double getDeterminantBoost(matrixDouble & a)
{
  boost::numeric::ublas::permutation_matrix<std::size_t> pm(a.size1());
  double determinant = 1.0;

  int result = boost::numeric::ublas::lu_factorize<matrixDouble,boost::numeric::ublas::permutation_matrix<> >(a, pm);

  if(result)
  {
    return 0.0;
  }
  //det(a) = det(P^-1)det(L)det(U) = (-1)^S*(product(u_ii))
  //Where S is the number of row exchanges
  for(unsigned int i = 0; i < a.size1(); i++)
  {
    //Multiple by current diagonal entry
    determinant *= a(i,i);
    if(i != pm(i))
    {
      //Found a row exchange
      determinant *= -1;
    }

  }
  return determinant;
}

double getDeterminant(std::vector<std::vector<double> > matrix){
        int dimensions = matrix.size();
        matrixDouble A(dimensions,dimensions);

        matrixConversionBoost(matrix, A);

        return getDeterminantBoost(A);

}

/*
double getDeterminant(std::vector<std::vector<double> > matrix){
        int dimensions = matrix.size();
        double A [dimensions*dimensions];

        matrixConversion(matrix, A);

    int *IPIV = new int[dimensions+1];
    int LWORK = dimensions*dimensions;
    double *WORK = new double[LWORK];
    int INFO;

    dgetrf_(&dimensions,&dimensions,A,&dimensions,IPIV,&INFO);

    double determinant =1;

    for(int index=0; index<dimensions; index++)
        determinant *= A[index*dimensions];

    delete IPIV;
    delete WORK;

        return determinant;
}
*/

void svdDecomposition(const std::vector<std::vector<double> > &matrix, std::vector<std::vector<double> > &left_singular_vectors, std::vector<std::vector<double> > &right_singular_vectors, std::vector<double> &singular_values, std::vector<std::vector<double> > &transformed_matrix) {
  /**
   * This function compute the singular value decomposition for given matrix
   * Input Parameters
   * matrix: provided data
   * Output Parameters
   * left_singular_vectors: stores the left singular vectors for given matrix
   * right_singular_vectors: stores the right singular vectors for given matrix
   * singular_values: stores the singular values for given matrix
   * transformed_matrix: stores the transformation matrix
   */
  unsigned int row = matrix.size();
  unsigned int col = matrix.at(0).size();
  unsigned int dim = 0;
  if(row > col) {
    dim = col;
  }else {
    dim = row;
  }
  Eigen::MatrixXd A(row,col);
  Eigen::MatrixXd U(row,row);
  Eigen::MatrixXd V(col,col);
  Eigen::MatrixXd X(row,dim);
  Eigen::VectorXd S(dim);
  matrixConversionToEigenType(matrix,A);
  Eigen::JacobiSVD<Eigen::MatrixXd> svd(A,Eigen::ComputeFullU | Eigen::ComputeFullV);
  U = svd.matrixU();
  V = svd.matrixV();
  S = svd.singularValues();
  for(unsigned int i = 0; i < dim; ++i) {
    X.col(i) = U.col(i)*sqrt(S(i));
  }
  matrixConversionToCxxVVectorType(U,left_singular_vectors);
  matrixConversionToCxxVVectorType(V,right_singular_vectors);
  vectorConversionToCxxVectorType(S,singular_values);
  matrixConversionToCxxVVectorType(X,transformed_matrix);
}

void getInverseTransformedMatrix(const std::vector<std::vector<double> > &left_singular_vectors, std::vector<double> &singular_values, std::vector<std::vector<double> > &inverse_transformed_matrix) {
  /**
   * This function compute the inverse transformation matrix
   * Input Parameters
   * left_singular_vectors: stores the left singular vectors for given matrix
   * singular_values: stores the singular values for given matrix
   * Output Parameters
   * inverse_transformed_matrix: stores the inverse transformation matrix
   */
  unsigned int row = left_singular_vectors.size();
  unsigned int col = left_singular_vectors.at(0).size();
  unsigned int dim = singular_values.size();
  Eigen::MatrixXd U(row,col);
  Eigen::MatrixXd inverseX(row,dim);
  matrixConversionToEigenType(left_singular_vectors,U);
  for (unsigned int i = 0; i < dim; ++i) {
    if (singular_values.at(i) == 0) {
      inverseX.col(i) = U.col(i) * 0.0;
    } else {
      inverseX.col(i) = U.col(i) * (1.0/sqrt(singular_values.at(i)));
    }
  }
  matrixConversionToCxxVVectorType(inverseX.transpose(),inverse_transformed_matrix);
}

void svdDecomposition(const std::vector<std::vector<double> > &matrix, std::vector<std::vector<double> > &left_singular_vectors, std::vector<std::vector<double> > &right_singular_vectors, std::vector<double> &singular_values, std::vector<std::vector<double> > &transformed_matrix, unsigned int rank) {
  /**
   * This function compute the singular value decomposition for given matrix
   * Input Parameters
   * matrix: provided data
   * Output Parameters
   * left_singular_vectors: stores the left singular vectors for given matrix
   * right_singular_vectors: stores the right singular vectors for given matrix
   * singular_values: stores the singular values for given matrix
   * rank: used for truncated svd, the number of singular values that will be kept for truncated svd
   */
  unsigned int row = matrix.size();
  unsigned int col = matrix.at(0).size();
  unsigned int dim = 0;
  if(row > col) {
    dim = col;
  }else {
    dim = row;
  }
  Eigen::MatrixXd A(row,col);
  Eigen::MatrixXd U(row,row);
  Eigen::MatrixXd V(col,col);
  Eigen::MatrixXd X(row,rank);
  Eigen::VectorXd S(dim);
  matrixConversionToEigenType(matrix,A);
  Eigen::JacobiSVD<Eigen::MatrixXd> svd(A,Eigen::ComputeFullU | Eigen::ComputeFullV);
  U = svd.matrixU();
  V = svd.matrixV();
  S = svd.singularValues();
  for(unsigned int i = 0; i < rank; ++i) {
    X.col(i) = U.col(i)*sqrt(S(i));
  }
  // transform and store the matrix for the truncated svd
  matrixConversionToCxxVVectorType(U.block(0,0,row,rank),left_singular_vectors);
  matrixConversionToCxxVVectorType(V.block(0,0,col,rank),right_singular_vectors);
  vectorConversionToCxxVectorType(S.head(rank),singular_values);
  matrixConversionToCxxVVectorType(X,transformed_matrix);
}

void  computeNearestSymmetricMatrix(const std::vector<std::vector<double> > &matrix, std::vector<std::vector<double> > &symmetric_matrix) {
  /**
   * This function used to compute the nearest symmetric matrix
   * Input Parameters
   * matrix: std::vector<std::vector<double> >, given matrix
   * Output Parameters
   * symmetric_matrix: std::vector<std::vector<double> >, the computed symmetric matrix
   */
  unsigned int row = matrix.size();
  unsigned int col = matrix.at(0).size();
  if (row != col) {
    throwError("The provided matrix is not a square matrix!" );
  }
  Eigen::MatrixXd A(row,col);
  Eigen::MatrixXd B(col,row);
  matrixConversionToEigenType(matrix,A);
  B = A.transpose();
  A = (A + B)*0.5;
  matrixConversionToCxxVVectorType(A,symmetric_matrix);
}

void resetSingularValues(std::vector<std::vector<double> > &left_singular_vectors, std::vector<std::vector<double> > &right_singular_vectors, std::vector<double> &singular_values,std::vector<std::vector<double> > &transformed_matrix) {
  /**
   * used to reset singular values
   * Input Parameters
   * left_singular_vectors: std::vector<std::vector<double> >, the left singular vectors
   * right_singular_vectors: std::vector<std::vector<double> >, the right singular vectors
   * singular_values: std::vector<double>, the singular values
   * transformed_matrix: std::vector<vector<double> >, the transformation matrix
   * Output Parameters
   * singular_values: std::vector<double>, the modified singular values
   * transformed_matrix: std::vector<vector<double> >, the updated transformation matrix
   */
  unsigned int row1 = left_singular_vectors.size();
  unsigned int col1 = left_singular_vectors.at(0).size();
  unsigned int row2 = right_singular_vectors.size();
  unsigned int col2 = right_singular_vectors.at(0).size();
  unsigned int rank = transformed_matrix.at(0).size();
  if (row1 != row2 && col1 != col2) {
    throwError("The provided matrices should have the same shape!" );
  }
  Eigen::MatrixXd A(row1,col1);
  Eigen::MatrixXd B(row2,col2);
  Eigen::MatrixXd X(row1,rank);
  matrixConversionToEigenType(left_singular_vectors,A);
  matrixConversionToEigenType(right_singular_vectors,B);
  double tol;
  tol = 1.0E-15;
  if (singular_values.at(0) == 0.0) {
    throwError("The provided covariance matrix is zero matrix!")
  }
  for (unsigned int i = 0; i < singular_values.size(); ++i) {
    if ((A.col(i) + B.col(i)).lpNorm<1>()/row1 < tol) {
      singular_values.at(i) = 0.0;
    }
    if (singular_values.at(i)/singular_values.at(0) < tol) {
      singular_values.at(i) = 0.0;
    }
  }
  for(unsigned int i = 0; i < rank; ++i) {
    X.col(i) = A.col(i)*sqrt(singular_values.at(i));
  }
  transformed_matrix.clear();
  matrixConversionToCxxVVectorType(X,transformed_matrix);
}


void matrixConversionToEigenType(std::vector<std::vector<double> > original, Eigen::MatrixXd &converted) {
  /**
   * This function convert the data from type std::vector<std::vector<double> > to Eigen::MatrixXd
   * Input Parameters
   * original: provided data with the type of std::vector<std::vector<double> >
   * Output Parameters
   * converted: output data with the type of Eigen::MatrixXd
   */
  converted.resize(original.size(),original.at(0).size());
  for(unsigned int row = 0; row < original.size(); ++row) {
    for(unsigned int col = 0; col < original.at(0).size(); ++col) {
      if(original.at(0).size() != original.at(row).size()) {
        throwError("The matrix stored in the C++ vector container with different lenght of columns");
      }
      converted(row,col) = original.at(row).at(col);
    }
  }
}

void matrixConversionToCxxVVectorType(const Eigen::MatrixXd & original, std::vector<std::vector<double> > &converted) {
  /**
   * This function convert the data from type Eigen::MatrixXd to type std::vector<double>
   * Input Parameters
   * original: provided data with the type of Eigen::MatrixXd
   * Output Parameters
   * converted: output data with the type of std::vector<std::vector<double> >
   */
  for(unsigned int row = 0; row < original.rows(); ++row) {
    std::vector<double> temp;
    for(unsigned int col = 0; col < original.cols(); ++col) {
      temp.push_back(original(row,col));
    }
    converted.push_back(temp);
  }
}

void vectorConversionToCxxVectorType(const Eigen::VectorXd & original, std::vector<double> &converted) {
  /**
   * This function convert the data from type Eigen::VectorXd to type std::vector<double>
   * Input Parameters
   * original: provided data with the type of Eigen::VectorXd
   * Output Parameters
   * converted: output data with the type of std::vector<double>
   */
  for(unsigned int dim = 0; dim < original.rows(); ++dim) {
    converted.push_back(original(dim));
  }
}

// void nrerror(const char error_text[]){     // added const to avoid "warning: deprecated conversion from string constant to *char
// /* Numerical Recipes standard error handler */
//  fprintf(stderr,"Numerical Recipes run-time error...\n");
//  fprintf(stderr,"%s\n",error_text);
//  fprintf(stderr,"...now exiting to system...\n");
// }
//
// double gammp(double a, double x){
// /* high level function for incomplete gamma function */
//         void gcf(double *gammcf,double a,double x,double *gln);
//         void gser(double *gamser,double a,double x,double *gln);
//         double gamser,gammcf,gln;
//         if(x < 0.0 || a <= 0.0) nrerror("Invalid arg in gammp");
//         if(x < (a+1.0)){
// /* here I change routine so that it returns \gamma(a,x)
//         or P(a,x)-just take out comments to get P(a,x) vs \gamma(a,x)-
//         to get latter use the exp(log(.)+gln) expression */
//                gser(&gamser,a,x,&gln);
// //      return exp(log(gamser)+gln);
//                return gamser;
//         }
//         else{
//                gcf(&gammcf,a,x,&gln);
// //      return exp(log(1.0-gammcf)+gln);
//                return 1.0-gammcf;
//         }
// }
//
// double loggam(double xx)
// {
//         double x,y,tmp,ser;
//         static double cof[6]={76.18009172947146, -86.50532032941677,
//                24.01409824083091,-1.231739572450155, 0.001208650973866179,
//                -5.395239384953e-006};
//         int j;
//         y=x=xx;
//         tmp=x+5.5;
//         tmp -= (x+0.5)*log(tmp);
//         ser=1.000000000190015;
//         for(j=0;j<=5;j++) ser += cof[j]/++y;
//         return -tmp+log(2.506628274631*ser/x);
// }
//
// #define ITMAX 100
// #define EPSW 3.0e-7
//
// void gser(double *gamser,double a,double x,double *gln){
//         int n;
//         double sum,del,ap;
//         *gln=loggam(a);
//         if(x <= 0.0){
//                if(x < 0.0) nrerror("x less than 0 in routine gser");
//                *gamser=0.0;
//                return;
//         }
//         else{
//                ap=a;
//                del=sum=1.0/a;
//                for(n=1;n<=ITMAX;n++){
//                       ++ap;
//                       del *= x/ap;
//                       sum += del;
//                       if(fabs(del) < fabs(sum)*EPSW){
//    *gamser=sum*exp(-x+a*log(x)-(*gln));
//    return;
//                       }
//                }
//                nrerror("a too large, ITMAX too small in routine gser");
//                return;
//         }
// }
//
//
// #define FPMIN 1.0e-30
//
// void gcf(double *gammcf,double a,double x,double *gln){
//         int i;
//         double an,b,c,d,del,h;
//         *gln=loggam(a);
//         b=x+1.0-a;
//         c=1.0/FPMIN;
//         d=1.0/b;
//         h=d;
//         for(i=1;i<=ITMAX;i++){
//                an = -i*(i-a);
//                b += 2.0;
//                d=an*d+b;
//                if(fabs(d) < FPMIN) d=FPMIN;
//                c=b+an/c;
//                if(fabs(c) < FPMIN) c=FPMIN;
//                d=1.0/d;
//                del=d*c;
//                h *= del;
//                if(fabs(del-1.0) < EPSW) break;
//         }
//         if(i > ITMAX) nrerror("a too large, ITMAX too small in gcf");
//         *gammcf=exp(-x+a*log(x)-(*gln))*h;
// }
//
// // Gamma function
// // source http://www.crbond.com/math.htm
// double gammaFunc(double x){
//  int i,k,m;
//  double ga,gr,r,z;
//
//  static double g[] = {
//   1.0,
//   0.5772156649015329,
//                 -0.6558780715202538,
//                 -0.420026350340952e-1,
//   0.1665386113822915,
//                 -0.421977345555443e-1,
//                 -0.9621971527877e-2,
//   0.7218943246663e-2,
//                 -0.11651675918591e-2,
//                 -0.2152416741149e-3,
//   0.1280502823882e-3,
//                 -0.201348547807e-4,
//                 -0.12504934821e-5,
//   0.1133027232e-5,
//                 -0.2056338417e-6,
//   0.6116095e-8,
//   0.50020075e-8,
//                 -0.11812746e-8,
//   0.1043427e-9,
//   0.77823e-11,
//                 -0.36968e-11,
//   0.51e-12,
//                 -0.206e-13,
//                 -0.54e-14,
//   0.14e-14};
//
//  if (x > 171.0) return 1e308;    // This value is an overflow flag.
//  if (x == (int)x) {
//   if (x > 0.0) {
//    ga = 1.0;               // use factorial
//    for (i=2;i<x;i++) {
//                                 ga *= i;
//    }
//                       }
//                       else
//    ga = 1e308;
//               }
//               else {
//   if (fabs(x) > 1.0) {
//    z = fabs(x);
//    m = (int)z;
//    r = 1.0;
//    for (k=1;k<=m;k++)
//     r *= (z-k);
//    z -= m;
//   }
//   else
//    z = x;
//   gr = g[24];
//   for (k=23;k>=0;k--)
//    gr = gr*z+g[k];
//
//   ga = 1.0/(gr*z);
//   if (fabs(x) > 1.0) {
//    ga *= r;
//    if (x < 0.0)
//     ga = -M_PI/(x*ga*sin(M_PI*x));
//   }
//  }
//  return ga;
// }
//
//
// // Beta function
// double betaFunc(double alpha, double beta){
//  double value=gammaFunc(alpha)*gammaFunc(beta)/gammaFunc(alpha+beta);
//  return value;
// }
//
//
// // log gamma using the Lanczos approximation
// double logGamma(double x) {
// const double c[8] = { 676.5203681218851, -1259.1392167224028,
//                       771.32342877765313, -176.61502916214059,
//                       12.507343278686905, -0.13857109526572012,
//                       9.9843695780195716e-6, 1.5056327351493116e-7 };
// double sum = 0.99999999999980993;
// double y = x;
// for (int j = 0; j < 8; j++)
//        sum += c[j] / ++y;
// return log(sqrt(2*3.14159) * sum / x) - (x + 7.5) + (x + 0.5) * log(x + 7.5);
// }
//
// // helper function for incomplete beta
// // computes continued fraction
//
// double betaContFrac(double a, double b, double x) {
//  const int MAXIT = 1000;
//  const double EPS = 3e-7;
//  double qab = a + b;
//  double qap = a + 1;
//  double qam = a - 1;
//  double c = 1;
//  double d = 1 - qab * x / qap;
//  if (fabs(d) < FPMIN) d = FPMIN;
//  d = 1 / d;
//  double h = d;
//  int m;
//  for (m = 1; m <= MAXIT; m++) {
//                int m2 = 2 * m;
//                double aa = m * (b-m) * x / ((qam + m2) * (a + m2));
//                d = 1 + aa * d;
//                if (fabs(d) < FPMIN) d = FPMIN;
//                c = 1 + aa / c;
//                if (fabs(c) < FPMIN) c = FPMIN;
//                d = 1 / d;
//                h *= (d * c);
//                aa = -(a+m) * (qab+m) * x / ((a+m2) * (qap+m2));
//                d = 1 + aa * d;
//                if (fabs(d) < FPMIN) d = FPMIN;
//                c = 1 + aa / c;
//                if (fabs(c) < FPMIN) c = FPMIN;
//                d = 1 / d;
//                double del = d*c;
//                h *= del;
//                if (fabs(del - 1) < EPS) break;
//  }
//  if (m > MAXIT) {
//                cerr << "betaContFrac: too many iterations\n";
//  }
//  return h;
// }
//
// // incomplete beta function
// // must have 0 <= x <= 1
// double betaInc(double a, double b, double x) {
//        if (x == 0)
//  return 0;
//        else if (x == 1)
//  return 1;
//        else {
//  double logBeta = logGamma(a+b) - logGamma(a) - logGamma(b)
//                + a * log(x) + b * log(1-x);
//  if (x < (a+1) / (a+b+2))
//                return exp(logBeta) * betaContFrac(a, b, x) / a;
//  else
//                return 1 - exp(logBeta) * betaContFrac(b, a, 1-x) / b;
//        }
// }
//
// double normRNG(double mu, double sigma, double RNG) {
//  static bool deviateAvailable=false;                        //        flag
//  static float storedDeviate;                        //        deviate from previous calculation
//  double polar, rsquared, var1, var2;
//  //srand(time(NULL));
//  //srand((unsigned)time(0));
//  //srand(time(0));
//  //Ran ran(time(0));
//  //        If no deviate has been stored, the polar Box-Muller transformation is
//  //        performed, producing two independent normally-distributed random
//  //        deviates.  One is stored for the next round, and one is returned.
//  if (!deviateAvailable) {
//
//   //        choose pairs of uniformly distributed deviates, discarding those
//   //        that don't fall within the unit circle
//   do {
//    var1 = 2.0*( RNG ) - 1.0;
//    var2 = 2.0*( RNG ) - 1.0;
//
//    //var1=2.0*( ran.doub() ) - 1.0;
//    //var2=2.0*( ran.doub() ) - 1.0;
//
//    rsquared=var1*var1+var2*var2;
//   } while ( rsquared>=1.0 || rsquared == 0.0);
//
//   //        calculate polar transformation for each deviate
//   polar=sqrt(-2.0*log(rsquared)/rsquared);
//
//   //        store first deviate and set flag
//   storedDeviate=var1*polar;
//   deviateAvailable=true;
//
//   //        return second deviate
//   return var2*polar*sigma + mu;
//  }
//
//  //        If a deviate is available from a previous call to this function, it is
//  //        returned, and the flag is set to false.
//  else {
//   deviateAvailable=false;
//   return storedDeviate*sigma + mu;
//  }
// }
//
//
// void loadData(double** data, int dimensionality, int cardinality, string filename) {
//        int x, y;
//
//        ifstream in(filename.c_str());
//
//        if (!in) {
//          cout << "Cannot open file.\n";
//          return;
//        }
//
//        for (y = 0; y < cardinality; y++) {
//          for (x = 0; x < dimensionality; x++) {
//            in >> data[y][x];
//          }
//        }
//
//        in.close();
// }
//
// double calculateCustomPdf(double position, double fitting, double** data_set, int number_samples){
//  double value=-1;
//  double min;
//  double max;
//
//  for (int i=1; i<number_samples; i++){
//   max=data_set[i][1];
//   min=data_set[i-1][1];
//
//   if((position>min)&(position<max)){
//    if (fitting==1)
//     value=data_set[i-1][2];
//    else
//     value=data_set[i-1][2]+(data_set[i][2]-data_set[i-1][2])/(data_set[i][1]-data_set[i-1][1])*(position-data_set[i-1][1]);
//   }
//   else
//    perror ("The following error occurred: distribution sampled out of its boundaries");
//  }
//
//  return value;
// }
//
// double calculateCustomCDF(double position, double fitting, double** data_set, int number_samples){
//  double value=-1;
//  double min;
//  double max;
//  double cumulative=0;
//
//  for (int i=1; i<number_samples; i++){
//   max=data_set[i][1];
//   min=data_set[i-1][1];
//
//   if((position>min)&(position<max)){
//    if (fitting==1)
//     value=cumulative+data_set[i-1][2]*(position-data_set[i-1][1]);
//    else{
//     double pdfValueInPosition =data_set[i-1][2]+(data_set[i][2]-data_set[i-1][2])/(data_set[i][1]-data_set[i-1][1])*(position-data_set[i-1][1]);
//     value=cumulative + (pdfValueInPosition+data_set[i-1][2])*(position-data_set[i-1][1])/2;
//    }
//   }
//   else
//    perror ("The following error occurred: distribution sampled out of its boundaries");
//
//   if (fitting==1)
//    cumulative=cumulative+data_set[i-1][2]*(data_set[i][1]-data_set[i-1][1]);
//   else
//    cumulative=cumulative+(data_set[i][2]+data_set[i-1][2])*(data_set[i][1]-data_set[i-1][1])/2;
//  }
//
//  return value;
// }
//
//        double stdGammaRNG(double shape);
//
// double gammaRNG(double shape, double scale){
//  double value=scale * stdGammaRNG(shape);
//  return value;
// }
//
//        double rkGauss();
//
// double stdGammaRNG(double shape)
// {
//          double b, c;
//          double U, V, X, Y;
//
//          if (shape == 1.0)
//          {
//              return -log(1.0 - rand());
//          }
//          else if (shape < 1.0)
//          {
//              for (;;)
//              {
//                  U = rand();
//                  V = -log(1.0 - rand());
//                  if (U <= 1.0 - shape)
//                  {
//                      X = pow(U, 1./shape);
//                      if (X <= V)
//                      {
//                          return X;
//                      }
//                  }
//                  else
//                  {
//                      Y = -log((1-U)/shape);
//                      X = pow(1.0 - shape + shape*Y, 1./shape);
//                      if (X <= (V + Y))
//                      {
//                          return X;
//                      }
//                  }
//              }
//          }
//          else
//          {
//              b = shape - 1./3.;
//              c = 1./sqrt(9*b);
//              for (;;)
//              {
//                  do
//                  {
//                      X = rkGauss();
//                      V = 1.0 + c*X;
//                  } while (V <= 0.0);
//
//                  V = V*V*V;
//                  U = rand();
//                  if (U < 1.0 - 0.0331*(X*X)*(X*X))
//                      return (b*V);
//                  if (log(U) < 0.5*X*X + b*(1. - V + log(V)))
//                      return (b*V);
//              }
//          }
// }
//
// double rkGauss() {
//  double f, x1, x2, r2;
//
//  do {
//   x1 = 2.0*rand() - 1.0;
//   x2 = 2.0*rand() - 1.0;
//   r2 = x1*x1 + x2*x2;
//  }
//  while (r2 >= 1.0 || r2 == 0.0);
//
//  /* Box-Muller transform */
//  f = sqrt(-2.0*log(r2)/r2);
//  return f*x2;
//
// }
//
//
// double betaRNG(double alpha, double beta){
//  // To be updated
//  double value=0;
//  return value;
// }
//
// double modifiedLogFunction(double x){
//          if (x <= -1.0)
//          {
//              std::stringstream os;
//              os << "Invalid input argument (" << x << "); must be greater than -1.0";
//              throw std::invalid_argument( os.str() );
//          }
//
//          if (fabs(x) > 1e-4)
//              return log(1.0 + x);
//          else
//              return (-0.5*x + 1.0)*x;
// }

        double abramStegunApproximation(double t)
        {
            // Abramowitz and Stegun formula

            double c[] = {2.515517, 0.802853, 0.010328};
            double d[] = {1.432788, 0.189269, 0.001308};
            return t - ((c[2]*t + c[1])*t + c[0]) /
                       (((d[2]*t + d[1])*t + d[0])*t + 1.0);
        }
