//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADBASE_H
#define INSADBASE_H

#include "ADKernel.h"

// Forward Declarations
template <ComputeStage>
class INSADBase;
namespace libMesh
{
class Elem;
}

declareADValidParams(INSADBase);

template <ComputeStage compute_stage>
using INSVectorValue = typename Moose::ValueType<compute_stage, VectorValue<Real>>::type;
template <ComputeStage compute_stage>
using INSReal = typename Moose::RealType<compute_stage>::type;

/**
 * This class computes strong and weak components of the INS governing
 * equations.  These terms can then be assembled in child classes
 */
template <ComputeStage compute_stage>
class INSADBase : public ADKernel<compute_stage>
{
public:
  INSADBase(const InputParameters & parameters);

  virtual ~INSADBase();

protected:
  virtual ADResidual computeQpResidual() = 0;

  virtual INSVectorValue<compute_stage> convectiveTerm();

  virtual INSVectorValue<compute_stage> weakViscousTermLaplace(unsigned comp);
  virtual INSVectorValue<compute_stage> weakViscousTermTraction(unsigned comp);

  virtual INSVectorValue<compute_stage> strongPressureTerm();
  virtual INSReal<compute_stage> weakPressureTerm();

  virtual INSVectorValue<compute_stage> gravityTerm();

  virtual INSVectorValue<compute_stage> timeDerivativeTerm();

  virtual INSReal<compute_stage> tau();

  /// Provides tau which yields superconvergence for 1D advection-diffusion
  virtual INSReal<compute_stage> tauNodal();

  void computeHMax();

  // Coupled variables
  const ADVariableValue & _u_vel;
  const ADVariableValue & _v_vel;
  const ADVariableValue & _w_vel;
  const ADVariableValue & _p;

  // Gradients
  const ADVariableGradient & _grad_u_vel;
  const ADVariableGradient & _grad_v_vel;
  const ADVariableGradient & _grad_w_vel;
  const ADVariableGradient & _grad_p;

  // Time derivatives
  const ADVariableValue & _u_vel_dot;
  const ADVariableValue & _v_vel_dot;
  const ADVariableValue & _w_vel_dot;

  RealVectorValue _gravity;

  // Material properties
  const ADMaterialProperty(Real) & _mu;
  const ADMaterialProperty(Real) & _rho;

  const Real _alpha;
  bool _laplace;
  bool _convective_term;
  bool _transient_term;

  MooseArray<typename Moose::RealType<compute_stage>::type> _tau;
  typename Moose::RealType<compute_stage>::type _hmax;

  usingKernelMembers;
};

#define usingINSBaseMembers                                                                        \
  usingKernelMembers;                                                                              \
  using INSADBase<compute_stage>::_u_vel;                                                          \
  using INSADBase<compute_stage>::_v_vel;                                                          \
  using INSADBase<compute_stage>::_w_vel;                                                          \
  using INSADBase<compute_stage>::_grad_u_vel;                                                     \
  using INSADBase<compute_stage>::_grad_v_vel;                                                     \
  using INSADBase<compute_stage>::_grad_w_vel;                                                     \
  using INSADBase<compute_stage>::_u_vel_dot;                                                      \
  using INSADBase<compute_stage>::_v_vel_dot;                                                      \
  using INSADBase<compute_stage>::_w_vel_dot;                                                      \
  using INSADBase<compute_stage>::_laplace;                                                        \
  using INSADBase<compute_stage>::_transient_term;                                                 \
  using INSADBase<compute_stage>::_convective_term;                                                \
  using INSADBase<compute_stage>::_rho;                                                            \
  using INSADBase<compute_stage>::_mu;                                                             \
  using INSADBase<compute_stage>::_gravity;                                                        \
  using INSADBase<compute_stage>::_grad_p

#endif
