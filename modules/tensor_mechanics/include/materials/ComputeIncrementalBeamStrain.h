/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEINCREMENTALBEAMSTRAIN_H
#define COMPUTEINCREMENTALBEAMSTRAIN_H

#include "Material.h"
#include "RankTwoTensor.h"
/**
 * ComputeIncrementalBeamStrain defines a displacement and rotation strain increment and rotation
 * increment (=1), for small strains.
 */

// Forward Declarations
class ComputeIncrementalBeamStrain;

template <>
InputParameters validParams<ComputeIncrementalBeamStrain>();

class ComputeIncrementalBeamStrain : public Material
{
public:
  ComputeIncrementalBeamStrain(const InputParameters & parameters);

protected:
  virtual void computeProperties() override;
  virtual void initQpStatefulProperties() override;

  // Compute the displacement and rotation strain increment
  void computeQpStrain();

  // Compute the stiffness matrices
  void computeJacobian();

  /// Coupled displacement variables
  unsigned int _nrot;
  unsigned int _ndisp;
  std::vector<unsigned int> _rot_num;
  std::vector<unsigned int> _disp_num;

  std::string _base_name;

  const VariableValue & _area;
  const VariableValue & _Ay;
  const VariableValue & _Az;
  const VariableValue & _Iy;
  const VariableValue & _Iz;

  MaterialProperty<RankTwoTensor> & _original_local_config;
  MaterialProperty<Real> & _original_length;
  MaterialProperty<RankTwoTensor> & _total_rotation;
  MaterialProperty<RealVectorValue> & _disp_strain_increment;
  MaterialProperty<RealVectorValue> & _rot_strain_increment;
  const MaterialProperty<RealVectorValue> & _material_stiffness;
  MaterialProperty<RankTwoTensor> & _K11;
  MaterialProperty<RankTwoTensor> & _K21_cross;
  MaterialProperty<RankTwoTensor> & _K21;
  MaterialProperty<RankTwoTensor> & _K22;
  MaterialProperty<RankTwoTensor> & _K22_cross;
  const bool _large_strain;

  RealVectorValue _grad_disp_0_local_t;
  RealVectorValue _grad_rot_0_local_t;
  RealVectorValue _avg_rot_local_t;
};

#endif // COMPUTEINCREMENTALBEAMSTRAIN_H
