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

template <ComputeStage compute_stage>
CoeffADVectorTimeDerivative<compute_stage>::CoeffADVectorTimeDerivative(
    const InputParameters & parameters)
  : ADVectorTimeDerivative<compute_stage>(parameters), _coefficient(getFunction("coefficient"))
{
}

template <ComputeStage compute_stage>
ADRealVectorValue
CoeffADVectorTimeDerivative<compute_stage>::precomputeQpResidual()
{
  return _coefficient.value(_t, _q_point[_qp]) *
         ADVectorTimeDerivative<compute_stage>::precomputeQpResidual();
}

adBaseClass(CoeffADVectorTimeDerivative);
