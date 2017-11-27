/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEELASTICITYBEAM_H
#define COMPUTEELASTICITYBEAM_H

#include "Material.h"

/**
 * ComputeElasticityBeam computes the equivalent of the elasticity tensor for the beam element,
 * which are vectors of material translational and flexural stiffness
 */

class ComputeElasticityBeam;

template <>
InputParameters validParams<ComputeElasticityBeam>();

class ComputeElasticityBeam : public Material
{
public:
  ComputeElasticityBeam(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Material stiffness vector that relates displacement strain increments to force increments
  MaterialProperty<RealVectorValue> & _material_stiffness;

  /// Material flexure vector that relates rotational strain increments to moment increments
  MaterialProperty<RealVectorValue> & _material_flexure;

  /// Prefactor function used to modify (i.e., multiply) the material stiffness and flexure vectors
  Function * const _prefactor_function;

  /// Young's modulus of the beam material
  const VariableValue & _youngs_modulus;

  /// Poisson's ratio of the beam material
  const VariableValue & _poissons_ratio;

  /// Shear coefficient for the beam cross-section
  const VariableValue & _shear_coefficient;
};

#endif // COMPUTEELASTICITYBEAM_H
