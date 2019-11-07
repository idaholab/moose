//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomHitSolutionModifier.h"

// MOOSE includes
#include "MooseVariableFE.h"
#include "NonlinearSystemBase.h"
#include "RandomHitUserObject.h"

registerMooseObject("MooseTestApp", RandomHitSolutionModifier);

InputParameters
RandomHitSolutionModifier::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<UserObjectName>(
      "random_hits", "The name of the UserObject to use for the positions of the random hits");
  params.addRequiredParam<VariableName>("modify", "The name of the variable to be modified");
  params.addRequiredParam<Real>("amount", "Amount to add at the random hit location");
  return params;
}

RandomHitSolutionModifier::RandomHitSolutionModifier(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _random_hits(getUserObject<RandomHitUserObject>("random_hits")),
    _mesh(_subproblem.mesh()),
    _variable(dynamic_cast<MooseVariable &>(
        _subproblem.getVariable(0, parameters.get<VariableName>("modify")))),
    _amount(parameters.get<Real>("amount"))
{
}

void
RandomHitSolutionModifier::execute()
{
  std::unique_ptr<PointLocatorBase> pl = _mesh.getMesh().sub_point_locator();
  pl->enable_out_of_mesh_mode();

  const std::vector<Point> & hits = _random_hits.hits();

  _nodes_that_were_hit.resize(hits.size());

  for (unsigned int i = 0; i < hits.size(); i++)
  {
    const Point & hit = hits[i];

    // First find the element the hit lands in
    const Elem * elem = (*pl)(hit);

    if (elem && (elem->processor_id() == processor_id()))
    {
      Real closest_distance = std::numeric_limits<unsigned int>::max();
      const Node * closest_node = NULL;

      // Find the node on that element that is closest.
      for (unsigned int n = 0; n < elem->n_nodes(); n++)
      {
        const Node * cur_node = elem->node_ptr(n);
        Real cur_distance = (hit - *cur_node).norm();

        if (cur_distance < closest_distance)
        {
          closest_distance = cur_distance;
          closest_node = cur_node;
        }
      }

      if (closest_node)
      {
        _subproblem.reinitNode(closest_node, 0);
        _variable.setNodalValue(_variable.getNodalValue(*closest_node) + _amount);
        _variable.insert(_fe_problem.getNonlinearSystemBase().solution());
      }
    }
  }

  _fe_problem.getNonlinearSystemBase().solution().close();
  _fe_problem.getNonlinearSystemBase().system().update();
}
