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
 * ComputeElasticityBeam defines the elasticity vector for the beam element
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
  virtual void initQpStatefulProperties() override;

  std::string _base_name;

  MaterialProperty<RealVectorValue> & _material_stiffness;
  MaterialProperty<RealVectorValue> & _material_flexure;

  /// prefactor function to multiply the elasticity tensor with
  Function * const _prefactor_function;

  const VariableValue & _youngs_modulus;
  const VariableValue & _shear_modulus;
  const VariableValue & _shear_coefficient;
};

#endif // COMPUTEELASTICITYBEAM_H
