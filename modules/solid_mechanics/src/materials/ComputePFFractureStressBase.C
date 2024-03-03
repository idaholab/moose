//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputePFFractureStressBase.h"

InputParameters
ComputePFFractureStressBase::validParams()
{
  InputParameters params = ComputeStressBase::validParams();
  params.addRequiredCoupledVar("c", "Name of damage variable");
  params.addParam<bool>(
      "use_current_history_variable", false, "Use the current value of the history variable.");
  params.addParam<bool>("use_snes_vi_solver",
                        false,
                        "Use PETSc's SNES variational inequalities solver to enforce damage "
                        "irreversibility condition and restrict damage value <= 1.");
  params.addParam<MaterialPropertyName>("barrier_energy",
                                        "Name of material property for fracture energy barrier.");
  params.addParam<MaterialPropertyName>(
      "E_name", "elastic_energy", "Name of material property for elastic energy");
  params.addParam<MaterialPropertyName>(
      "D_name", "degradation", "Name of material property for energetic degradation function.");
  params.addParam<MaterialPropertyName>(
      "I_name", "indicator", "Name of material property for damage indicator function.");
  params.addParam<MaterialPropertyName>(
      "F_name",
      "local_fracture_energy",
      "Name of material property for local fracture energy function.");
  return params;
}

ComputePFFractureStressBase::ComputePFFractureStressBase(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name)),
    _c(coupledValue("c")),
    _l(getMaterialProperty<Real>("l")),
    _gc(getMaterialProperty<Real>("gc_prop")),
    _pressure(getDefaultMaterialProperty<Real>("fracture_pressure")),
    _use_current_hist(getParam<bool>("use_current_history_variable")),
    _use_snes_vi_solver(getParam<bool>("use_snes_vi_solver")),
    _H(declareProperty<Real>("hist")),
    _H_old(getMaterialPropertyOld<Real>("hist")),
    _barrier(getDefaultMaterialProperty<Real>("barrier_energy")),
    _E(declareProperty<Real>(getParam<MaterialPropertyName>("E_name"))),
    _dEdc(declarePropertyDerivative<Real>(getParam<MaterialPropertyName>("E_name"),
                                          coupledName("c", 0))),
    _d2Ed2c(declarePropertyDerivative<Real>(
        getParam<MaterialPropertyName>("E_name"), coupledName("c", 0), coupledName("c", 0))),
    _dstress_dc(
        declarePropertyDerivative<RankTwoTensor>(_base_name + "stress", coupledName("c", 0))),
    _d2Fdcdstrain(declareProperty<RankTwoTensor>("d2Fdcdstrain")),
    _D(getMaterialProperty<Real>("D_name")),
    _dDdc(getMaterialPropertyDerivative<Real>("D_name", coupledName("c", 0))),
    _d2Dd2c(
        getMaterialPropertyDerivative<Real>("D_name", coupledName("c", 0), coupledName("c", 0))),
    _I(getDefaultMaterialProperty<Real>("I_name")),
    _dIdc(getMaterialPropertyDerivative<Real>("I_name", coupledName("c", 0))),
    _d2Id2c(getMaterialPropertyDerivative<Real>("I_name", coupledName("c", 0), coupledName("c", 0)))
{
}

void
ComputePFFractureStressBase::initQpStatefulProperties()
{
  _H[_qp] = 0.0;
}
