/*
 * powerGrid.C
 *
 *  Created on: Aug 9, 2012
 *      Author: mandd
 */

#include "powerGrid.h"
#include "CrowTools.h"

template<>
InputParameters validParams<powerGrid>(){

   InputParameters params = validParams<RavenTools>();

   params.addRequiredParam<double>("status", "Power Grid status (1 = ON,0 = OFF)");
   return params;
}

powerGrid::powerGrid(const std::string & name, InputParameters parameters):
  RavenTools(name,parameters)
{
  _tool_parameters["status"      ] = getParam<double>("status");
}

powerGrid::~powerGrid()
{
}

