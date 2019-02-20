#ifndef LAPIDUS_H
#define LAPIDUS_H

#include "StabilizationSettings.h"

class Lapidus;
class FlowModelSinglePhase;
class FlowModelTwoPhase;

template <>
InputParameters validParams<Lapidus>();

class Lapidus : public StabilizationSettings
{
public:
  Lapidus(const InputParameters & parameters);

  virtual void addVariables(FlowModel & fm, unsigned int subdomain_id) const;
  virtual void initMooseObjects(FlowModel & fm);
  virtual void addMooseObjects(FlowModel & fm, InputParameters & pars) const;

protected:
  void setup1Phase(FlowModelSinglePhase & fm, InputParameters & pars) const;
  void setup2Phase(FlowModelTwoPhase & fm, InputParameters & pars) const;

  /// User specified coefficient for single phase
  const Real & _cl;
  /// User specified coefficient for liquid phase
  const Real & _cl_liquid;
  /// User specified coefficient for vapor phase
  const Real & _cl_vapor;
};

#endif /* LAPIDUS_H */
