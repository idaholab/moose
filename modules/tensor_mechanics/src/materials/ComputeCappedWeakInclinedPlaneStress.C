/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeCappedWeakInclinedPlaneStress.h"
#include "RotationMatrix.h" // for rotVecToZ

template<>
InputParameters validParams<ComputeCappedWeakInclinedPlaneStress>()
{
  InputParameters params = validParams<ComputeCappedWeakPlaneStress>();
  params.addClassDescription("Capped weak inclined-plane plasticity stress calculator");
  params.addRequiredParam<RealVectorValue>("normal_vector", "The normal vector to the weak plane");
  return params;
}

ComputeCappedWeakInclinedPlaneStress::ComputeCappedWeakInclinedPlaneStress(const InputParameters & parameters) :
    ComputeCappedWeakPlaneStress(parameters),
    _n_input(getParam<RealVectorValue>("normal_vector")),
    _n(declareProperty<RealVectorValue>("weak_plane_normal")),
    _n_old(declarePropertyOld<RealVectorValue>("weak_plane_normal")),
    _rot(RealTensorValue())
{
  if (_n_input.norm() == 0)
    mooseError("ComputeCappedWeakInclinedPlaneStress: normal_vector must not have zero length");
  else
    _n_input /= _n_input.norm();
}

void
ComputeCappedWeakInclinedPlaneStress::initQpStatefulProperties()
{
  ComputeCappedWeakPlaneStress::initQpStatefulProperties();
  _n[_qp] = _n_input;
}

void
ComputeCappedWeakInclinedPlaneStress::computeQpStress()
{
  rotate();

  if (_t_step >= 2)
    _step_one = false;

  ComputeCappedWeakPlaneStress::returnMap();

  unrotate();

  //Update measures of strain
  _elastic_strain[_qp] = _elastic_strain_old[_qp] + _strain_increment[_qp] - (_plastic_strain[_qp] - _plastic_strain_old[_qp]);

  //Rotate the tensors to the current configuration
  if (_perform_finite_strain_rotations)
  {
    _stress[_qp] = _rotation_increment[_qp]*_stress[_qp]*_rotation_increment[_qp].transpose();
    _elastic_strain[_qp] = _rotation_increment[_qp] * _elastic_strain[_qp] * _rotation_increment[_qp].transpose();
    _plastic_strain[_qp] = _rotation_increment[_qp] * _plastic_strain[_qp] * _rotation_increment[_qp].transpose();

    // Rotate n by _rotation_increment
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    {
      _n[_qp](i) = 0.0;
      for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
        _n[_qp](i) += _rotation_increment[_qp](i, j) * _n_old[_qp](j);
    }
  }
}

void
ComputeCappedWeakInclinedPlaneStress::rotate()
{
  _rot = RotationMatrix::rotVecToZ(_n[_qp]);  // rotation matrix that will rotate _n to the "z" axis

  // _elasticity_tensor and _strain_increment are both "const" as usually in computing
  // stress these don't need to be altered.  Here, however, we need to rotate them.
  (const_cast<MaterialProperty<RankFourTensor> &>(_elasticity_tensor))[_qp].rotate(_rot);
  _stress_old[_qp].rotate(_rot);
  _plastic_strain_old[_qp].rotate(_rot);
  (const_cast<MaterialProperty<RankTwoTensor> &>(_strain_increment))[_qp].rotate(_rot);
}

void
ComputeCappedWeakInclinedPlaneStress::unrotate()
{
  _rot = _rot.transpose(); // rotation matrix will now rotate "z" axis back to _n

  (const_cast<MaterialProperty<RankFourTensor> &>(_elasticity_tensor))[_qp].rotate(_rot);
  _stress_old[_qp].rotate(_rot);
  _plastic_strain_old[_qp].rotate(_rot);
  (const_cast<MaterialProperty<RankTwoTensor> &>(_strain_increment))[_qp].rotate(_rot);
  _Jacobian_mult[_qp].rotate(_rot);
  _stress[_qp].rotate(_rot);
  _plastic_strain[_qp].rotate(_rot);
}

void
ComputeCappedWeakInclinedPlaneStress::errorHandler(const std::string & message)
{
  unrotate();
  ComputeCappedWeakPlaneStress::errorHandler(message);
}
