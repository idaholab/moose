#pragma once

#include "NodalConstraint.h"

class JunctionEnergyConstraint;
class JunctionStagnationEnthalpyUserObject;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<JunctionEnergyConstraint>();

/**
 * Adds energy boundary fluxes for a junction
 *
 * Note that in MOOSE, this is a Constraint rather than a BC because currently
 * BC cannot see more than one subdomain, whereas constraints can.
 */
class JunctionEnergyConstraint : public NodalConstraint
{
public:
  JunctionEnergyConstraint(const InputParameters & parameters);

  virtual void computeResidual(NumericVector<Number> & residual);
  virtual void computeJacobian(SparseMatrix<Number> & jacobian);

protected:
  virtual Real computeQpResidual(Moose::ConstraintType type);
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type);

  const std::vector<Real> _normals;
  const std::vector<Real> _K;

  const VariableValue & _A;
  const VariableValue & _rhoA;
  const VariableValue & _rhouA;
  const VariableValue & _rhouA_old;
  const VariableValue & _rhoEA;
  const VariableValue & _s_junction;

  const unsigned int _rhoA_var_number;
  const unsigned int _rhouA_var_number;
  const unsigned int _rhoEA_var_number;
  const unsigned int _s_junction_var_number;

  const VariableValue & _vel;
  const VariableValue & _v;
  const VariableValue & _e;
  const VariableValue & _p;

  const JunctionStagnationEnthalpyUserObject & _H_junction_uo;

  const SinglePhaseFluidProperties & _fp;
};
