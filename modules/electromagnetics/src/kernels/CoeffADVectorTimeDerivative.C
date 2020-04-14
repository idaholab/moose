#include "CoeffADVectorTimeDerivative.h"
#include "Function.h"

registerADMooseObject("ElkApp", CoeffADVectorTimeDerivative);

defineADValidParams(
    CoeffADVectorTimeDerivative,
    ADVectorTimeDerivative,
    params.addClassDescription("The time derivative operator with the weak form of $(\\psi_i, "
                               "a(\\vec{r}, t) \\frac{\\partial u_h}{\\partial t})$, where $a$ is "
                               "a coefficient function.");
    params.addRequiredParam<FunctionName>("coefficient", "Coefficient function."););

CoeffADVectorTimeDerivative::CoeffADVectorTimeDerivative(
    const InputParameters & parameters)
  : ADVectorTimeDerivative(parameters), _coefficient(getFunction("coefficient"))
{
}

ADRealVectorValue
CoeffADVectorTimeDerivative::precomputeQpResidual()
{
  return _coefficient.value(_t, _q_point[_qp]) *
         ADVectorTimeDerivative::precomputeQpResidual();
}


