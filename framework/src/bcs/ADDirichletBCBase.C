//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADDirichletBCBase.h"

defineADLegacyParams(ADDirichletBCBase);

template <ComputeStage compute_stage>
InputParameters
ADDirichletBCBase<compute_stage>::validParams()
{
  InputParameters params = ADNodalBC<compute_stage>::validParams();
  params.addParam<bool>("preset",
                        true,
                        "Whether or not to preset the BC (apply the value before the solve "
                        "begins). Note that the default value of this parameter is handled by the "
                        "use_legacy_dirichlet_bc parameter on the MooseApp.");
  return params;
}

template <ComputeStage compute_stage>
ADDirichletBCBase<compute_stage>::ADDirichletBCBase(const InputParameters & parameters)
  : ADNodalBC<compute_stage>(parameters),
    // If the user sets preset, abide by it. Otherwise, pick the default depending on the
    // application's preference to using the legacy dirichlet BC (legacy: preset = false,
    // non-legacy: preset = true)
    _preset(parameters.isParamSetByUser("preset")
                ? getParam<bool>("preset")
                : !this->_app.parameters().template get<bool>("use_legacy_dirichlet_bc"))
{
}

template <ComputeStage compute_stage>
void
ADDirichletBCBase<compute_stage>::computeValue(NumericVector<Number> & current_solution)
{
  mooseAssert(_preset, "BC is not preset");

  if (_var.isNodalDefined())
  {
    auto && dof_idx = _var.nodalDofIndex();
    current_solution.set(dof_idx, MetaPhysicL::raw_value(computeQpValue()));
  }
}

template <ComputeStage compute_stage>
ADReal
ADDirichletBCBase<compute_stage>::computeQpResidual()
{
  return _u - computeQpValue();
}

// explicit instantiation is required for AD base classes
adBaseClass(ADDirichletBCBase);
