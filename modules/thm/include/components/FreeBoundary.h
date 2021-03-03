#pragma once

#include "FlowConnection.h"

/**
 * Adds the boundary terms resulting from an integration by parts of the
 * advection terms, using no external boundary data.
 *
 * Deprecated
 */
class FreeBoundary : public FlowConnection
{
public:
  FreeBoundary(const InputParameters & parameters);

public:
  static InputParameters validParams();
};
