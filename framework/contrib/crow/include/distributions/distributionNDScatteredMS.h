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
  cdf(std::vector<double> x)
  {
    return _interpolator.interpolateAt(x);
  };

//  std::vector<double>
//  inverseCdf(double /*min*/, double /*max*/)
//  {
//    return std::vector<double>(2,-1.0);
//  };

  std::vector<double>
  inverseCdf(double f, double g)
  {
   return _interpolator.ndInverseFunctionGrid(f,g);
   //return _interpolator.NDinverseFunction(min, max);
  };

  double
    inverseMarginal(double /* f */, int /* dimension */)
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
