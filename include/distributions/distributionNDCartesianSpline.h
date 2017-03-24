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
  BasicMultiDimensionalCartesianSpline(const char * data_filename,std::vector<double> alpha, std::vector<double> beta, bool cdf_provided);

  BasicMultiDimensionalCartesianSpline(std::string data_filename, bool cdf_provided);

  BasicMultiDimensionalCartesianSpline(const char * data_filename, bool cdf_provided);

  BasicMultiDimensionalCartesianSpline(std::vector< std::vector<double> > & discretizations, std::vector<double> & values, std::vector<double> alpha, std::vector<double> beta, bool cdf_provided);

  BasicMultiDimensionalCartesianSpline(std::string data_filename,std::vector<double> alpha, std::vector<double> beta, bool cdf_provided);

  void basicMultiDimensionalCartesianSplineInit();

  BasicMultiDimensionalCartesianSpline();

  virtual ~BasicMultiDimensionalCartesianSpline()
  {
  };

  double pdf(std::vector<double> x);

  double cdf(std::vector<double> x);

  std::vector<double> inverseCdf(double f, double g);

  double inverseMarginal(double f, int dimension);

  int returnDimensionality();

  void updateRNGparameter(double tolerance, double initial_divisions);

  double marginal(double x, int dimension);

  double returnUpperBound(int dimension);

  double returnLowerBound(int dimension);

protected:
  bool _cdf_provided;
  NDSpline _interpolator;
  NDSpline _CDFinterpolator;
};

#endif /* DISTRIBUTION_ND_CARTESIAN_SPLINE_H */
