#pragma once

#include "FlowBoundary.h"

class Outlet1Phase;

template <>
InputParameters validParams<Outlet1Phase>();

/**
 * Boundary condition with prescribed pressure for 1-phase flow channels
 */
class Outlet1Phase : public FlowBoundary
{
public:
  Outlet1Phase(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  bool _reversible;
  bool _legacy;

  void add3EqnStaticPBC();
  void add3EqnStaticPBCLegacy();
  void add3EqnStaticPReverseBC();
  void addMooseObjects3EqnRDG();
};
