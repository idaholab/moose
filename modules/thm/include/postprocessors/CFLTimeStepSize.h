#pragma once

#include "ElementPostprocessor.h"

class CFLTimeStepSize;

template <>
InputParameters validParams<CFLTimeStepSize>();

/**
 * Computes a time step size based on user-specified CFL number
 */
class CFLTimeStepSize : public ElementPostprocessor
{
public:
  CFLTimeStepSize(const InputParameters & parameters);

  virtual void execute() override;

  virtual void initialize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// User-specified CFL number
  const Real _CFL;

  /// Velocity material property name(s)
  const std::vector<MaterialPropertyName> & _vel_names;
  /// Sound speed material property name(s)
  const std::vector<MaterialPropertyName> & _c_names;

  /// Number of phases
  const unsigned int _n_phases;

  /// Velocity material properties
  std::vector<const MaterialProperty<Real> *> _vel;
  /// Sound speed material properties
  std::vector<const MaterialProperty<Real> *> _c;

  /// Time step size
  Real _dt;
};
