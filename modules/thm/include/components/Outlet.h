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

protected:
  virtual void check() const override;
};
