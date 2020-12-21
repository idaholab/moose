// * This file is part of the MOOSE framework
// * https://www.mooseframework.org
// *
// * All rights reserved, see COPYRIGHT for full restrictions
// * https://github.com/idaholab/moose/blob/master/COPYRIGHT
// *
// * Licensed under LGPL 2.1, please see LICENSE for details
// * https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledLineSourceRayKernelTestTempl.h"

registerMooseObject("RayTracingTestApp", CoupledLineSourceRayKernelTest);
registerMooseObject("RayTracingTestApp", ADCoupledLineSourceRayKernelTest);

template <bool is_ad>
InputParameters
CoupledLineSourceRayKernelTestTempl<is_ad>::validParams()
{
  InputParameters params = GenericRayKernel<is_ad>::validParams();
  params.addRequiredCoupledVar("coupled", "The coupled variable");
  return params;
}

template <bool is_ad>
CoupledLineSourceRayKernelTestTempl<is_ad>::CoupledLineSourceRayKernelTestTempl(
    const InputParameters & params)
  : GenericRayKernel<is_ad>(params),
    _coupled(coupled("coupled")),
    _coupled_val(this->template coupledGenericValue<is_ad>("coupled"))
{
}

template <bool is_ad>
GenericReal<is_ad>
CoupledLineSourceRayKernelTestTempl<is_ad>::computeQpResidual()
{
  return -_test[_i][_qp] * _coupled_val[_qp] * _coupled_val[_qp];
}

template <bool is_ad>
GenericReal<is_ad>
CoupledLineSourceRayKernelTestTempl<is_ad>::computeQpOffDiagJacobian(const unsigned int jvar_num)
{
  if (jvar_num == _coupled)
    return -2.0 * _phi[_j][_qp] * _test[_i][_qp] * _coupled_val[_qp];
  return 0;
}

template class CoupledLineSourceRayKernelTestTempl<false>;
template class CoupledLineSourceRayKernelTestTempl<true>;
