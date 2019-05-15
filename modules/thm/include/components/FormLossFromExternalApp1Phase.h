#pragma once

#include "Component.h"

class FormLossFromExternalApp1Phase;

template <>
InputParameters validParams<FormLossFromExternalApp1Phase>();

/**
 * A component for prescribing a form loss computed by an external application
 */
class FormLossFromExternalApp1Phase : public Component
{
public:
  FormLossFromExternalApp1Phase(const InputParameters & params);

  virtual void init() override;
  virtual void check() const override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  /// block IDs corresponding to the connected flow channel
  std::vector<unsigned int> _block_ids_flow_channel;
  /// Subdomains corresponding to the connected flow channel
  std::vector<SubdomainName> _flow_channel_subdomains;
  /// Name of the flow channel component
  VariableName _K_prime_var_name;
  /// Name of the flow channel component
  const std::string & _flow_channel_name;
};
