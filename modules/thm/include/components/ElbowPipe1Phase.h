#pragma once

#include "FlowChannel1Phase.h"

class ElbowPipe1Phase;

template <>
InputParameters validParams<ElbowPipe1Phase>();

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
};
