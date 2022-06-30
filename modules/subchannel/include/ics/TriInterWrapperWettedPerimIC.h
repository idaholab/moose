#pragma once

#include "TriInterWrapperBaseIC.h"

/**
 * Sets the wetted perimeter of the quadrilater inter-wrapper flow channel
 */
class TriInterWrapperWettedPerimIC : public TriInterWrapperBaseIC
{
public:
  TriInterWrapperWettedPerimIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
