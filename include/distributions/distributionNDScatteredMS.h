/*
 * distributionNDScatteredMS.h
 * Created by @wangc on Oct. 23, 2015
 * Extracted from  distribution_base_ND.h
 *
 */
#ifndef DISTRIBUTION_ND_SCATTERED_MS_H
#define DISTRIBUTION_ND_SCATTERED_MS_H
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

class BasicMultiDimensionalScatteredMS: public virtual BasicDistributionND
{
public:
  BasicMultiDimensionalScatteredMS(std::string data_filename,double p,int precision): _interpolator(data_filename,p,precision)
  {
  };
  BasicMultiDimensionalScatteredMS(double p,int precision): _interpolator(p,precision)
  {
  };
  virtual ~BasicMultiDimensionalScatteredMS()
  {
  };
  double
  pdf(std::vector<double> x)
  {
    return _interpolator.interpolateAt(x);
  };
  double
  Cdf(std::vector<double> x)
  {
    return _interpolator.interpolateAt(x);
  };

//  std::vector<double>
//  inverseCdf(double /*min*/, double /*max*/)
//  {
//    return std::vector<double>(2,-1.0);
//  };

  std::vector<double>
  inverseCdf(double F, double g)
  {
   return _interpolator.ndInverseFunctionGrid(F,g);
   //return _interpolator.NDinverseFunction(min, max);
  };

  double
    inverseMarginal(double /* F */, int /* dimension */)
  {
          throwError("BasicMultiDimensionalScatteredMS: inverseMarginal not available");
          return 0.0;
  }

  int
  returnDimensionality()
  {
          return _interpolator.returnDimensionality();
  }

  double returnLowerBound(int dimension){
    throwError("BasicMultiDimensionalScatteredMS: returnLowerBound not available");
    return 0.0;
  }

  double returnUpperBound(int dimension){
    throwError("BasicMultiDimensionalScatteredMS: returnUpperBound not available");
    return 0.0;
  }

protected:
  MicroSphere _interpolator;
};

#endif /* DISTRIBUTION_ND_SCATTERED_MS_H */
