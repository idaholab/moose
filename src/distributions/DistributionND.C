/*
 * DistributionND.C
 *
 *  Created on: Feb 6, 2014
 *      Author: alfoa
 *
 */

#include "DistributionND.h"
#include "distributionNDCartesianSpline.h"
#include <string>
#include <list>

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

template<>
InputParameters validParams<DistributionND>()
{
  InputParameters params = validParams<MooseObject>();
  params.addParam<double>("ProbabilityThreshold", 1.0, "Probability Threshold");
  params.addRequiredParam<std::string>("type","distribution type");
  params.addRequiredParam<std::string>("data_filename","Name of the file containing the data points to be interpolated");
  params.addParam<double>("PB_window_Low", 0.0, "Probability window lower bound");
  params.addParam<double>("PB_window_Up", 1.0, "Probability window upper bound");
  params.addRequiredParam<EPbFunctionType>("function_type","PDF or CDF");
  params.registerBase("DistributionND");
  return params;
}

class DistributionND;

DistributionND::DistributionND(const InputParameters & parameters):
      MooseObject(parameters)
{
   _type          = getParam<std::string>("type");
   _data_filename = getParam<std::string>("data_filename");
   //_function_type = getParam<EPbFunctionType>("function_type");
   _dis_parameters["ProbabilityThreshold"]  = getParam<double>("ProbabilityThreshold");
   _dis_parameters["PB_window_Low"] = getParam<double>("PB_window_Low");
   _dis_parameters["PB_window_Up"]  = getParam<double>("PB_window_Up");

   _check_status   = false;
}

DistributionND::~DistributionND(){
}

/*
 * CLASS ND DISTRIBUTION InverseWeight
 */

template<>
InputParameters validParams<MultiDimensionalInverseWeight>(){

   InputParameters params = validParams<DistributionND>();
   params.addRequiredParam<double>("p", "Minkowski distance parameter");
   params.addRequiredParam<bool>("CDF", "Boolean value (True if CDF is provided)");
   return params;

}

MultiDimensionalInverseWeight::MultiDimensionalInverseWeight(const InputParameters & parameters):
    DistributionND(parameters),
    BasicMultiDimensionalInverseWeight(getParam<std::string>("data_filename"),getParam<double>("p"), getParam<bool>("CDF"))
{
}

//MultiDimensionalInverseWeight::MultiDimensionalInverseWeight(const char * name, InputParameters parameters):
//    DistributionND(parameters),
//    BasicMultiDimensionalInverseWeight(getParam<std::string>("data_filename"),getParam<double>("p"))
//{
//}

MultiDimensionalInverseWeight::~MultiDimensionalInverseWeight()
{
}

/*
 * CLASS ND DISTRIBUTION MultivariateNormal
 */

template<>
InputParameters validParams<MultivariateNormal>(){

   InputParameters params = validParams<DistributionND>();
   params.addRequiredParam<std::vector<double> >("mu", "Mu vector");
   return params;

}

MultivariateNormal::MultivariateNormal(const InputParameters & parameters):
    DistributionND(parameters),
    BasicMultivariateNormal(getParam<std::string>("data_filename"),getParam<std::vector<double> >("mu"))
{
}

MultivariateNormal::~MultivariateNormal()
{
}


/*
 * CLASS ND DISTRIBUTION MultiDimensionalScatteredMS
 */

template<>
InputParameters validParams<MultiDimensionalScatteredMS>(){

   InputParameters params = validParams<DistributionND>();
   params.addRequiredParam<double>("p", "Minkowski distance parameter");
   params.addRequiredParam<int>("precision", " ");
   return params;

}

MultiDimensionalScatteredMS::MultiDimensionalScatteredMS(const InputParameters & parameters):
    DistributionND(parameters),
    BasicMultiDimensionalScatteredMS(getParam<std::string>("data_filename"),getParam<double>("p"),getParam<int>("precision"))
{
}

MultiDimensionalScatteredMS::~MultiDimensionalScatteredMS()
{
}

/*
 * CLASS ND DISTRIBUTION MultiDimensionalCartesianSpline
 */

template<>
InputParameters validParams<MultiDimensionalCartesianSpline>(){

   InputParameters params = validParams<DistributionND>();
   params.addRequiredParam<bool>("CDF", "Boolean value (True if CDF is provided)");
   //params.addRequiredParam<std::vector<double> >("alpha", "alpha");
   //params.addRequiredParam<std::vector<double> >("beta", "beta");
   return params;

}

MultiDimensionalCartesianSpline::MultiDimensionalCartesianSpline(const InputParameters & parameters):
    DistributionND(parameters),
        //BasicMultiDimensionalCartesianSpline(getParam<std::string>("data_filename"),getParam<std::vector<double> >("alpha"),getParam<std::vector<double> >("beta"), getParam<bool>("CDF"))
    BasicMultiDimensionalCartesianSpline(getParam<std::string>("data_filename"), getParam<bool>("CDF"))
{
}

MultiDimensionalCartesianSpline::~MultiDimensionalCartesianSpline()
{
}


///*
// * CLASS ND DISTRIBUTION MultiDimensionalLinear
// */
//
//template<>
//InputParameters validParams<MultiDimensionalLinear>(){
//
//   InputParameters params = validParams<DistributionND>();
//   return params;
//
//}
//
//MultiDimensionalLinear::MultiDimensionalLinear(const InputParameters & parameters):
//    DistributionND(parameters),
//    BasicMultiDimensionalLinear(getParam<std::string>("data_filename"))
//{
//}
//
//MultiDimensionalLinear::~MultiDimensionalLinear()
//{
//}
