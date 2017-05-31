#ifndef FLOWJUNCTION_H
#define FLOWJUNCTION_H

#include <string>
#include "Junction.h"

class FlowJunction;

template <>
InputParameters validParams<FlowJunction>();

/**
 * Joint for flow
 */
class FlowJunction : public Junction
{
public:
  FlowJunction(const InputParameters & params);

  virtual void check() override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  std::string _lm_name;
  std::vector<Real> _K;
  Real _scaling_factor;
  std::vector<Real> _scaling_factor_bcs;
};

#endif /* FLOWJUNCTION_H */
