#pragma once

#include "Component.h"

/**
 * Component that connects torque of turbomachinery components
 */
class Shaft : public Component
{
public:
  Shaft(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;
  virtual VariableName getOmegaVariableName() const;

protected:
  virtual void init() override;
  virtual void check() const override;

  /// scaling factor for scalar variable omega
  const Real & _scaling_factor_omega;
  /// Name of the omega variable
  const VariableName _omega_var_name;
  /// Components connected to this shaft
  const std::vector<std::string> & _connected_components;
  /// Vector of subdomain names of the connected geometrical flow components
  std::vector<SubdomainName> _connected_subdomain_names;

public:
  static InputParameters validParams();
};
