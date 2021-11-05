#include "NonlinearTimeDerivative.h"

registerMooseObject("PhaseFieldApp", NonlinearTimeDerivative);

InputParameters
NonlinearTimeDerivative::validParams()
{
  InputParameters params = ADTimeDerivative::validParams();
  params.addClassDescription("Compute the nonlinear time derivative term.");
  params.addRequiredParam<std::string>("coefficient", 
                        "nonlinear coefficient the of time derivative term");
  return params;
}

NonlinearTimeDerivative::NonlinearTimeDerivative(const InputParameters & parameters)
  : ADTimeDerivative(parameters),
    _coefficient_name(getParam<std::string>("coefficient")),
    _coefficient(getADMaterialProperty<Real>(_coefficient_name))
{
}

ADReal NonlinearTimeDerivative::precomputeQpResidual() {
  return _coefficient[_qp] * ADTimeDerivative::precomputeQpResidual();
}