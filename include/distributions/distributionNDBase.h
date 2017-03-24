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
 * distributionNDBase.h
 * Created by @wangc on Oct. 23, 2015
 * Extracted from  distribution_base_ND.h
 *
 */
#ifndef DISTRIBUTION_ND_BASE_H
#define DISTRIBUTION_ND_BASE_H
#include <map>
#include <string>
#include <vector>
#include "ND_Interpolation_Functions.h"
#include "distributionFunctions.h"
#include <stdexcept>
#include <iostream>
#include <fstream>

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

enum EPbFunctionType{PDF,CDF};

class BasicDistributionND
{
public:

   BasicDistributionND();
   virtual ~BasicDistributionND();
   double  getVariable(const std::string & variable_name);                       ///< getVariable from mapping
   void updateVariable(const std::string & variable_name, double & new_value);
   virtual double  pdf(std::vector<double> x) = 0;                              ///< pdf function at coordinate x
   virtual double  cdf(std::vector<double> x) = 0;                              ///< cdf function at coordinate x
   virtual std::vector<double> inverseCdf(double f, double g) = 0;

   virtual double inverseMarginal(double f, int dimension) = 0;
   virtual int returnDimensionality() = 0;
   double cellIntegral(std::vector<double> center, std::vector<double> dx);

   virtual double returnUpperBound(int dimension) = 0;
   virtual double returnLowerBound(int dimension) = 0;

   std::string & getType();

   std::vector<int> oneDtoNDconverter(int one_d_coordinate, std::vector<int> indexes);

protected:
   std::string _type; ///< Distribution type
   std::string _data_filename;
   EPbFunctionType _function_type;
   std::map <std::string,double> _dis_parameters;
   bool _check_status;

   double _tolerance;
   int _initial_divisions;

   //marginal distribution functions
   //std::vector<NDInterpolation> marginalDistributions;
};

//class BasicMultiDimensionalLinear: public  virtual BasicDistributionND
//{
//public:
//  BasicMultiDimensionalLinear(std::string data_filename): _interpolator(data_filename)
//  {
//   bool LBcheck = _interpolator.checkLB(0.0);
//   if (LBcheck == false)
//    throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element below 0.0 in file: " << data_filename);
//
//   bool UBcheck = _interpolator.checkUB(1.0);
//   if (UBcheck == false)
//    throwError("BasicMultiDimensionalCartesianSpline Distribution error: CDF values given as input contain element above 1.0 in file: " << data_filename);
//  };
//  BasicMultiDimensionalLinear(): _interpolator()
//  {
//  };
//  virtual ~BasicMultiDimensionalLinear()
//  {
//  };
//  double
//  pdf(std::vector<double> x)
//  {
//    return _interpolator.interpolateAt(x);
//  };
//  double
//  cdf(std::vector<double> x)
//  {
//     double value = _interpolator.interpolateAt(x);
//
//     if (value > 1.0)
//      value=1.0;
//
//     return value;
//  };
//  double
//  inverseCdf(std::vector<double> /*x*/)
//  {
//    return -1.0;
//  };
//  std::vector<double>
//  inverseCdf(double /*min*/, double /*max*/)
//  {
//    return std::vector<double>(2,-1.0);
//  };
//protected:
//  NDlinear _interpolator;
//};


#endif /* DISTRIBUTION_ND_BASE_H */
