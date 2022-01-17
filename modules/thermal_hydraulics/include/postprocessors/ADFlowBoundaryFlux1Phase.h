#pragma once

#include "SideIntegralPostprocessor.h"

class ADBoundaryFluxBase;

/**
 * Retrieves an entry of a flux vector for a 1-phase boundary
 */
class ADFlowBoundaryFlux1Phase : public SideIntegralPostprocessor
{
public:
  ADFlowBoundaryFlux1Phase(const InputParameters & parameters);

  virtual Real computeQpIntegral() override;

protected:
  /// Number of components in the solution vector used to compute the flux
  const unsigned int _n_components;
  /// Variables to pass to boundary flux user object, in the correct order
  std::vector<const ADVariableValue *> _U;
  /// Boundary component name
  const std::string & _boundary_name;
  /// Boundary user object name
  const std::string _boundary_uo_name;
  /// Boundary user object
  const ADBoundaryFluxBase & _boundary_uo;
  /// Index within flux vector to query
  const unsigned int _equation_index;

public:
  static InputParameters validParams();
};
