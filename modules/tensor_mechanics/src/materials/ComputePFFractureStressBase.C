//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePFFractureStressBase.h"

template <>
InputParameters
validParams<ComputePFFractureStressBase>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addRequiredCoupledVar("c", "Name of damage variable");
  params.addParam<bool>(
      "use_current_history_variable", false, "Use the current value of the history variable.");
  params.addParam<MaterialPropertyName>("barrier_energy",
                                        "Name of material property for fracture energy barrier.");
  params.addParam<MaterialPropertyName>(
      "E_name", "elastic_energy", "Name of material property for elastic energy");
  params.addParam<MaterialPropertyName>(
      "D_name", "degradation", "Name of material property for energetic degradation function.");
  params.addParam<MaterialPropertyName>(
      "F_name",
      "local_fracture_energy",
      "Name of material property for local fracture energy function.");
  return params;
}

ComputePFFractureStressBase::ComputePFFractureStressBase(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _c(coupledValue("c")),
    _l(getMaterialProperty<Real>("l")),
    _gc(getMaterialProperty<Real>("gc_prop")),
    _use_current_hist(getParam<bool>("use_current_history_variable")),
    _H(declareProperty<Real>("hist")),
    _H_old(getMaterialPropertyOld<Real>("hist")),
    _barrier(getDefaultMaterialProperty<Real>("barrier_energy")),
    _E(declareProperty<Real>(getParam<MaterialPropertyName>("E_name"))),
    _dEdc(declarePropertyDerivative<Real>(getParam<MaterialPropertyName>("E_name"),
                                          getVar("c", 0)->name())),
    _d2Ed2c(declarePropertyDerivative<Real>(
        getParam<MaterialPropertyName>("E_name"), getVar("c", 0)->name(), getVar("c", 0)->name())),
    _dstress_dc(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "stress", getVar("c", 0)->name())),
    _d2Fdcdstrain(declareProperty<RankTwoTensor>("d2Fdcdstrain")),
    _D(getMaterialProperty<Real>("D_name")),
    _dDdc(getMaterialPropertyDerivative<Real>("D_name", getVar("c", 0)->name())),
    _d2Dd2c(getMaterialPropertyDerivative<Real>(
        "D_name", getVar("c", 0)->name(), getVar("c", 0)->name()))
{
}

void
ComputePFFractureStressBase::initQpStatefulProperties()
{
  _H[_qp] = 0.0;
}
