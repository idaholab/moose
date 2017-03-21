/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MOMENTBALANCING_H
#define MOMENTBALANCING_H

#include "Kernel.h"

// Forward Declarations
class MomentBalancing;
class RankFourTensor;
class RankTwoTensor;

template <>
InputParameters validParams<MomentBalancing>();

/**
 * This Kernel computes
 * epsilon_ijk * stress_jk (sum over j and k)
 * "i" is called _component in this class
 * and epsilon is the permutation pseudo-tensor
 *
 * This Kernel is added to CosseratStressDivergenceTensors
 * to form the equilibrium equations for the
 * Cosserat moment-stress.
 */
class MomentBalancing : public Kernel
{
public:
  MomentBalancing(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// the stress tensor (not the moment stress) at the quad-point.
  const MaterialProperty<RankTwoTensor> & _stress;

  /**
   * d(stress tensor)/(d strain tensor)
   * Here strain_ij = grad_j disp_i + epsilon_ijk * wc_k
   */
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;

  /// The Kernel computes epsilon_{component j k}*stress_{j k}
  const unsigned int _component;

  // number of Cosserat rotation components provided by the user
  const unsigned int _nrots;

  /// the moose variable numbers for the Cosserat rotation degrees of freedom
  std::vector<unsigned int> _wc_var;

  // number of displacement components provided by the user
  const unsigned int _ndisp;

  /// the moose variable numbers for the displacements
  std::vector<unsigned int> _disp_var;
};

#endif // MOMENTBALANCING_H
