//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"
#include "IntegratedBCBase.h"

#ifdef NEML2_ENABLED

// MOOSE includes
#include "NEML2CentralDifference.h"

registerMooseObject("SolidMechanicsApp", NEML2CentralDifference);

InputParameters
NEML2CentralDifference::validParams()
{
  InputParameters params = ExplicitMixedOrder::validParams();
  params.addClassDescription(
      "Central difference time integrator using NEML2 material models and kernels.");
  params.addRequiredParam<UserObjectName>(
      "assembly", "The NEML2Assembly object to use to provide assembly information");
  params.addRequiredParam<UserObjectName>(
      "fe", "The NEML2FEInterpolation object to use to couple variables");
  return params;
}

NEML2CentralDifference::NEML2CentralDifference(const InputParameters & parameters)
  : ExplicitMixedOrder(parameters)
{
}

void
NEML2CentralDifference::initialSetup()
{
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

  ExplicitMixedOrder::initialSetup();
  _neml2_assembly = &_fe_problem.getUserObject<NEML2Assembly>("assembly", /*tid=*/0);
  _fe = &_fe_problem.getUserObject<NEML2FEInterpolation>("fe", /*tid=*/0);
}

void
NEML2CentralDifference::postSolve()
{
  ExplicitMixedOrder::postSolve();
  _fe->invalidateInterpolations();
}

void
NEML2CentralDifference::evaluateRHSResidual()
{
  if (_fe->contextUpToDate() && _neml2_assembly->upToDate())
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

#endif // NEML2_ENABLED
