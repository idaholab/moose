#pragma once

#include "StabilizationSettings.h"

class Rusanov;
class FlowModelSinglePhase;
class FlowModelTwoPhase;

template <>
InputParameters validParams<Rusanov>();

class Rusanov : public StabilizationSettings
{
public:
  Rusanov(const InputParameters & parameters);

  virtual void addVariables(FlowModel & fm, unsigned int subdomain_id) const;
  virtual void initMooseObjects(FlowModel & fm);
  virtual void addMooseObjects(FlowModel & fm, InputParameters & pars) const;

protected:
  void setup1Phase(FlowModelSinglePhase & fm, InputParameters & pars) const;
  void setup2Phase(FlowModelTwoPhase & fm, InputParameters & pars) const;
};
