#include "ComputeLagrangianStressCauchy.h"

InputParameters
ComputeLagrangianStressCauchy::validParams()
{
  InputParameters params = ComputeLagrangianStressBase::validParams();
  return params;
}

ComputeLagrangianStressCauchy::ComputeLagrangianStressCauchy(const InputParameters & parameters)
  : ComputeLagrangianStressBase(parameters),
    _inv_df(getMaterialPropertyByName<RankTwoTensor>("inv_inc_def_grad")),
    _inv_def_grad(getMaterialPropertyByName<RankTwoTensor>("inv_def_grad")),
    _detJ(getMaterialPropertyByName<Real>("detJ"))
{
}

void
ComputeLagrangianStressCauchy::computeQpStressUpdate()
{
  computeQpCauchyStress();
  _wrapCauchyStress();
}

void
ComputeLagrangianStressCauchy::_wrapCauchyStress()
{
  // Actually do the (annoying) wrapping
  if (_ld)
  {
    _pk1_stress[_qp] = _detJ[_qp] * _cauchy_stress[_qp] * _inv_def_grad[_qp].transpose();

    _pk1_jacobian[_qp].zero();
    for (size_t i = 0; i < 3; i++)
    {
      for (size_t J = 0; J < 3; J++)
      {
        for (size_t k = 0; k < 3; k++)
        {
          for (size_t L = 0; L < 3; L++)
          {
            for (size_t m = 0; m < 3; m++)
            {
              _pk1_jacobian[_qp](i, J, k, L) +=
                  _detJ[_qp] * _cauchy_stress[_qp](i, m) *
                  (_inv_def_grad[_qp](L, k) * _inv_def_grad[_qp](J, m) -
                   _inv_def_grad[_qp](J, k) * _inv_def_grad[_qp](L, m));
              for (size_t n = 0; n < 3; n++)
              {
                for (size_t t = 0; t < 3; t++)
                {
                  _pk1_jacobian[_qp](i, J, k, L) += _detJ[_qp] * _cauchy_jacobian[_qp](i, m, n, t) *
                                                    _inv_def_grad[_qp](J, m) * _inv_df[_qp](n, k) *
                                                    _inv_def_grad[_qp](L, t);
                }
              }
            }
          }
        }
      }
    }
  }
  else
  {
    _pk1_stress[_qp] = _cauchy_stress[_qp];
    _pk1_jacobian[_qp] = _cauchy_jacobian[_qp];
  }
}
