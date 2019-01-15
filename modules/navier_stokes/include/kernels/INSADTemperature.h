//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADTEMPERATURE_H
#define INSADTEMPERATURE_H

#include "INSADBase.h"

// Forward Declarations
template <ComputeStage>
class INSADTemperature;

declareADValidParams(INSADTemperature);

/**
 * This class computes the residual and Jacobian contributions for the
 * incompressible Navier-Stokes temperature (energy) equation.
 */
template <ComputeStage compute_stage>
class INSADTemperature : public INSADBase<compute_stage>
{
public:
  INSADTemperature(const InputParameters & parameters);

  virtual ~INSADTemperature() {}

protected:
  virtual ADResidual computeQpResidual() override;
  void beforeQpLoop() override;
  void beforeTestLoop() override;
  void computeTestTerms();
  void computeGradTestTerms();

  // Required parameters
  const ADMaterialProperty(Real) & _k;
  const ADMaterialProperty(Real) & _cp;
  const ADMaterialProperty(RealVectorValue) & _grad_k;

  const ADVariableValue & _u_dot;

  const bool _supg;

  typename Moose::RealType<compute_stage>::type _test_terms;
  VectorValue<typename Moose::RealType<compute_stage>::type> _grad_test_terms;
  VectorValue<typename Moose::RealType<compute_stage>::type> _U;
  typename Moose::RealType<compute_stage>::type _strong_convective_part;

  usingINSBaseMembers;
};

#endif // INSADTEMPERATURE_H
