#pragma once

#include "FlowJunction.h"

class SimpleJunction;

template <>
InputParameters validParams<SimpleJunction>();

/**
 * Joint for flow
 */
class SimpleJunction : public FlowJunction
{
public:
  SimpleJunction(const InputParameters & params);

  virtual void check() const override;
  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  void add1Phase();
  void add2Phase();

  std::string _lm_name;
  Real _scaling_factor;
};
