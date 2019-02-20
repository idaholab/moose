#ifndef FREEBOUNDARY_H
#define FREEBOUNDARY_H

#include "FlowBoundary.h"

class FreeBoundary;

template <>
InputParameters validParams<FreeBoundary>();

/**
 * Adds the boundary terms resulting from an integration by parts of the
 * advection terms, using no external boundary data.
 */
class FreeBoundary : public FlowBoundary
{
public:
  FreeBoundary(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void init() override;
  virtual void check() const override;

  void addMooseObjectsCG();
  void addMooseObjectsBoundaryFlux3Eqn();
  void addMooseObjectsBoundaryFlux7Eqn();

  bool _is_two_phase;
  std::vector<bool> _is_liquid;
  std::vector<std::string> _phase_suffix;
  std::vector<VariableName> _arhoA_name;
  std::vector<VariableName> _arhouA_name;
  std::vector<VariableName> _arhoEA_name;
  std::vector<VariableName> _alpha_name;
  std::vector<VariableName> _velocity_name;
  std::vector<VariableName> _enthalpy_name;
  std::vector<VariableName> _pressure_name;
};

#endif /* FREEBOUNDARY_H */
