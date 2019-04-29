#pragma once

#include "FlowBoundary.h"

class Outlet;

template <>
InputParameters validParams<Outlet>();

/**
 * A for component with prescribed pressure
 *
 */
class Outlet : public FlowBoundary
{
public:
  Outlet(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  bool _reversible;
  bool _legacy;

  void add3EqnStaticPBC();
  void add3EqnStaticPBCLegacy();
  void add3EqnStaticPReverseBC();
  void addMooseObjects3EqnRDG();

  void add7EqnStaticPBC();
  void add7EqnStaticPBCLegacy();
  void add7EqnStaticPReverseBC();
  void addNCGStaticPBC();
  void addMooseObjects7EqnRDG();
};
