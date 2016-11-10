/*
 * TableFunction.C
 *
 *  Created on: July 10, 2013
 *      Author: alfoa
 */

#include "TableFunction.h"
#include "CrowTools.h"
#include "InterpolationFunctions.h"

template<>
InputParameters validParams<TableFunction>(){

   InputParameters params = validParams<CrowTools>();

   params.addRequiredParam<std::vector<double> >("x_coordinates", "x values (1D array)");
   params.addRequiredParam<std::vector<double> >("y_coordinates", "y values (1D array)");
   params.addParam<double>("scaling_factor", 1.0 ,"scaling factor. Result = InterpolatedValue*scaling_factor");
   MooseEnum fitting_enum("step_left=0 step_right=1 linear=2 cubic_spline=3","linear");
   params.addParam<MooseEnum>("fitting_type",fitting_enum, "type of fitting (0 = STEP_LEFT,1 = STEP_RIGHT,2 = LINEAR,3 = CUBIC_SPLINE)");
   return params;
}

#define ECustomDistFitType custom_dist_fit_type

TableFunction::TableFunction(const InputParameters & parameters):
  CrowTools(parameters)
{
  _tool_parameters["scaling_factor"] = getParam<double>("scaling_factor");
  _interpolation=InterpolationFunctions(getParam<std::vector<double> >("x_coordinates"),
                 getParam<std::vector<double> >("y_coordinates"),
       static_cast<ECustomDistFitType>((int)getParam<MooseEnum>("fitting_type")));
}
TableFunction::~TableFunction()
{
}
double
TableFunction::compute(double x)
{
  return _interpolation.interpolation(x)*getVariable(std::string("scaling_factor"));
}
