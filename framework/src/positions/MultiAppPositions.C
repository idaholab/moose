//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppPositions.h"

registerMooseObject("MooseApp", MultiAppPositions);

InputParameters
MultiAppPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addRequiredParam<std::vector<MultiAppName>>(
      "multiapps", "Name(s) of the multiapps providing the positions");
  // Execute after multiapps have been created
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  params.addClassDescription(
      "Obtain positions from MultiApps. This must NOT be used to set the same multiapp positions");
  return params;
}

MultiAppPositions::MultiAppPositions(const InputParameters & parameters) : Positions(parameters) {}

void
MultiAppPositions::initialize()
{
  clearPositions();

  std::vector<MultiAppName> multiapps = getParam<std::vector<MultiAppName>>("multiapps");
  _positions_2d.resize(multiapps.size());

  for (unsigned int m_it = 0; m_it < multiapps.size(); m_it++)
  {
    const std::string multiapp_name = multiapps[m_it];
    const auto & multiapp = _fe_problem.getMultiApp(multiapp_name);

    for (const auto & i_global : make_range(multiapp->numGlobalApps()))
    {
      auto p = multiapp->position(i_global);
      _positions.push_back(p);
      _positions_2d[m_it].push_back(p);
    }
  }
}