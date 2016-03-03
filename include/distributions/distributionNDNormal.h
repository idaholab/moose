/*
 * distributionNDNormal.h
 * Created by @wangc on Oct. 23, 2015
 * Extracted from  distribution_base_ND.h
 *
 */
#ifndef DISTRIBUTION_ND_NORMAL_H
#define DISTRIBUTION_ND_NORMAL_H
#include <map>
#include <string>
#include <vector>
#include "ND_Interpolation_Functions.h"
#include "distributionFunctions.h"
#include "distributionNDBase.h"
#include "distributionNDCartesianSpline.h"
#include <stdexcept>
//#include "distribution_min.h"
#include <iostream>
#include <fstream>

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }


class BasicMultivariateNormal: public virtual BasicDistributionND
{
public:
  BasicMultivariateNormal(const char * data_filename, std::vector<double> mu);
  BasicMultivariateNormal(std::string data_filename, std::vector<double> mu);
  BasicMultivariateNormal(std::vector<std::vector<double> > covMatrix, std::vector<double> mu);
  BasicMultivariateNormal(std::vector<double> vecCovMatrix, std::vector<double> mu);
  BasicMultivariateNormal(std::vector<double> vecCovMatrix, std::vector<double> mu, const char * type, int rank);

  //void BasicMultivariateNormal_init(std::string data_filename, std::vector<double> mu);
  void BasicMultivariateNormal_init(unsigned int & rows, unsigned int &columns, std::vector<std::vector<double> > covMatrix, std::vector<double> mu);

  virtual ~BasicMultivariateNormal();
  double  Pdf(std::vector<double> x);
  double  Cdf(std::vector<double> x);
  std::vector<double> InverseCdf(double F, double g);
  double inverseMarginal(double F, int dimension);
  void updateRNGparameter(double tolerance, double initial_divisions);
  double Marginal(double x, int dimension);

  int returnDimensionality();

  //double MVNDST(std::vector<double> a, std::vector<double> b, double alpha, double epsilon, int Nmax);
  double phi(double x);
  double phi_inv(double x);
  //double rn();
  double * cholesky(double *A, int n);
  std::vector<std::vector<double> > choleskyDecomposition(std::vector<std::vector<double> > matrix);
  void show_matrix(double *A, int n);
  // used to obtain the information from the svd decomposition
  std::vector<double> getSingularValues();
  std::vector<double> getSingularValues(std::vector<int> index);
  std::vector<double> getLeftSingularVectors();
  std::vector<double> getLeftSingularVectors(std::vector<int> index);
  std::vector<double> getRightSingularVectors();
  std::vector<double> getRightSingularVectors(std::vector<int> index);
  std::vector<int> getLeftSingularVectorsDimensions();
  std::vector<int> getLeftSingularVectorsDimensions(std::vector<int> index);
  std::vector<int> getRightSingularVectorsDimensions();
  std::vector<int> getRightSingularVectorsDimensions(std::vector<int> index);
  std::vector<double> getTransformationMatrix();
  std::vector<double> getTransformationMatrix(std::vector<int> index);
  std::vector<int> getTransformationMatrixDimensions();
  std::vector<int> getTransformationMatrixDimensions(std::vector<int> index);
  int  getSingularValuesDimension();
  int  getSingularValuesDimension(std::vector<int> index);

  std::vector<double> coordinateInTransformedSpace(int rank);
  std::vector<double> coordinateInverseTransformed(std::vector<double> &coordinate);
  std::vector<double> coordinateInverseTransformed(std::vector<double> &coordinate,std::vector<int> index);

  double cellProbabilityWeight(std::vector<double> center, std::vector<double> dx);
  double inverseMarginalForPCA(double F);
  double marginalCdfForPCA(double x);

  void computeSVD();
  void computeSVD(int rank);
  double pdfInTransformedSpace(std::vector<double> x);

  double returnLowerBound(int dimension){
    return _lowerBounds.at(dimension);
  }

  double returnUpperBound(int dimension){
    return _upperBounds.at(dimension);
  }

private:
  std::vector<double> _mu;
  std::vector<std::vector<double> > _cov_matrix;
  std::vector<std::vector<double> > _inverse_cov_matrix;
  std::vector<std::vector<double> > _cholesky_C;
  // parameters for singular value decomposition
  std::vector<std::vector<double> > _leftSingularVectors;
  std::vector<double> _singularValues;
  std::vector<std::vector<double> > _rightSingularVectors;
  unsigned int _rank; // used for dimensionality reduction
  // store U*sqrt(S), where U, S, V = svd(A)
  std::vector<std::vector<double> > _svdTransformedMatrix;
  std::string _covarianceType;
  double _determinant_cov_matrix;

  std::vector<double> _upperBounds;
  std::vector<double> _lowerBounds;

  BasicMultiDimensionalCartesianSpline _cartesianDistribution;

  void base10tobaseN(int value_base10, int base, std::vector<int> & value_baseN);
  double getPdf(std::vector<double> x, std::vector<double> mu, std::vector<std::vector<double> > inverse_cov_matrix);
};


#endif /* DISTRIBUTION_ND_NORMAL_H */
