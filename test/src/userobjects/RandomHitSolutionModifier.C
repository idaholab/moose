/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "RandomHitSolutionModifier.h"

#include "RandomHitUserObject.h"

template<>
InputParameters validParams<RandomHitSolutionModifier>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addRequiredParam<UserObjectName>("random_hits", "The name of the UserObject to use for the positions of the random hits");
  params.addRequiredParam<VariableName>("modify", "The name of the variable to be modified");
  params.addRequiredParam<Real>("amount", "Amount to add at the random hit location");
  return params;
}

RandomHitSolutionModifier::RandomHitSolutionModifier(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
    _random_hits(getUserObject<RandomHitUserObject>("random_hits")),
    _mesh(_subproblem.mesh()),
    _point_locator(_mesh.getMesh().point_locator()),
    _variable(_subproblem.getVariable(0, parameters.get<VariableName>("modify"))),
    _amount(parameters.get<Real>("amount"))
{}

void
RandomHitSolutionModifier::execute()
{
  const std::vector<Point> & hits = _random_hits.hits();

  _nodes_that_were_hit.resize(hits.size());


  for(unsigned int i=0; i<hits.size(); i++)
  {
    const Point & hit = hits[i];

    AutoPtr<PointLocatorBase> pl = PointLocatorBase::build(TREE, _mesh);

    // First find the element the hit lands in
    const Elem * elem = (*pl)(hit);

    if(elem->processor_id() == libMesh::processor_id())
    {
      Real closest_distance = std::numeric_limits<unsigned int>::max();
      Node * closest_node = NULL;

      // Find the node on that element that is closest.
      for(unsigned int n=0; n<elem->n_nodes(); n++)
      {
        Node * cur_node = elem->get_node(n);
        Real cur_distance = (hit - *cur_node).size();

        if(cur_distance < closest_distance)
        {
          closest_distance = cur_distance;
          closest_node = cur_node;
        }
      }

      _subproblem.reinitNode(closest_node, 0);
      _variable.setNodalValue(_variable.getNodalValue(*closest_node) + _amount);
      _variable.insert(_fe_problem.getNonlinearSystem().solution());
    }
  }

  _fe_problem.getNonlinearSystem().solution().close();
  _fe_problem.getNonlinearSystem().sys().update();
}
