/*
 * Batteries.C
 *
 *  Created on: Aug 9, 2012
 *      Author: mandd
 */

#include "Batteries.h"
#include "CrowTools.h"

template<>
InputParameters validParams<Batteries>(){

   InputParameters params = validParams<CrowTools>();

   params.addRequiredParam<double>("initial_life", "Initial battery life");
   params.addRequiredParam<double>("start_time", "Start time for using battery");
   params.addRequiredParam<double>("status", "Battery status (1 = ON,0 = OFF)");
   return params;
}



Batteries::Batteries(const InputParameters & parameters) :
  CrowTools(parameters)
{
  if((getParam<double>("initial_life") > 0)  & (getParam<double>("status") != 1)) throw("Bad input - Batteries values do not agree");
  if((getParam<double>("initial_life") <= 0) & (getParam<double>("status") != 0)) throw("Bad input - Batteries values do not agree");

  if(getParam<double>("initial_life") < 0)
  {
    _tool_parameters["initial_life"] = 0.0;
    _tool_parameters["status"] = 0;
    _tool_parameters["start_time"] = getParam<double>("start_time");
  }
  else
  {
    _tool_parameters["initial_life"] = getParam<double>("initial_life");
    _tool_parameters["start_time"] = getParam<double>("start_time");
    _tool_parameters["status"      ] = getParam<double>("status");
  }
}

Batteries::~Batteries()
{
}

double
Batteries::compute(double time)
{
 double status=getVariable(std::string("status"));

 if (status==1){
  if (time>(getVariable(std::string("initial_life"))+getVariable(std::string("start_time"))))
   status=0;
  else
   status=1;
 }
 else
  status=0;

 return status;
}

