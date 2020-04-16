#pragma once

#include "FlowJunction.h"

/**
 * Junction connecting one flow channel to one other flow channel
 *
 * The assumptions made by this component are as follows:
 * @li The connected channels are parallel.
 * @li The connected channels have the same flow area at the junction.
 */
class JunctionOneToOne : public FlowJunction
{
public:
  JunctionOneToOne(const InputParameters & params);

  virtual void check() const override;
  virtual void addMooseObjects() override;

protected:
  virtual void setupMesh() override;
  virtual void addMooseObjects1Phase() const;
  virtual void addMooseObjects2Phase() const;
  virtual void addJunctionUserObject2Phase() const;

public:
  static InputParameters validParams();
};
