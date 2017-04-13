/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef TESTLAPBC_H
#define TESTLAPBC_H

#include "IntegratedBC.h"

// Forward Declarations
class TestLapBC;

template <>
InputParameters validParams<TestLapBC>();

/**
 * This BC enforces: grad(u) \cdot n = (1/2)*Lap(u) weakly.  This is
 * consistent with the exact solution, for which we know that:
 * grad(u).n = 2 on the right-hand boundary, and Lap(u) = 4
 * everywhere.  This is just an artificial way to introduce the
 * Laplacian into the BC.  There is a minus sign since this term is
 * moved to the LHS of the equation when the residual is formed.
 */
class TestLapBC : public IntegratedBC
{
public:
  TestLapBC(const InputParameters & parameters);
  virtual ~TestLapBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const VariableSecond & _second_u;
  const VariablePhiSecond & _second_phi;
  const VariableTestSecond & _second_test;
};

#endif // TESTLAPBC_H
