//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADMOMENTUMADVECTION}_H
#define INSADMOMENTUMADVECTION}_H

#include "INSADBase.h"

// Forward Declarations
template <ComputeStage>
class INSADMomentumAdvection;

declareADValidParams(INSADMomentumAdvection);

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
template <ComputeStage compute_stage>
class INSADMomentumAdvection : public INSADBase<compute_stage>
{
public:
  INSADMomentumAdvection(const InputParameters & parameters);

  virtual ~INSADMomentumAdvection() {}

protected:
  virtual ADResidual computeQpResidual() override;
  void beforeQpLoop() override;
  void beforeTestLoop() override;
  void computeTestTerms();
  void computeGradTestTerms();
  void computeGradTestComponentTerms();

  unsigned _component;
  bool _integrate_p_by_parts;
  bool _supg;
  Function & _ffn;

  typename Moose::RealType<compute_stage>::type _test_terms;
  VectorValue<typename Moose::RealType<compute_stage>::type> _grad_test_terms;
  typename Moose::RealType<compute_stage>::type _grad_test_component_terms;

  usingINSBaseMembers;
};

#define usingINSMomentumBaseMembers                                                                \
  usingINSBaseMembers;                                                                             \
  using INSADMomentumAdvection<compute_stage>::_component;                                              \
  using INSADMomentumAdvection<compute_stage>::_integrate_p_by_parts;                                   \
  using INSADMomentumAdvection<compute_stage>::_supg;                                                   \
  using INSADMomentumAdvection<compute_stage>::_ffn

#endif // INSADMOMENTUMADVECTION}_H
