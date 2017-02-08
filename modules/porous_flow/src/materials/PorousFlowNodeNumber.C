/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowNodeNumber.h"
#include "libmesh/quadrature.h"
#include <limits>

template<>
InputParameters validParams<PorousFlowNodeNumber>()
{
  InputParameters params = validParams<Material>();
  params.addParam<bool>("on_initial_only", false, "It set to true then the node number is calculated for each quadpoint only initially.  This is appropriate if no mesh adaptivity is being used and it reduces computational cost.");
  params.addClassDescription("This Material must be used by all PorousFlow simulations.  It gives the node number for each quadpoint in each element.  This is used in the full upwinding scheme.");
  return params;
}

PorousFlowNodeNumber::PorousFlowNodeNumber(const InputParameters & parameters) :
    Material(parameters),
    _on_initial_only(getParam<bool>("on_initial_only")),
    _node_number(declareProperty<unsigned int>("PorousFlow_node_number")),
    _node_number_old(declarePropertyOld<unsigned int>("PorousFlow_node_number"))
{
}

void
PorousFlowNodeNumber::initQpStatefulProperties()
{
  _node_number[_qp] = nearest();
}

void
PorousFlowNodeNumber::computeQpProperties()
{
  if (_on_initial_only)
    _node_number[_qp] = _node_number_old[_qp];
  else
    _node_number[_qp] = nearest();
}

unsigned
PorousFlowNodeNumber::nearest()
{
  if (!_bnd || true) // 6 Dec, added "|| true" for getting ready for deprecating this class, and commented check below
  {
    //if (_current_elem->n_nodes() > _qrule->n_points())
    //  mooseError2("PorousFlowNodeNumber: PorousFlow currently assumes that the number of nodes in an element does not exceed the number of quadpoints in the element, for elements not on a boundary.  This is so that PorousFlow Materials can store nodal information at the quadpoints.");
    /**
     * Coders take note:
     * For performance reasons, PorousFlow assumes that for _bnd=false
     * (and if numberQps >= numberNodes) then all Materials will store
     * information for node0 at qp0, information for node1 at qp1,
     * information for node2 at qp2, etc.
     * If you want to instead use the closest qp to a node to store
     * the nodal information then you'll have to use the algorithm
     * below for all elements, not just those with _bnd=true, and
     * then "volumetric" MOOSE objects (eg, Kernels) will have to
     * use a strategy for figuring out which qp contains the nodal
     * information that they need.  Such a strategy is currently
     * implemented in the PorousFlow BCs.
     */
    return _qp;
  }
  unsigned n = 0;
  Real smallest_dist = std::numeric_limits<Real>::max();
  for (unsigned i = 0; i < _current_elem->n_nodes(); ++i)
  {
    const Real this_dist = (_current_elem->point(i) - _q_point[_qp]).size();
    if (this_dist < smallest_dist)
    {
      n = i;
      smallest_dist = this_dist;
    }
  }
  return n;
}
