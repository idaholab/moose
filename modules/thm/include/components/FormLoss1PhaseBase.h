#pragma once

#include "Component.h"

class FormLoss1PhaseBase;

template <>
InputParameters validParams<FormLoss1PhaseBase>();

/**
 * Base class for prescribing a form loss over a 1-phase flow channel
 */
class FormLoss1PhaseBase : public Component
{
public:
  FormLoss1PhaseBase(const InputParameters & params);

  virtual void init() override;
  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  /// Subdomains corresponding to the connected flow channel
  std::vector<SubdomainName> _flow_channel_subdomains;
  /// Name of the flow channel component
  const std::string & _flow_channel_name;
};
