//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DirichletBCBase.h"

defineLegacyParams(DirichletBCBase);

InputParameters
DirichletBCBase::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addParam<bool>("preset",
                        true,
                        "Whether or not to preset the BC (apply the value before the solve "
                        "begins). Note that the default value of this parameter is handled by the "
                        "use_legacy_dirichlet_bc parameter on the MooseApp.");
  return params;
}

DirichletBCBase::DirichletBCBase(const InputParameters & parameters)
  : NodalBC(parameters),
    // If the user sets preset, abide by it. Otherwise, pick the default depending on the
    // application's preference to using the legacy dirichlet BC (legacy: preset = false,
    // non-legacy: preset = true)
    _preset(parameters.isParamSetByUser("preset")
                ? getParam<bool>("preset")
                : !_app.parameters().get<bool>("use_legacy_dirichlet_bc"))
{
}

void
DirichletBCBase::computeValue(NumericVector<Number> & current_solution)
{
  mooseAssert(_preset, "BC is not preset");

  if (_var.isNodalDefined())
  {
    const dof_id_type & dof_idx = _var.nodalDofIndex();
    current_solution.set(dof_idx, computeQpValue());
  }
}

Real
DirichletBCBase::computeQpResidual()
{
  return _u[_qp] - computeQpValue();
}
