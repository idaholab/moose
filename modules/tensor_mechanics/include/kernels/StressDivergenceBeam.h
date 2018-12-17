/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef STRESSDIVERGENCEBEAM_H
#define STRESSDIVERGENCEBEAM_H

#include "Kernel.h"

// Forward Declarations
class StressDivergenceBeam;
template <typename>
class RankTwoTensorTempl;
typedef RankTwoTensorTempl<Real> RankTwoTensor;

template <>
InputParameters validParams<StressDivergenceBeam>();

class StressDivergenceBeam : public Kernel
{
public:
  StressDivergenceBeam(const InputParameters & parameters);
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(MooseVariableFEBase & jvar) override;
  using Kernel::computeOffDiagJacobian;

protected:
  virtual Real computeQpResidual() override { return 0.0; }

  /// Computes the force and moment due to stiffness proportional damping and HHT time integration
  void computeDynamicTerms(std::vector<RealVectorValue> & global_force_res,
                           std::vector<RealVectorValue> & global_moment_res);

  /// Computes the residual corresponding to displacement and rotational variables given the forces and moments
  void computeGlobalResidual(const MaterialProperty<RealVectorValue> * force,
                             const MaterialProperty<RealVectorValue> * moment,
                             const MaterialProperty<RankTwoTensor> * total_rotation,
                             std::vector<RealVectorValue> & global_force_res,
                             std::vector<RealVectorValue> & global_moment_res);

  /// Direction along which force/moment is calculated
  const unsigned int _component;

  /// Number of coupled displacement variables
  unsigned int _ndisp;

  /// Variable numbers corresponding to displacement variables
  std::vector<unsigned int> _disp_var;

  /// Number of coupled rotational variables
  unsigned int _nrot;

  /// Variable numbers corresponding to rotational variables
  std::vector<unsigned int> _rot_var;

  /// Current force vector in global coordinate system
  const MaterialProperty<RealVectorValue> & _force;

  /// Current moment vector in global coordinate system
  const MaterialProperty<RealVectorValue> & _moment;

  /// Stiffness matrix relating displacement DOFs of same node or across nodes
  const MaterialProperty<RankTwoTensor> & _K11;

  /// Stiffness matrix relating rotational DOFs of same node
  const MaterialProperty<RankTwoTensor> & _K22;

  /// Stiffness matrix relating rotational DOFs across nodes
  const MaterialProperty<RankTwoTensor> & _K22_cross;

  /// Stiffness matrix relating displacement of one node to rotations of another node
  const MaterialProperty<RankTwoTensor> & _K21_cross;

  /// Stiffness matrix relating displacement and rotations of same node
  const MaterialProperty<RankTwoTensor> & _K21;

  /// Initial length of beam
  const MaterialProperty<Real> & _original_length;

  /// Rotational transformation from global to current beam local coordinate system
  const MaterialProperty<RankTwoTensor> & _total_rotation;

  /// Stiffness proportional Rayleigh damping parameter
  const MaterialProperty<Real> & _zeta;

  /// HHT time integration parameter
  const Real & _alpha;

  /// Boolean flag to turn on Rayleigh damping or numerical damping due to HHT time integration
  const bool _isDamped;

  /// Old force vector in global coordinate system
  const MaterialProperty<RealVectorValue> * _force_old;

  /// Old moment vector in global coordinate system
  const MaterialProperty<RealVectorValue> * _moment_old;

  /// Rotational transformation from global to old beam local coordinate system
  const MaterialProperty<RankTwoTensor> * _total_rotation_old;

  /// Older force vector in global coordinate system
  const MaterialProperty<RealVectorValue> * _force_older;

  /// Older moment vector in global coordinate system
  const MaterialProperty<RealVectorValue> * _moment_older;

  /// Rotational transformation from global to older beam local coordinate system
  const MaterialProperty<RankTwoTensor> * _total_rotation_older;

  /// Residual corresponding to displacement DOFs at the nodes in global coordinate system
  std::vector<RealVectorValue> _global_force_res;

  /// Residual corresponding to rotational DOFs at the nodes in global coordinate system
  std::vector<RealVectorValue> _global_moment_res;

  /// Forces at each Qp in the beam local configuration
  std::vector<RealVectorValue> _force_local_t;

  /// Moments at each Qp in the beam local configuration
  std::vector<RealVectorValue> _moment_local_t;

  /// Residual corresponding to displacement DOFs at the nodes in beam local coordinate system
  std::vector<RealVectorValue> _local_force_res;

  /// Residual corresponding to rotational DOFs at the nodes in beam local coordinate system
  std::vector<RealVectorValue> _local_moment_res;
};

#endif // STRESSDIVERGENCEBEAM_H
