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

  /// Computes the displacement and rotation strain increments
  void computeQpStrain();

  /// Computes the stiffness matrices
  void computeJacobian();

  /// Number of coupled rotational variables
  unsigned int _nrot;

  /// Number of coupled displacement variables
  unsigned int _ndisp;

  /// Variable numbers corresponding to the rotational variables
  std::vector<unsigned int> _rot_num;

  /// Variable numbers corresponding to the displacement variables
  std::vector<unsigned int> _disp_num;

  /// Coupled variable for the beam cross-sectional area
  const VariableValue & _area;

  /// Coupled variable for the first moment of area in y direction, i.e., integral of y*dA over the cross-section
  const VariableValue & _Ay;

  /// Coupled variable for the first moment of area in z direction, i.e., integral of z*dA over the cross-section
  const VariableValue & _Az;

  /// Coupled variable for the second moment of area in y direction, i.e., integral of y^2*dA over the cross-section
  const VariableValue & _Iy;

  /// Coupled variable for the second moment of area in z direction, i.e., integral of z^2*dA over the cross-section
  const VariableValue & _Iz;

  /// Rotational transformation from global coordinate system to initial beam local configuration
  MaterialProperty<RankTwoTensor> & _original_local_config;

  /// Initial length of the beam
  MaterialProperty<Real> & _original_length;

  /// Rotational transformation from global coordinate system to beam local configuration at time t
  MaterialProperty<RankTwoTensor> & _total_rotation;

  /// Current total displacement strain integrated over the cross-section in global coordinate system.
  MaterialProperty<RealVectorValue> & _total_disp_strain;

  /// Current total rotational strain integrated over the cross-section in global coordinate system.
  MaterialProperty<RealVectorValue> & _total_rot_strain;

  /// Old total displacement strain integrated over the cross-section in global coordinate system.
  const MaterialProperty<RealVectorValue> & _total_disp_strain_old;

  /// Old total rotational strain integrated over the cross-section in global coordinate system.
  const MaterialProperty<RealVectorValue> & _total_rot_strain_old;

  /// Mechanical displacement strain increment (after removal of eigenstrains) integrated over the cross-section.
  MaterialProperty<RealVectorValue> & _mech_disp_strain_increment;

  /// Mechanical rotation strain increment (after removal of eigenstrains) integrated over the cross-section
  MaterialProperty<RealVectorValue> & _mech_rot_strain_increment;

  /// Material stiffness vector that relates displacement strain increments to force increments
  const MaterialProperty<RealVectorValue> & _material_stiffness;

  /// Stiffness matrix between displacement DOFs of same node or across nodes
  MaterialProperty<RankTwoTensor> & _K11;

  /// Stiffness matrix between displacement DOFs of one node to rotational DOFs of another node
  MaterialProperty<RankTwoTensor> & _K21_cross;

  /// Stiffness matrix between displacement DOFs and rotation DOFs of the same node
  MaterialProperty<RankTwoTensor> & _K21;

  /// Stiffness matrix between rotation DOFs of the same node
  MaterialProperty<RankTwoTensor> & _K22;

  /// Stiffness matrix between rotation DOFs of different nodes
  MaterialProperty<RankTwoTensor> & _K22_cross;

  /// Boolean flag to turn on large strain calculation
  const bool _large_strain;

  /// Gradient of displacement calculated in the beam local configuration at time t
  RealVectorValue _grad_disp_0_local_t;

  /// Gradient of rotation calculated in the beam local configuration at time t
  RealVectorValue _grad_rot_0_local_t;

  /// Average rotation calculated in the beam local configuration at time t
  RealVectorValue _avg_rot_local_t;

  /// Vector of beam eigenstrain names
  std::vector<MaterialPropertyName> _eigenstrain_names;

  /// Vector of current displacement eigenstrains
  std::vector<const MaterialProperty<RealVectorValue> *> _disp_eigenstrain;

  /// Vector of current rotational eigenstrains
  std::vector<const MaterialProperty<RealVectorValue> *> _rot_eigenstrain;

  /// Vector of old displacement eigenstrains
  std::vector<const MaterialProperty<RealVectorValue> *> _disp_eigenstrain_old;

  /// Vector of old rotational eigenstrains
  std::vector<const MaterialProperty<RealVectorValue> *> _rot_eigenstrain_old;
};

#endif // COMPUTEINCREMENTALBEAMSTRAIN_H
