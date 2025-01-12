//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVTimeKernel.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"
#include "Assembly.h"
#include "SubProblem.h"

InputParameters
INSFVTimeKernel::validParams()
{
  auto params = FVFunctorTimeKernel::validParams();
  params += INSFVMomentumResidualObject::validParams();

  // These options can be set to facilitate restarts with RC coefficients that do not depend on
  // the time steps. By default, they are not active
  params.addParam<bool>(
      "contribute_to_rc",
      true,
      "Whether the time derivative term should contribute to Rhie-Chow coefficients");
  params.addParam<Real>("start_fixed_dt_contribution_to_RC",
                        0,
                        "Time at which to start a fixed dt contribution to the RC coefficients");
  params.addParam<Real>("fixed_dt_contribution_to_RC",
                        "Fixed dt for the contribution to the Rhie Chow coefficients");
  params.declareControllable("fixed_dt_contribution_to_RC");
  params.addParamNamesToGroup("contribute_to_rc start_fixed_dt_contribution_to_RC", "Advanced");
  return params;
}

INSFVTimeKernel::INSFVTimeKernel(const InputParameters & params)
  : FVFunctorTimeKernel(params),
    INSFVMomentumResidualObject(*this),
    _contribute_to_rc_coeffs(getParam<bool>("contribute_to_rc")),
    _use_fixed_dt_rc_contrib(isParamValid("fixed_dt_contribution_to_RC")),
    _fixed_dt_rc_contrib_start(getParam<Real>("start_fixed_dt_contribution_to_RC")),
    _fixed_dt_rc_contrib(_use_fixed_dt_rc_contrib ? getParam<Real>("fixed_dt_contribution_to_RC")
                                                  : _zero)
{
  if (!_use_fixed_dt_rc_contrib && isParamSetByUser("start_fixed_dt_contribution_to_RC"))
    paramError("start_fixed_dt_contribution_to_RC",
               "Should not be set unless 'start_fixed_dt_contribution_to_RC' is set");
}

void
INSFVTimeKernel::addResidualAndJacobian(const ADReal & residual, const dof_id_type dof_index)
{
  addResidualsAndJacobian(_assembly,
                          std::array<ADReal, 1>{{residual}},
                          std::array<dof_id_type, 1>{{dof_index}},
                          _var.scalingFactor());
}
