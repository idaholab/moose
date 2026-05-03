#pragma once

#include "LinearFVElementalKernel.h"
#include "TimeIntegrator.h"
#include "NS.h"

/**
 * Conservative linear FV time derivative for weakly-compressible momentum:
 * d(rho * u) / dt.
 *
 * The generic LinearFVTimeDerivative represents factor * du/dt. For variable-density
 * momentum we instead need the time derivative of the product rho*u, which requires
 * old-state density history on the right-hand side.
 */
class LinearWCNSFVMomentumTimeDerivative : public LinearFVElementalKernel
{
public:
  static InputParameters validParams();

  LinearWCNSFVMomentumTimeDerivative(const InputParameters & params);

  virtual Real computeMatrixContribution() override;
  virtual Real computeRightHandSideContribution() override;
  virtual void setCurrentElemInfo(const ElemInfo * elem_info) override;

protected:
  /// Density functor
  const Moose::Functor<Real> & _rho;

  /// The time integrator to use in this kernel
  const TimeIntegrator & _time_integrator;

private:
  /// Old-state density history aligned with the time integrator RHS convention:
  /// [rho_old, rho_older, ...]
  std::vector<Real> _rho_history;

  /// State args used to fetch old-state density history
  std::vector<Moose::StateArg> _history_state_args;
};
