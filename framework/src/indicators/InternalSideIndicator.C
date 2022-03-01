//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalSideIndicator.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/dense_vector.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/quadrature.h"

const BoundaryID InternalSideIndicator::InternalBndId = 12345;

InputParameters
InternalSideIndicator::validParams()
{
  InputParameters params = Indicator::validParams();
  params.addRequiredParam<VariableName>(
      "variable", "The name of the variable that this side indicator applies to");
  params.addParam<bool>("scale_by_flux_faces",
                        false,
                        "Whether or not to scale the error values by "
                        "the number of flux faces.  This attempts to "
                        "not penalize elements on boundaries for "
                        "having less neighbors.");

  params.addPrivateParam<BoundaryID>("_boundary_id", InternalSideIndicator::InternalBndId);
  return params;
}

InternalSideIndicator::InternalSideIndicator(const InputParameters & parameters)
  : Indicator(parameters),
    NeighborCoupleable(this, false, false),
    ScalarCoupleable(this),
    NeighborMooseVariableInterface(
        this, false, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD),
    _field_var(_subproblem.getStandardVariable(_tid, name())),

    _current_elem(_assembly.elem()),
    _neighbor_elem(_assembly.neighbor()),

    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),

    _coord_sys(_assembly.coordSystem()),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),

    _boundary_id(parameters.get<BoundaryID>("_boundary_id")),

    _var(mooseVariableField()),
    _scale_by_flux_faces(parameters.get<bool>("scale_by_flux_faces")),

    _u(_var.sln()),
    _grad_u(_var.gradSln()),

    _normals(_assembly.normals()),

    _u_neighbor(_var.slnNeighbor()),
    _grad_u_neighbor(_var.gradSlnNeighbor())
{
  const std::vector<MooseVariableFieldBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);

  addMooseVariableDependency(&mooseVariableField());
}

void
InternalSideIndicator::computeIndicator()
{
  Real sum = 0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();

  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);

    _solution.add(_field_var.nodalDofIndex(), sum * _current_elem->hmax());
    _solution.add(_field_var.nodalDofIndexNeighbor(), sum * _neighbor_elem->hmax());
  }
}

void
InternalSideIndicator::finalize()
{
  unsigned int n_flux_faces = 0;

  if (_scale_by_flux_faces)
  {
    // Figure out the total number of sides contributing to the error.
    // We'll scale by this so boundary elements are less penalized
    for (unsigned int side = 0; side < _current_elem->n_sides(); side++)
      if (_current_elem->neighbor_ptr(side) != nullptr)
        n_flux_faces++;
  }
  else
    n_flux_faces = 1;

  // The 0 is because CONSTANT MONOMIALS only have one coefficient per element...
  Real value = _field_var.dofValues()[0];

  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _solution.set(_field_var.nodalDofIndex(), std::sqrt(value) / static_cast<Real>(n_flux_faces));
  }
}
