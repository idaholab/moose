#pragma once
#include "QuadInterWrapperBaseIC.h"

/**
 * Sets the wetted perimeter of the quadrilater inter-wrapper flow channel
 */
class QuadInterWrapperWettedPerimIC : public QuadInterWrapperBaseIC
{
public:
  QuadInterWrapperWettedPerimIC(const InputParameters & params);
  Real value(const Point & p) override;

public:
  static InputParameters validParams();
};
