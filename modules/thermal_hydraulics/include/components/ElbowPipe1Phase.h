#pragma once

#include "FlowChannel1Phase.h"

/**
 * Bent pipe for 1-phase flow
 */
class ElbowPipe1Phase : public FlowChannel1Phase
{
public:
  ElbowPipe1Phase(const InputParameters & params);

protected:
  virtual void buildMeshNodes() override;

  /// Radius of the pipe [m]
  Real _radius;
  /// Start angle [degrees]
  Real _start_angle;
  /// End angle [degrees]
  Real _end_angle;
  /// central angle
  Real _central_angle;

public:
  static InputParameters validParams();
};
