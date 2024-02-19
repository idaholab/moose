//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeCrystalPlasticityEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"

/**
 * ComputeCrystalPlasticityVolumetricEigenstrain computes an eigenstrain for swelling due
 * to spherical voids in the crystal grain interiors. The formulations for volumetric
 * deformation gradient and volumetric eigenstrain are as follows:
 *    $\dot{Fv} * Fv^{-1} = \dot{_volumetric_strain}$
 *    volumetric_eigenstrain = 0.5 * (Fv^{T} * Fv - I)
 */
class ComputeCrystalPlasticityVolumetricEigenstrain
  : public DerivativeMaterialInterface<ComputeCrystalPlasticityEigenstrainBase>
{
public:
  static InputParameters validParams();

  ComputeCrystalPlasticityVolumetricEigenstrain(const InputParameters & parameters);

protected:
  ///Compute the deformation gradient due to volumetric expansion
  virtual void computeQpDeformationGradient() override;

  /**
   * Computes the linear component of the volume change due to voids,
   * assuming perfect spheres with the same radius value
   */
  virtual Real computeLinearComponentVolume(const Real & radius, const Real & density);

  /**
   * Spherical void number density, in 1/mm^3, as computed by a separate material.
   *  Note that this value is the OLD material property and thus lags the current
   * value by a single timestep.
   */
  const MaterialProperty<Real> & _void_density;
  const MaterialProperty<Real> & _void_density_old;

  /**
   * Mean spherical void radius, in mm, as computed by a separate material.
   *  Note that this value is the OLD material property and thus lags the current
   * value by a single timestep.
   */
  const MaterialProperty<Real> & _void_radius;
  const MaterialProperty<Real> & _void_radius_old;

  /**
   * Linear strain value, to be multiplied by the identity matrix, resulting
   * from the current volume change conversion
   */
  MaterialProperty<Real> & _equivalent_linear_change;
};
