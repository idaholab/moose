//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeCrystalPlasticityVolumetricEigenstrain.h"

registerMooseObject("SolidMechanicsApp", ComputeCrystalPlasticityVolumetricEigenstrain);

InputParameters
ComputeCrystalPlasticityVolumetricEigenstrain::validParams()
{
  InputParameters params = ComputeCrystalPlasticityEigenstrainBase::validParams();
  params.addClassDescription("Computes the deformation gradient from the volumetric eigenstrain "
                             "due to spherical voids in a crystal plasticity simulation");
  params.addRequiredParam<MaterialPropertyName>(
      "spherical_void_number_density",
      "The material property name of the number density of the spherical voids, in 1/mm^3.");
  params.addRequiredParam<MaterialPropertyName>(
      "mean_spherical_void_radius",
      "The material property name for the mean radius value, in mm, for the spherical voids");

  return params;
}

ComputeCrystalPlasticityVolumetricEigenstrain::ComputeCrystalPlasticityVolumetricEigenstrain(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeCrystalPlasticityEigenstrainBase>(parameters),
    _void_density(getMaterialProperty<Real>("spherical_void_number_density")),
    _void_density_old(getMaterialPropertyOld<Real>("spherical_void_number_density")),
    _void_radius(getMaterialProperty<Real>("mean_spherical_void_radius")),
    _void_radius_old(getMaterialPropertyOld<Real>("mean_spherical_void_radius")),
    _equivalent_linear_change(declareProperty<Real>("equivalent_linear_change"))
{
}

void
ComputeCrystalPlasticityVolumetricEigenstrain::computeQpDeformationGradient()
{
  // check that the values of the radius and the density are both positive
  if (_void_radius[_qp] < 0.0)
    mooseException("A negative mean spherical void radius value, ",
                   _void_radius[_qp],
                   ", has been provided; this value is "
                   "non-physical and violates the assumptions of this eigenstrain class");
  if (_void_density[_qp] < 0.0)
    mooseException(
        "A negative, non-physical spherical void number density has been provided: ",
        _void_density[_qp],
        ". This value is non-physical and violates the assumptions of this eigenstrain class");

  // compute the linear commponent of the current and old volume due to the voids
  _equivalent_linear_change[_qp] =
      computeLinearComponentVolume(_void_radius[_qp], _void_density[_qp]);
  Real previous_linear =
      computeLinearComponentVolume(_void_radius_old[_qp], _void_density_old[_qp]);

  const Real linear_increment = _equivalent_linear_change[_qp] - previous_linear;

  // scale by the ratio of the substep to full time step for consistency
  // in cases where substepping is used
  RankTwoTensor eigenstrain = RankTwoTensor::Identity() * linear_increment * _substep_dt / _dt;

  // Rotate the eigenstrain for the crystal deformation gradient with Euler angles
  RankTwoTensor residual_equivalent_volumetric_expansion_increment =
      RankTwoTensor::Identity() - eigenstrain.rotated(_crysrot[_qp]);

  _deformation_gradient[_qp] =
      residual_equivalent_volumetric_expansion_increment.inverse() * _deformation_gradient_old[_qp];
}

Real
ComputeCrystalPlasticityVolumetricEigenstrain::computeLinearComponentVolume(const Real & radius,
                                                                            const Real & density)
{
  const Real cb_radius = Utility::pow<3>(radius);
  const Real volume = 4.0 * (libMesh::pi)*cb_radius * density / 3.0;
  const Real linear_component = std::cbrt(volume);

  return linear_component;
}
