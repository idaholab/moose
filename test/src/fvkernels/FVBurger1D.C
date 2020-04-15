
#include "FVBurger1D.h"

registerMooseObject("MooseTestApp", FVBurger1D);

InputParameters
FVBurger1D::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  return params;
}

FVBurger1D::FVBurger1D(const InputParameters & params)
  : FVFluxKernel(params)
{
}

ADReal
FVBurger1D::computeQpResidual()
{
  mooseAssert(_face_info->leftElem().dim() == 1, "FVBurger1D works only in 1D");
  ADReal r = 0;
  ADReal u_av = 0.5 * (_u_left[_qp] + _u_right[_qp]);
  if (u_av * _normal(0) > 0)
    r = 0.5 * _u_left[_qp] * _u_left[_qp] * _normal(0);
  else
    r = 0.5 * _u_right[_qp] * _u_right[_qp] * _normal(0);
  return r;
}
