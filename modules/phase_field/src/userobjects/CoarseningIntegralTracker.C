/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CoarseningIntegralTracker.h"
#include "MooseMesh.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<CoarseningIntegralTracker>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Track the pre-coarsening integral of elements.");
  params.addRequiredCoupledVar("v", "variable to correct");
  return params;
}

CoarseningIntegralTracker::CoarseningIntegralTracker(const InputParameters & parameters) :
    ElementUserObject(parameters),
    _mesh(_fe_problem.mesh()),
    _v(coupledValue("v")),
    _pre_adaptivity_ran(false)
{
  _fe_problem.requestCacheMeshChanged();
}

void
CoarseningIntegralTracker::initialize()
{
  // clear the stored child element integrals and set flaf to indicat new data was collected
  _pre_adaptivity_integral.clear();
  _pre_adaptivity_ran = true;

  // clear the corrective source term for the current timestep
  _corrective_source.clear();
}

void
CoarseningIntegralTracker::execute()
{
  Real sum = 0.0;
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    sum += _JxW[qp] * _coord[qp] * _v[qp];
  _pre_adaptivity_integral.insert(std::make_pair(_current_elem, sum));
}

void
CoarseningIntegralTracker::meshChanged()
{
  // if the userobject was not scheduled to run due to lack of dependencies we need to bail out
  if (!_pre_adaptivity_ran)
    return;

  // loop over elements that just have been coarsened (currently not threaded)
  for (const auto & parent : *_mesh.coarsenedElementRange())
  {
    // compute sum of child integrals
    Real child_sum = 0.0;
    const auto & children = _mesh.coarsenedElementChildren(parent);
    for (const auto & child : children)
    {
      auto val = _pre_adaptivity_integral.find(child);
      mooseAssert(val != _pre_adaptivity_integral.end(), "Refined child not found for element");
      child_sum += val->second;
    }

    // compute coarsened parent integral
    _fe_problem.prepare(parent, 0);
    _fe_problem.reinitElem(parent, 0);
    Real parent_sum = 0.0;
    Real parent_vol = 0.0;
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    {
      parent_vol += _JxW[qp] * _coord[qp];
      parent_sum += _JxW[qp] * _coord[qp] * _v[qp];
    }

    // store corrective term
    if (parent_vol > 0.0)
      _corrective_source.insert(std::make_pair(parent, (child_sum - parent_sum) / parent_vol));
  }

  _pre_adaptivity_ran = false;
}

void
CoarseningIntegralTracker::threadJoin(const UserObject & y)
{
  const CoarseningIntegralTracker & uo = static_cast<const CoarseningIntegralTracker &>(y);
  _pre_adaptivity_integral.insert(uo._pre_adaptivity_integral.begin(), uo._pre_adaptivity_integral.end());
}

Real
CoarseningIntegralTracker::sourceValue(const Elem * elem) const
{
  auto val_it = _corrective_source.find(elem);
  if (val_it == _corrective_source.end())
    return 0.0;
  else
    return val_it->second;
}
