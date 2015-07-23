/*
 * DieselGeneratorBase.C
 *
 *  Created on: Jun 6, 2012
 *      Author: alfoa
 */

#include "DieselGeneratorBase.h"
#include "CrowTools.h"

template<>
InputParameters validParams<DieselGeneratorBase>(){

   InputParameters params = validParams<CrowTools>();

   params.addRequiredParam<double>("supply_time", "Supply");
   params.addRequiredParam<double>("start_time", "start_time");
   params.addRequiredParam<double>("status", "Diesel Generator status (1 = ON, 0 = OFF)");
   return params;
}

DieselGeneratorBase::DieselGeneratorBase(const InputParameters & parameters):
  CrowTools(parameters)
{
  if((getParam<double>("supply_time") > 0)  & (getParam<double>("status") != 1)) throw("Bad input - diesel generators' values do not agree");
  if((getParam<double>("supply_time") <= 0) & (getParam<double>("status") != 0)) throw("Bad input - diesel generators' values do not agree");

  if(getParam<double>("supply_time") < 0)
  {
    _tool_parameters["supply_time"] = 0.0;
    _tool_parameters["status"] = 0;
  }
  else
  {
    _tool_parameters["supply_time"] = getParam<double>("supply_time");
    _tool_parameters["status"      ] = getParam<double>("status");
    _tool_parameters["start_time"      ] = getParam<double>("start_time");
  }
}

DieselGeneratorBase::~DieselGeneratorBase()
{
}

double
DieselGeneratorBase::compute(double time)
{
 double status=getVariable(std::string("status"));

 if (status==1){
  if (time>(getVariable(std::string("supply_time"))+getVariable(std::string("start_time"))))
   status=0;
  else
   status=1;
 }
 else
  status=0;

 return status;
}
