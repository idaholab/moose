/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEBEAMRESULTANTS_H
#define COMPUTEBEAMRESULTANTS_H

#include "Material.h"
#include "RankTwoTensor.h"

/**
 * ComputeBeamResultants computes forces and moments using elasticity
 */

class ComputeBeamResultants;

template <>
InputParameters validParams<ComputeBeamResultants>();

class ComputeBeamResultants : public Material
{
public:
  ComputeBeamResultants(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  /// Mechanical displacement strain increment in beam local coordinate system
  const MaterialProperty<RealVectorValue> & _disp_strain_increment;

  /// Mechanical rotational strain increment in beam local coordinate system
  const MaterialProperty<RealVectorValue> & _rot_strain_increment;

  /// Material stiffness vector that relates displacement strain increment to force increment
  const MaterialProperty<RealVectorValue> & _material_stiffness;

  /// Material flexure vector that relates rotational strain increment to moment increment
  const MaterialProperty<RealVectorValue> & _material_flexure;

  /// Rotational transformation from global to current beam local coordinate system
  const MaterialProperty<RankTwoTensor> & _total_rotation;

  /// Current force vector in global coordinate system
  MaterialProperty<RealVectorValue> & _force;

  /// Current moment vector in global coordinate system
  MaterialProperty<RealVectorValue> & _moment;

  /// Old force vector in global coordinate system
  const MaterialProperty<RealVectorValue> & _force_old;

  /// Old force vector in global coordinate system
  const MaterialProperty<RealVectorValue> & _moment_old;
};

#endif // COMPUTEBEAMRESULTANTS_H
