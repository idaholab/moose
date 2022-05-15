#pragma once

#include "TriInterWrapperBaseIC.h"

class TriInterWrapperMesh;

/**
 * This class calculates the area of the triangular, edge, and corner subchannels for hexagonal fuel
 * assemblies
 */
class TriInterWrapperFlowAreaIC : public TriInterWrapperBaseIC
{
public:
  TriInterWrapperFlowAreaIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
