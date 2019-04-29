#pragma once

#include "NodalConstraint.h"
#include "PostprocessorInterface.h"

class OldJunctionMomentumConstraint;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<OldJunctionMomentumConstraint>();

/**
 * Momentum constraint for the Junction component
 */
class OldJunctionMomentumConstraint : public NodalConstraint
{
public:
  OldJunctionMomentumConstraint(const InputParameters & parameters);

  virtual void computeResidual(NumericVector<Number> & residual);
  virtual void computeJacobian(SparseMatrix<Number> & jacobian);

protected:
  virtual Real computeQpResidual(Moose::ConstraintType type);
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type);

  Real pressure(unsigned int i);

  std::vector<Real> _normals;

  // Coupled variables
  const VariableValue & _area;
  const VariableValue & _rhoA;
  const VariableValue & _rhouA;
  const VariableValue & _rhoEA;
  const VariableValue & _rho;
  const VariableValue & _vel;
  const VariableValue & _pressure_junction;

  const VariableValue & _total_mfr_in;
  const VariableValue & _total_int_energy_rate_in;

  /// Loss coefficient
  const std::vector<Real> & _k_coeff;
  /// reverse loss coefficient
  const std::vector<Real> & _kr_coeff;
  /// Reference area passed from the OldJunction component
  const Real & _ref_area;
  /// Initial density
  Real _initial_rho;

  unsigned int _rhoA_var_number;
  unsigned int _rhoEA_var_number;
  unsigned int _pbr_var_number;

  const SinglePhaseFluidProperties & _fp;
};
