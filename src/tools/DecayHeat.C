/*
 * DecayHeat.C
 *
 *  Created on: Aug 8, 2012
 *      Author: mandd
 */

#include "DecayHeat.h"
#include <cmath>
#include "CrowTools.h"

//enum{}

template<>
InputParameters validParams<DecayHeat>(){

   InputParameters params = validParams<CrowTools>();

   params.addRequiredParam<int>   ("eq_type", "Equation Type (1/2)");
   params.addRequiredParam<double>("initial_pow", "Initial Power (W)");
   params.addRequiredParam<double>("operating_time", "operating time");
   params.addRequiredParam<double>("power_coefficient", "scaling power coefficient");
   params.addRequiredParam<double>("start_time", "time at which decay heat calculation start");
   return params;
}

DecayHeat::DecayHeat(const InputParameters & parameters):
  CrowTools(parameters)
{
  _tool_parameters["initial_pow"      ] = getParam<double>("initial_pow");
  _tool_parameters["operating_time"   ] = getParam<double>("operating_time");
  _tool_parameters["power_coefficient"] = getParam<double>("power_coefficient");
  _tool_parameters["start_time"] = getParam<double>("start_time");
  _equation_type = getParam<int>("eq_type");
  if(_equation_type != 1 and _equation_type != 2)  throw("DecayHeat supports only equation type 1 or 2 so far.");
}

DecayHeat::~DecayHeat()
{
}

double
DecayHeat::compute(double time)
{
  double powerValue;
  double real_time = time + getVariable(std::string("start_time"));
  if(_equation_type == 1)
  {
    powerValue=getVariable(std::string("initial_pow"))*getVariable(std::string("power_coefficient"))*(pow(real_time,-0.2) - pow(time+getVariable("operating_time"),-0.2));
  }
  else{
    powerValue=getVariable(std::string("initial_pow"))*getVariable(std::string("power_coefficient"))*(pow(real_time+10,-0.2) - pow(time+getVariable(std::string("operating_time"))+10,-0.2) - 0.87*(pow(real_time+pow(10,7),-0.2)-pow(real_time+getVariable(std::string("operating_time"))+2*pow(10,7),-0.2)));
  }
  return powerValue;
}
