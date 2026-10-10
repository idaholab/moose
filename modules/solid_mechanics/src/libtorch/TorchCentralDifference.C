//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "MooseTypes.h"
#include "IntegratedBCBase.h"

// MOOSE includes
#include "TorchCentralDifference.h"

registerMooseObject("SolidMechanicsApp", TorchCentralDifference);

InputParameters
TorchCentralDifference::validParams()
{
  InputParameters params = ExplicitMixedOrder::validParams();
  params.addClassDescription(
      "Central difference time integrator using batched libtorch finite-element kernels (and "
      "optionally NEML2 material models for the constitutive update).");
  params.addRequiredParam<UserObjectName>(
      "assembly", "The TorchAssembly object to use to provide assembly information");
  params.addRequiredParam<UserObjectName>(
      "fe", "The TorchFEInterpolation object to use to couple variables");
  return params;
}

TorchCentralDifference::TorchCentralDifference(const InputParameters & parameters)
  : ExplicitMixedOrder(parameters)
{
}

void
TorchCentralDifference::rebuildBoundaryElementList()
{
  if (!_nl)
    return;

  _boundary_elems.clear();

  // build boundary element list by iterating over all integrated BCs
  const auto & ibcs = _nl->getIntegratedBCWarehouse();
  std::unordered_set<BoundaryID> bnds;
  for (const auto & ibc : ibcs.getObjects())
    bnds.insert(ibc->boundaryIDs().begin(), ibc->boundaryIDs().end());

  if (!bnds.empty())
  {
    mooseInfo("Dectected BCs on ", bnds.size(), " boundaries.");

    // deduplicate elements that have multiple boundaries
    std::unordered_set<const Elem *> unique_elems;
    const auto end = _fe_problem.mesh().bndElemsEnd();
    for (auto it = _fe_problem.mesh().bndElemsBegin(); it != end; ++it)
      if (bnds.find((*it)->_bnd_id) != bnds.end())
        unique_elems.insert((*it)->_elem);

    _boundary_elems.assign(unique_elems.begin(), unique_elems.end());
    mooseInfo("Adding ", _boundary_elems.size(), " elements to the algebraic range.");
  }

  _boundary_elems_dirty = false;
}

void
TorchCentralDifference::initialSetup()
{
  ExplicitMixedOrder::initialSetup();
  _assembly = &_fe_problem.getUserObject<TorchAssembly>("assembly", /*tid=*/0);
  _fe = &_fe_problem.getUserObject<TorchFEInterpolation>("fe", /*tid=*/0);
  rebuildBoundaryElementList();
}

void
TorchCentralDifference::meshChanged()
{
  ExplicitMixedOrder::meshChanged();
  _boundary_elems_dirty = true;
}

void
TorchCentralDifference::postSolve()
{
  ExplicitMixedOrder::postSolve();
  _fe->invalidateInterpolations();
}

void
TorchCentralDifference::evaluateRHSResidual()
{
  if (_boundary_elems_dirty)
    rebuildBoundaryElementList();

  if (_fe->contextUpToDate() && _assembly->upToDate())
  {
    libMesh::ConstElemRange boundary_elem_range(&_boundary_elems);
    _fe_problem.setCurrentAlgebraicElementRange(&boundary_elem_range);

    libMesh::ConstNodeRange null_node_range(&_no_node);
    _fe_problem.setCurrentAlgebraicNodeRange(&null_node_range);
  }

  ExplicitMixedOrder::evaluateRHSResidual();

  // Reset the algebraic ranges
  _fe_problem.setCurrentAlgebraicElementRange(nullptr);
  _fe_problem.setCurrentAlgebraicNodeRange(nullptr);
}

#endif // MOOSE_LIBTORCH_ENABLED
