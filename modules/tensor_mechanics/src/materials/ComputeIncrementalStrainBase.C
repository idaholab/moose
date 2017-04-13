/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeIncrementalStrainBase.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ComputeIncrementalStrainBase>()
{
  InputParameters params = validParams<ComputeStrainBase>();
  return params;
}

ComputeIncrementalStrainBase::ComputeIncrementalStrainBase(const InputParameters & parameters)
  : ComputeStrainBase(parameters),
    _grad_disp_old(3),
    _strain_rate(declareProperty<RankTwoTensor>(_base_name + "strain_rate")),
    _strain_increment(declareProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _rotation_increment(declareProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _deformation_gradient(declareProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _mechanical_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "mechanical_strain")),
    _total_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _eigenstrains_old(_eigenstrain_names.size())
{
  for (unsigned int i = 0; i < _eigenstrains_old.size(); ++i)
    _eigenstrains_old[i] = &getMaterialPropertyOld<RankTwoTensor>(_eigenstrain_names[i]);

  // fetch coupled old displacement gradient, setting components for unused dimensions to zero
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (_fe_problem.isTransient() && i < _ndisp)
      _grad_disp_old[i] = &coupledGradientOld("displacements", i);
    else
      _grad_disp_old[i] = &_grad_zero;
  }
}

void
ComputeIncrementalStrainBase::initQpStatefulProperties()
{
  _mechanical_strain[_qp].zero();
  _total_strain[_qp].zero();
  _deformation_gradient[_qp].zero();
  _deformation_gradient[_qp].addIa(1.0);

  // Note that for some models (small strain), the rotation increment is
  // never updated. Because we always have stateful properties, this method
  // always gets called, so we can rely on this getting set here without
  // setting it again when properties get computed.
  _rotation_increment[_qp].zero();
  _rotation_increment[_qp].addIa(1.0);
}

void
ComputeIncrementalStrainBase::subtractEigenstrainIncrementFromStrain(RankTwoTensor & strain)
{
  for (unsigned int i = 0; i < _eigenstrains.size(); ++i)
  {
    strain -= (*_eigenstrains[i])[_qp];
    strain += (*_eigenstrains_old[i])[_qp];
  }
}
