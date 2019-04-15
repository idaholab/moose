//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ADPhaseFieldFracture.h"

registerADMooseObject("PhaseFieldApp", ADPhaseFieldFracture);

defineADValidParams(
    ADPhaseFieldFracture,
    ADKernel,
    params.addClassDescription(
        "Kernel to compute bulk energy contribution to damage order parameter residual equation");
    params.addParam<MaterialPropertyName>("l_name", "l", "Interface width");
    params.addParam<MaterialPropertyName>("gc", "gc_prop", "Critical fracture energy density"););

template <ComputeStage compute_stage>
ADPhaseFieldFracture<compute_stage>::ADPhaseFieldFracture(const InputParameters & parameters)
  : ADKernel<compute_stage>(parameters),
    _gc_prop(adGetMaterialProperty<Real>("gc")),
    _l(adGetMaterialProperty<Real>("l_name")),
    _hist(adGetADMaterialProperty<Real>("hist")),
    _hist_old(adGetMaterialPropertyOld<Real>("hist"))
{
}

template <ComputeStage compute_stage>
ADResidual
ADPhaseFieldFracture<compute_stage>::computeQpResidual()
{
  // return (-_gc_prop[_qp] * _l[_qp] * _grad_u[_qp] * _grad_test[_i][_qp] +
  //         2.0 * (1.0 - _u[_qp]) * _test[_i][_qp] * _hist_old[_qp] -
  //         _gc_prop[_qp] / _l[_qp] * _u[_qp] * _test[_i][_qp]) /
  //        _gc_prop[_qp];

  // if (_current_elem->id() == 0 && _qp == 0)
  //   std::cout << "hist_old = " << MetaPhysicL::raw_value(_hist_old[_qp]) << ", part 1 = "
  //             << MetaPhysicL::raw_value(-3.0 / 4 * _gc_prop[_qp] * _l[_qp] * _grad_u[_qp] *
  //                                       _grad_test[_i][_qp])
  //             << ", part 2 = "
  //             << MetaPhysicL::raw_value(2.0 * (1.0 - _u[_qp]) * _test[_i][_qp] * _hist_old[_qp])
  //             << ", part 3 = "
  //             << MetaPhysicL::raw_value(_gc_prop[_qp] / _l[_qp] * (3.0 / 8) * _test[_i][_qp])
  //             << ", residual = "
  //             << MetaPhysicL::raw_value(
  //                    (-3.0 / 4 * _gc_prop[_qp] * _l[_qp] * _grad_u[_qp] * _grad_test[_i][_qp] +
  //                     2.0 * (1.0 - _u[_qp]) * _test[_i][_qp] * _hist_old[_qp] -
  //                     _gc_prop[_qp] / _l[_qp] * (3.0 / 8) * _test[_i][_qp]) /
  //                    _gc_prop[_qp])
  //             << std::endl;

  return (-3.0 / 4 * _gc_prop[_qp] * _l[_qp] * _grad_u[_qp] * _grad_test[_i][_qp] +
          2.0 * (1.0 - _u[_qp]) * _test[_i][_qp] * _hist_old[_qp] -
          _gc_prop[_qp] / _l[_qp] * (3.0 / 8) * _test[_i][_qp]) /
         _gc_prop[_qp];
}

template class ADPhaseFieldFracture<RESIDUAL>;
template class ADPhaseFieldFracture<JACOBIAN>;
