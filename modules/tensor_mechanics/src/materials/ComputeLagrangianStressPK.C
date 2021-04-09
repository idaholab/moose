#include "ComputeLagrangianStressPK.h"

InputParameters
ComputeLagrangianStressPK::validParams()
{
  InputParameters params = ComputeLagrangianStressBase::validParams();
  return params;
}

ComputeLagrangianStressPK::ComputeLagrangianStressPK(const InputParameters & parameters)
  : ComputeLagrangianStressBase(parameters),
    _inv_df(getMaterialPropertyByName<RankTwoTensor>("inv_inc_def_grad")),
    _F(getMaterialPropertyByName<RankTwoTensor>("deformation_gradient")),
    _detJ(getMaterialPropertyByName<Real>("detJ"))
{
}

void
ComputeLagrangianStressPK::computeQpStressUpdate()
{
  computeQpPKStress();
  _wrapPKStress();
}

void
ComputeLagrangianStressPK::_wrapPKStress()
{
  // Actually do the (annoying) wrapping
  if (_ld)
  {
    _cauchy_stress[_qp] = _pk1_stress[_qp] * _F[_qp].transpose() / _detJ[_qp];
    _cauchy_jacobian[_qp].zero();
    auto f = _inv_df[_qp].inverse();
    for (size_t i = 0; i < 3; i++)
    {
      for (size_t j = 0; j < 3; j++)
      {
        for (size_t k = 0; k < 3; k++)
        {
          for (size_t l = 0; l < 3; l++)
          {
            _cauchy_jacobian[_qp](i, j, k, l) +=
                f(j, k) * _cauchy_stress[_qp](i, l) - _cauchy_stress[_qp](i, j) * f(l, k);
            for (size_t A = 0; A < 3; A++)
            {
              for (size_t m = 0; m < 3; m++)
              {
                for (size_t N = 0; N < 3; N++)
                {
                  _cauchy_jacobian[_qp](i, j, k, l) += 1.0 / _detJ[_qp] * _F[_qp](j, A) *
                                                       _pk1_jacobian[_qp](i, A, m, N) * f(m, k) *
                                                       _F[_qp](l, N);
                }
              }
            }
          }
        }
      }
    }
  }
  // Small deformations these are the same
  else
  {
    _cauchy_stress[_qp] = _pk1_stress[_qp];
    _cauchy_jacobian[_qp] = _pk1_jacobian[_qp];
  }
}
