/*
 * distributionNDCartesianSpline.h
 * Created by @wangc on Oct. 23, 2015
 * Extracted from  distribution_base_ND.h
 *
 */
#ifndef DISTRIBUTION_ND_CARTESIAN_SPLINE_H
#define DISTRIBUTION_ND_CARTESIAN_SPLINE_H
#include <map>
#include <string>
#include <vector>
#include "ND_Interpolation_Functions.h"
#include "distributionFunctions.h"
#include "distributionNDBase.h"
#include <stdexcept>
#include <iostream>
#include <fstream>

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

class BasicMultiDimensionalCartesianSpline: public virtual BasicDistributionND
{
public:
  BasicMultiDimensionalCartesianSpline(const char * data_filename,std::vector<double> alpha, std::vector<double> beta, bool CDFprovided);

  BasicMultiDimensionalCartesianSpline(std::string data_filename, bool CDFprovided);

  BasicMultiDimensionalCartesianSpline(const char * data_filename, bool CDFprovided);

  BasicMultiDimensionalCartesianSpline(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta, bool CDFprovided);

  BasicMultiDimensionalCartesianSpline(std::string data_filename,std::vector<double> alpha, std::vector<double> beta, bool CDFprovided);

  void BasicMultiDimensionalCartesianSpline_init();

  BasicMultiDimensionalCartesianSpline();

  virtual ~BasicMultiDimensionalCartesianSpline()
  {
  };

  double Pdf(std::vector<double> x);

  double Cdf(std::vector<double> x);

  std::vector<double> inverseCdf(double F, double g);

  double inverseMarginal(double F, int dimension);

  int returnDimensionality();

  void updateRNGparameter(double tolerance, double initial_divisions);

  double Marginal(double x, int dimension);

  double returnUpperBound(int dimension);

  double returnLowerBound(int dimension);

protected:
  bool _CDFprovided;
  NDSpline _interpolator;
  NDSpline _CDFinterpolator;
};

#endif /* DISTRIBUTION_ND_CARTESIAN_SPLINE_H */
