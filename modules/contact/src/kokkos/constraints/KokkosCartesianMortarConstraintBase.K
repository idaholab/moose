//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosCartesianMortarConstraintBase.h"

#include "FEProblemBase.h"

InputParameters
KokkosCartesianMortarConstraintBase::validParams()
{
  InputParameters params = MortarConstraintBase::validParams();
  params.addRequiredParam<VariableName>("lm_y",
                                        "The contact Lagrange multiplier along the y direction.");
  params.addParam<VariableName>("lm_z",
                                "The contact Lagrange multiplier along the z direction, required "
                                "in three dimensions.");
  params.setDocString("variable", "The contact Lagrange multiplier along the x direction.");

  // The constraint is posed on the current configuration
  params.set<bool>("use_displaced_mesh") = true;
  params.suppressParameter<bool>("use_displaced_mesh");
  params.set<bool>("interpolate_normals") = false;
  params.suppressParameter<bool>("interpolate_normals");

  return params;
}

KokkosCartesianMortarConstraintBase::KokkosCartesianMortarConstraintBase(
    const InputParameters & parameters)
  : MortarConstraintBase(parameters), _dim(_mesh.dimension())
{
  if (_dim == 3 && !isParamValid("lm_z"))
    paramError("lm_z", "The multiplier along z is required in three dimensions.");
  if (_dim != 3 && isParamValid("lm_z"))
    paramError("lm_z", "The multiplier along z applies only in three dimensions.");

  const std::string names[3] = {"variable", "lm_y", "lm_z"};
  _pinned_lm_vars.clear();
  for (const auto c : make_range(_dim))
  {
    const auto & var = c ? _sys.getVariable(_tid, getParam<VariableName>(names[c])) : _var;
    if (var.feType() != _var.feType() || var.useDual() != _use_dual)
      paramError(names[c],
                 "The components of the multiplier must share one finite element type and basis.");
    if (var.feType().family != libMesh::LAGRANGE || var.feType().order != libMesh::FIRST)
      paramError(names[c],
                 "The multiplier must be a first-order Lagrange variable, whose degrees of "
                 "freedom are at the secondary nodes.");

    _lm[c].init(var);
    _pinned_lm_vars.push_back(&var);
    addMooseVariableDependency(const_cast<MooseVariableFieldBase *>(&var));
  }

  if (_n_disp != _dim)
    paramError("variable",
               "Every displacement of the [Mesh] block must be a solution variable of the system "
               "of the multiplier.");
}

KokkosCartesianMortarConstraintBase::KokkosCartesianMortarConstraintBase(
    const KokkosCartesianMortarConstraintBase & object)
  : MortarConstraintBase(object), _dim(object._dim)
{
  for (const auto c : make_range(3u))
    _lm[c] = object._lm[c];
}
