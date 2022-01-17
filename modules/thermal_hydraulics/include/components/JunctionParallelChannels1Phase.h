#pragma once

#include "VolumeJunctionBase.h"
#include "VolumeJunction1Phase.h"

/**
 * Junction between 1-phase flow channels that are parallel
 */
class JunctionParallelChannels1Phase : public VolumeJunction1Phase
{
public:
  JunctionParallelChannels1Phase(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void init() override;
  /**
   * Builds user object for computing and storing the fluxes
   */
  virtual void buildVolumeJunctionUserObject() override;

  /// Directions at each connection
  std::vector<RealVectorValue> _directions;

public:
  static InputParameters validParams();
};
