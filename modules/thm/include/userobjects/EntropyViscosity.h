#pragma once

#include "StabilizationSettings.h"

class EntropyViscosity;

template <>
InputParameters validParams<EntropyViscosity>();

class EntropyViscosity : public StabilizationSettings
{
public:
  EntropyViscosity(const InputParameters & parameters);

  virtual void addVariables(FlowModel & fm, const SubdomainName & subdomain_name) const;
  virtual void initMooseObjects(FlowModel & fm);
  virtual void addMooseObjects(FlowModel & fm, InputParameters & pars) const;
};
