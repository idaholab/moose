//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalSideIndicatorBase.h"

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

const BoundaryID InternalSideIndicatorBase::InternalBndId = 12345;

InputParameters
InternalSideIndicatorBase::validParams()
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

  params.addPrivateParam<BoundaryID>("_boundary_id", InternalSideIndicatorBase::InternalBndId);
  return params;
}

InternalSideIndicatorBase::InternalSideIndicatorBase(const InputParameters & parameters)
  : Indicator(parameters),
    NeighborCoupleable(this, false, false),
    ScalarCoupleable(this),
    _field_var(_subproblem.getStandardVariable(_tid, name())),
    _current_elem(_assembly.elem()),
    _neighbor_elem(_assembly.neighbor()),
    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _coord_sys(_assembly.coordSystem()),
    _qrule(_assembly.qRuleFace()),
    _q_point(_assembly.qPointsFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),
    _boundary_id(parameters.get<BoundaryID>("_boundary_id")),
    _scale_by_flux_faces(parameters.get<bool>("scale_by_flux_faces")),
    _normals(_assembly.normals())
{
  const std::vector<MooseVariableFieldBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : coupled_vars)
    addMooseVariableDependency(var);

  // Not supported with the base linear lagrange case
  if (_use_displaced_mesh)
    paramError("use_displaced_mesh",
               "Internal side indicators do not support using the displaced mesh at this time. "
               "They can be used on the undisplaced mesh in a Problem with displaced mesh");
  // Access into the solution vector assumes constant monomial
  if (_field_var.feType() != libMesh::FEType(CONSTANT, MONOMIAL))
    mooseError("Only constant monomial variables for the internal side indicator are supported");
}

void
InternalSideIndicatorBase::computeIndicator()
{
  // Derived class decides the contribution at each qp to the indicator value
  Real sum = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();

  // Contribution is added to the two elements
  {
    const auto sys_num = _field_var.sys().number();
    const auto var_num = _field_var.number();
    _solution.add(_current_elem->dof_number(sys_num, var_num, 0), sum * _current_elem->hmax());
    if (_field_var.hasBlocks(_neighbor_elem->subdomain_id()))
      _solution.add(_neighbor_elem->dof_number(sys_num, var_num, 0), sum * _neighbor_elem->hmax());
  }
}

void
InternalSideIndicatorBase::finalize()
{
  unsigned int n_flux_faces = 0;

  if (_scale_by_flux_faces)
  {
    if (isVarFV())
      paramError("scale_by_flux_faces", "Unsupported at this time for finite volume variables");

    // Figure out the total number of sides contributing to the error.
    // We'll scale by this so boundary elements are less penalized
    for (unsigned int side = 0; side < _current_elem->n_sides(); side++)
      if (_current_elem->neighbor_ptr(side) != nullptr)
        n_flux_faces++;
  }
  else
    n_flux_faces = 1;

  // The 0 is because CONSTANT MONOMIALS only have one coefficient per element
  Real value = _field_var.dofValues()[0];

  {
    const auto sys_num = _field_var.sys().number();
    const auto var_num = _field_var.number();

    _solution.set(_current_elem->dof_number(sys_num, var_num, 0),
                  std::sqrt(value) / static_cast<Real>(n_flux_faces));
  }
}
