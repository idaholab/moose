#pragma once

#include "FlowJunction.h"

/**
 * Junction connecting one flow channel to one other flow channel for 1-phase flow
 *
 * The assumptions made by this component are as follows:
 * @li The connected channels are parallel.
 * @li The connected channels have the same flow area at the junction.
 */
class JunctionOneToOne1Phase : public FlowJunction
{
public:
  JunctionOneToOne1Phase(const InputParameters & params);

  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  virtual void setupMesh() override;

public:
  static InputParameters validParams();
};
