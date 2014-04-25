/*
 * decayHeat.C
 *
 *  Created on: Aug 8, 2012
 *      Author: mandd
 */

#include "decayHeat.h"
#include <math.h>
#include "CrowTools.h"

//enum{}

template<>
InputParameters validParams<decayHeat>(){

   InputParameters params = validParams<CrowTools>();

   params.addRequiredParam<int>   ("eq_type", "Equation Type (1/2)");
   params.addRequiredParam<double>("initial_pow", "Initial Power (W)");
   params.addRequiredParam<double>("operating_time", "operating time");
   params.addRequiredParam<double>("power_coefficient", "scaling power coefficient");
   return params;
}

decayHeat::decayHeat(const std::string & name, InputParameters parameters):
  CrowTools(name,parameters)
{
  _tool_parameters["initial_pow"      ] = getParam<double>("initial_pow");
  _tool_parameters["operating_time"   ] = getParam<double>("operating_time");
  _tool_parameters["power_coefficient"] = getParam<double>("power_coefficient");
  _equationType = getParam<int>("eq_type");
  if(_equationType != 1 and _equationType != 2)  throw("DecayHeat supports only equation type 1 or 2 so far.");
}

decayHeat::~decayHeat()
{
}

double
decayHeat::compute(double time)
{
  double powerValue;
  if(_equationType == 1)
  {
    powerValue=getVariable(std::string("initial_pow"))*getVariable(std::string("power_coefficient"))*(pow(time,-0.2) - pow(time+getVariable("operating_time"),-0.2));
  }
  else{
    powerValue=getVariable(std::string("initial_pow"))*getVariable(std::string("power_coefficient"))*(pow(time+10,-0.2) - pow(time+getVariable(std::string("operating_time"))+10,-0.2) - 0.87*(pow(time+pow(10,7),-0.2)-pow(time+getVariable(std::string("operating_time"))+2*pow(10,7),-0.2)));
  }
  return powerValue;
}
