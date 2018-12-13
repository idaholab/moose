//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADVECTIVEFLUXCALCULATORCONSTANTVELOCITY_H
#define ADVECTIVEFLUXCALCULATORCONSTANTVELOCITY_H

#include "AdvectiveFluxCalculatorBase.h"

/**
 * Computes Advective fluxes for a constant velocity
 **/
class AdvectiveFluxCalculatorConstantVelocity;

template <>
InputParameters validParams<AdvectiveFluxCalculatorConstantVelocity>();

class AdvectiveFluxCalculatorConstantVelocity : public AdvectiveFluxCalculatorBase
{
public:
  AdvectiveFluxCalculatorConstantVelocity(const InputParameters & parameters);

protected:
  virtual Real getInternodalVelocity(unsigned i, unsigned j, unsigned qp) const override;

  virtual Real getU(dof_id_type id) const override;

  /// advection velocity
  RealVectorValue _velocity;

  /// the nodal values of u
  MooseVariable * _u_nodal;

  /// the moose variable number of u
  unsigned _u_var_num;

  const VariableValue & _u_coupledNV;

  /// Kuzmin-Turek shape function
  const VariablePhiValue & _phi;

  /// grad(Kuzmin-Turek shape function)
  const VariablePhiGradient & _grad_phi;
};

#endif // ADVECTIVEFLUXCALCULATORCONSTANTVELOCITY_H
