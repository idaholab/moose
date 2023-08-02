/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                       BlackBear                              */
/*                                                              */
/*           (c) 2017 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifdef NEML2_ENABLED

#include "CauchyStressFromNEML2Receiver.h"

registerMooseObject("BlackBearApp", CauchyStressFromNEML2Receiver);

InputParameters
CauchyStressFromNEML2Receiver::validParams()
{
  InputParameters params = ComputeLagrangianObjectiveStress::validParams();
  params.addClassDescription(
      "retrieve the batched output vector from a NEML2 material model and use the output variables "
      "to perform the objective stress integration.");
  params.addRequiredParam<UserObjectName>(
      "neml2_uo", "The NEML2 user object that performs the batched computation");
  return params;
}

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
