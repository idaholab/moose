 /*
 * PumpCoastdownExponential.C
 *
 *  Created on: Aug 8, 2012
 *      Author: mandd
 */

#include "PumpCoastdownExponential.h"
#include <cmath>
#include "CrowTools.h"

/*
 * Pump coast down exponential
 */

template<>
InputParameters validParams<PumpCoastdownExponential>(){

   InputParameters params = validParams<CrowTools>();

   params.addRequiredParam<double>("coefficient", "Exponential constant coefficient");
   params.addRequiredParam<double>("initial_flow_rate", "Initial flow rate");
   return params;
}



PumpCoastdownExponential::PumpCoastdownExponential(const InputParameters & parameters):
  CrowTools(parameters)
{
  _tool_parameters["coefficient"      ] = getParam<double>("coefficient");
  _tool_parameters["initial_flow_rate"] = getParam<double>("initial_flow_rate");
}

PumpCoastdownExponential::~PumpCoastdownExponential()
{
}

double
PumpCoastdownExponential::compute(double time)
{
  return getVariable("initial_flow_rate")*(exp(-time/getVariable("coefficient")));
}

/*
 * Pump coast down curve provided
 */


//template<>
//InputParameters validParams<PumpCoastdownCurve>(){
//
//   InputParameters params = validParams<CrowTools>();
//
//   params.addRequiredParam<std::vector <double> >("time_points", "Time points");
//   params.addRequiredParam<std::vector <double> >("flow_rate_points", "Flow rate points");
//   params.addRequiredParam<double>("initial_flow_rate", "Initial flow rate");
//   params.addParam<int>("interpolation_num_points", 2 ,"Number of adjacent points must be used for interpolating");
//   params.addParam<custom_dist_fit_type>("interpolation_type", LINEAR ,"Interpolation type");
//   return params;
//}

//PumpCoastdownCurve::PumpCoastdownCurve(const std::string & name, InputParameters parameters):
//  CrowTools(name,parameters)
//
//{
//  _tool_parameters["initial_flow_rate"       ] = getParam<double>("initial_flow_rate");
//  _tool_parameters["interpolation_num_points"] = double(getParam<int>("interpolation_num_points"));
//
//  _interpolation=InterpolationFunctions(getParam<std::vector <double> >("time_points"),
//                                         getParam<std::vector <double> >("flow_rate_points"),
//                                         getParam<int>("interpolation_num_points"),
//                                         getParam<custom_dist_fit_type>("interpolation_type"));
//}
//
//PumpCoastdownCurve::~PumpCoastdownCurve()
//{
//}
//
//double
//PumpCoastdownCurve::compute(double time)
//{
//  return _interpolation.interpolation(time);
//}



