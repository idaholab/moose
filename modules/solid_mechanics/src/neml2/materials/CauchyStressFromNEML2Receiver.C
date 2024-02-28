//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CauchyStressFromNEML2Receiver.h"
#include "NEML2Utils.h"

registerMooseObject("SolidMechanicsApp", CauchyStressFromNEML2Receiver);

InputParameters
CauchyStressFromNEML2Receiver::validParams()
{
  InputParameters params = ComputeLagrangianObjectiveStress::validParams();
  NEML2Utils::addClassDescription(
      params,
      "Retrieve the batched output vector from a NEML2 material model and use the output variables "
      "to perform the objective stress integration");
  params.addRequiredParam<UserObjectName>(
      "neml2_uo", "The NEML2 user object that performs the batched computation");
  return params;
}

#ifndef NEML2_ENABLED

CauchyStressFromNEML2Receiver::CauchyStressFromNEML2Receiver(const InputParameters & parameters)
  : ComputeLagrangianObjectiveStress(parameters)
{
  NEML2Utils::libraryNotEnabledError(parameters);
}

#else

CauchyStressFromNEML2Receiver::CauchyStressFromNEML2Receiver(const InputParameters & parameters)
  : ComputeLagrangianObjectiveStress(parameters),
    _neml2_uo(getUserObject<CauchyStressFromNEML2UO>("neml2_uo")),
    _output(_neml2_uo.getOutputData())
{
}

void
CauchyStressFromNEML2Receiver::computeQpSmallStress()
{
  if (!_neml2_uo.outputReady())
    return;

  const auto index = _neml2_uo.getIndex(_current_elem->id());
  _small_stress[_qp] = std::get<0>(_output[index + _qp]);
  _small_jacobian[_qp] = std::get<1>(_output[index + _qp]);
}

#endif // NEML2_ENABLED
