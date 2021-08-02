#pragma once

#include "VolumeJunction1Phase.h"

/**
 * Pump between 1-phase flow channels that has a non-zero volume
 */
class Pump1Phase : public VolumeJunction1Phase
{
public:
  Pump1Phase(const InputParameters & params);

protected:
  virtual void buildVolumeJunctionUserObject() override;

  /// Pump head [m]
  const Real & _head;

public:
  static InputParameters validParams();
};
