/*
 * PowerGrid.C
 *
 *  Created on: Aug 9, 2012
 *      Author: mandd
 */

#include "PowerGrid.h"
#include "CrowTools.h"

template<>
InputParameters validParams<PowerGrid>(){

   InputParameters params = validParams<CrowTools>();

   params.addRequiredParam<double>("status", "Power Grid status (1 = ON,0 = OFF)");
   return params;
}

PowerGrid::PowerGrid(const InputParameters & parameters):
  CrowTools(parameters)
{
  _tool_parameters["status"      ] = getParam<double>("status");
}

PowerGrid::~PowerGrid()
{
}

