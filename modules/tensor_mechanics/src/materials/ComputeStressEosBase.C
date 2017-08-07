/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeStressEosBase.h"

template <>
InputParameters
validParams<ComputeStressEosBase>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Adds a volumetric extra stress including a Birch-Murnaghan EOS"
                             "and bulk viscosity damping for shock propagation"
                             "that is substituted to the volumetric stress "
                             "calculated by the constitutive model");
  params.addRequiredParam<Real>("n_Murnaghan", "exponent in Birch-Murnaghan EOS");
  params.addRequiredParam<Real>("bulk_modulus_ref",
                                "reference bulk modulus in Birch-Murnaghan EOS");
  params.addRequiredParam<Real>("C0", "Von Neumann damping coefficient");
  params.addRequiredParam<Real>("C1", "Landshoff damping coefficient");
  return params;
}

ComputeStressEosBase::ComputeStressEosBase(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _n_Murnaghan(getParam<Real>("n_Murnaghan")),
    _Bulk_Modulus_Ref(getParam<Real>("bulk_modulus_ref")),
    _C0(getParam<Real>("C0")),
    _C1(getParam<Real>("C1")),
    _deformation_gradient(getMaterialProperty<RankTwoTensor>("deformation_gradient")),
    _deformation_gradient_old(getMaterialPropertyOld<RankTwoTensor>("deformation_gradient"))
{
}

void
ComputeStressEosBase::computeQpProperties()
{
  RankTwoTensor volumetric_stress, EOS_stress, bulk_viscosity_stress;
  Real dspecific_volume_dt;

  computeQpStress();

  volumetric_stress.zero();
  // Calculate volumetric stress
  volumetric_stress.addIa(_stress[_qp].trace() / 3.0);

  EOS_stress.zero();
  // Birch-Murnaghan EOS
  EOS_stress.addIa((_Bulk_Modulus_Ref / _n_Murnaghan) *
                   (1.0 - std::pow(1.0 / _deformation_gradient[_qp].det(), _n_Murnaghan)));

  // Calculate rate of change of the specific volume
  dspecific_volume_dt =
      (_deformation_gradient[_qp].det() - _deformation_gradient_old[_qp].det()) / _dt;
  dspecific_volume_dt /= _deformation_gradient_old[_qp].det();

  bulk_viscosity_stress.zero();
  // Calculate bulk viscosity damping
  bulk_viscosity_stress.addIa(_C0 * dspecific_volume_dt * abs(dspecific_volume_dt));
  bulk_viscosity_stress.addIa(_C1 * dspecific_volume_dt);

  // Subtract the original volumetric stress, add the one given by the EOS and bulk viscosity
  _stress[_qp] = _stress[_qp] + EOS_stress + bulk_viscosity_stress - volumetric_stress;

  // Add in extra stress
  _stress[_qp] += _extra_stress[_qp];
}
