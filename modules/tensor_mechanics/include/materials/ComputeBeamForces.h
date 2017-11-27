/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEBEAMFORCES_H
#define COMPUTEBEAMFORCES_H

#include "Material.h"
#include "RankTwoTensor.h"

/**
 * ComputeBeamForces computes forces and moments using elasticity
 */

class ComputeBeamForces;

template <>
InputParameters validParams<ComputeBeamForces>();

class ComputeBeamForces : public Material
{
public:
  ComputeBeamForces(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  std::string _base_name;

  const MaterialProperty<RealVectorValue> & _disp_strain_increment;
  const MaterialProperty<RealVectorValue> & _rot_strain_increment;
  const MaterialProperty<RealVectorValue> & _material_stiffness;
  const MaterialProperty<RealVectorValue> & _material_flexure;
  const MaterialProperty<RankTwoTensor> & _total_rotation;

  MaterialProperty<RealVectorValue> & _force;
  MaterialProperty<RealVectorValue> & _moment;
  const MaterialProperty<RealVectorValue> & _force_old;
  const MaterialProperty<RealVectorValue> & _moment_old;
};

#endif // COMPUTEBEAMFORCES_H
