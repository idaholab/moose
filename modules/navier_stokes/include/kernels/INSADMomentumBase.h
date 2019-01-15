//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADMOMENTUMBASE_H
#define INSADMOMENTUMBASE_H

#include "INSADBase.h"

// Forward Declarations
template <ComputeStage>
class INSADMomentumBase;

declareADValidParams(INSADMomentumBase);

/**
 * This class computes the momentum equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
template <ComputeStage compute_stage>
class INSADMomentumBase : public INSADBase<compute_stage>
{
public:
  INSADMomentumBase(const InputParameters & parameters);

  virtual ~INSADMomentumBase() {}

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
  using INSADMomentumBase<compute_stage>::_component;                                              \
  using INSADMomentumBase<compute_stage>::_integrate_p_by_parts;                                   \
  using INSADMomentumBase<compute_stage>::_supg;                                                   \
  using INSADMomentumBase<compute_stage>::_ffn

#endif // INSADMOMENTUMBASE_H
