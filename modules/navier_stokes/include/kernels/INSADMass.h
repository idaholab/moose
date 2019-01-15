//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSADMASS_H
#define INSADMASS_H

#include "INSADBase.h"

// Forward Declarations
template <ComputeStage>
class INSADMass;

declareADValidParams(INSADMass);

/**
 * This class computes the mass equation residual and Jacobian
 * contributions for the incompressible Navier-Stokes momentum
 * equation.
 */
template <ComputeStage compute_stage>
class INSADMass : public INSADBase<compute_stage>
{
public:
  INSADMass(const InputParameters & parameters);

  virtual ~INSADMass() {}

protected:
  virtual ADResidual computeQpResidual() override;

  void computeQpStrongResidual();
  void computeQpPGStrongResidual();

  void beforeTestLoop() override;
  void beforeQpLoop() override;

  bool _pspg;
  Function & _x_ffn;
  Function & _y_ffn;
  Function & _z_ffn;

  typename Moose::RealType<compute_stage>::type _strong_residual;
  VectorValue<typename Moose::RealType<compute_stage>::type> _strong_pg_residual;

  usingINSBaseMembers;
};

#endif // INSADMASS_H
