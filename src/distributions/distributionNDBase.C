/*
 * distributionNDBase.C
 * Created on Oct. 23, 2015
 * Author: @wangc
 * Extracted from @alfoa (Feb 6, 2014) distribution_base_ND.C
 *
 */

#include "distributionNDBase.h"
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

double BasicDistributionND::cellIntegral(std::vector<double> center, std::vector<double> dx){
    /**
     * This function calculates the integral of the pdf in a cell region
     * In the 1D case a cell region is an interval [a,b], thus the integral of the pdf in such interval is
     * calculated as CDF(b)-CDF(a). This functions perform a similar evolution but for a generic ND cell
     */

  double value = 0.0;

  int numberOfVerteces = (int)pow(2,center.size());
  double sign = 1.0;

  int counter=1;

  for(int i=numberOfVerteces; i>0; i--){
    std::vector<double> index = int2binary(i-1,center.size());
    std::vector<double> NDcoordinate(center.size());

    for(unsigned int j=0; j<center.size(); j++){
      if (index.at(j)==0)
        NDcoordinate.at(j) = center.at(j) - dx.at(j)/2.0;
      else
        NDcoordinate.at(j) = center.at(j) + dx.at(j)/2.0;
    }
    value += Cdf(NDcoordinate) * sign;

    sign = sign * (-1.0);
    counter++;
    if (counter%2)
      sign = sign * (-1.0);
  }

  return value;
}

//double BasicDistributionND::cellIntegral(std::vector<double> center, std::vector<double> dx){
//  double value=0.0;
//
//  int numberOfVerteces = (int)pow(2,center.size());
//
//  for(int i=0; i<numberOfVerteces; i++){
//    std::vector<double> index = int2binary(i,center.size());
//    std::vector<double> NDcoordinate(center.size());
//
//    for(unsigned int j=0; j<center.size(); j++){
//      if (index[j]==0)
//        NDcoordinate.at(j) = center.at(j) - dx.at(j)/2.0;
//      else
//        NDcoordinate.at(j) = center.at(j) + dx.at(j)/2.0;
//    }
//    value += Cdf(NDcoordinate);
//  }
//
//  value = value/numberOfVerteces;
//
//  return value;
//}


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

//std::vector<double> DistributionInverseCdf(BasicDistributionND & dist, double & min, double & max){
std::vector<double> DistributionInverseCdf(BasicDistributionND & dist, double & F, double & g){
  return dist.InverseCdf(F,g);
}

double returnUpperBound(BasicDistributionND & dist, int dimension){
  /**
   * this function returns the upper bound of the distribution for a particular dimension
   */
  return dist.returnUpperBound(dimension);
}

double returnLowerBound(BasicDistributionND & dist, int dimension){
   /**
    * this function returns the upper lower of the distribution for a particular dimension
    */
  return dist.returnLowerBound(dimension);
}
