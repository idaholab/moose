//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppPositions.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"

registerMooseObject("MooseApp", MultiAppPositions);

InputParameters
MultiAppPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addRequiredParam<std::vector<MultiAppName>>(
      "multiapps", "Name(s) of the multiapps providing the positions");
  params.addParam<bool>("use_apps_centroid",
                        false,
                        "Whether to use the mesh centroid offset by the app position rather than "
                        "just the position of each child app");

  // Execute after multiapps have been created
  params.set<ExecFlagEnum>("execute_on") = EXEC_INITIAL;
  // Positions of subapps should stay ordered the same as the subapps
  params.set<bool>("auto_sort") = false;
  // Subapp positions are known for all processes already
  params.set<bool>("auto_broadcast") = false;

  params.addClassDescription(
      "Obtain positions from MultiApps. This may only be used to set the positions of those same "
      "multiapps if an 'initial_positions' parameter is used.");
  return params;
}

MultiAppPositions::MultiAppPositions(const InputParameters & parameters)
  : Positions(parameters), _use_apps_centroid(getParam<bool>("use_apps_centroid"))
{
  // Centroids cannot be computed for non-local apps
  if (_use_apps_centroid)
    _need_broadcast = true;
}

void
MultiAppPositions::initialize()
{
  clearPositions();

  const auto & multiapps = getParam<std::vector<MultiAppName>>("multiapps");
  _positions_2d.resize(multiapps.size());

  for (const auto m_it : index_range(multiapps))
  {
    const std::string & multiapp_name = multiapps[m_it];
    const auto & multiapp = _fe_problem.getMultiApp(multiapp_name);

    for (const auto & i_global : make_range(multiapp->numGlobalApps()))
    {
      const auto p = multiapp->position(i_global);
      if (!_use_apps_centroid)
      {
        _positions.push_back(p);
        _positions_2d[m_it].push_back(p);
      }
      // Get the centroid of each subapp mesh
      else
      {
        // Cant compute centroid if does not own the mesh
        if (!multiapp->hasLocalApp(i_global))
          continue;
        auto & fe_problem_base = multiapp->appProblemBase(i_global);
        const MeshBase & mesh = (getParam<bool>("use_displaced_mesh") &&
                                 fe_problem_base.getDisplacedProblem().get() != NULL)
                                    ? fe_problem_base.getDisplacedProblem()->mesh().getMesh()
                                    : fe_problem_base.mesh().getMesh();
        const Point centroid = MooseMeshUtils::meshCentroidCalculator(mesh);

        // There's a broadcast after, no need to add on every rank
        if (multiapp->isFirstLocalRank())
        {
          _positions.push_back(p + centroid);
          _positions_2d[m_it].push_back(p + centroid);
        }
      }
    }
  }
}
