#pragma once

#include "StabilizationSettings.h"

class EntropyViscosity;

template <>
InputParameters validParams<EntropyViscosity>();

class EntropyViscosity : public StabilizationSettings
{
public:
  EntropyViscosity(const InputParameters & parameters);

  virtual void addVariables(FlowModel & fm, SubdomainID subdomain_id) const;
  virtual void initMooseObjects(FlowModel & fm);
  virtual void addMooseObjects(FlowModel & fm, InputParameters & pars) const;

};
