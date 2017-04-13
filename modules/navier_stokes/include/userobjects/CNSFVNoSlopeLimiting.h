/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVNOSLOPELIMITING_H
#define CNSFVNOSLOPELIMITING_H

#include "SlopeLimitingBase.h"

// Forward Declarations
class CNSFVNoSlopeLimiting;

template <>
InputParameters validParams<CNSFVNoSlopeLimiting>();

/**
 * A user object that does no slope limiting in multi-dimensions
 */
class CNSFVNoSlopeLimiting : public SlopeLimitingBase
{
public:
  CNSFVNoSlopeLimiting(const InputParameters & parameters);

  /// compute the limited slope of the cell
  virtual std::vector<RealGradient> limitElementSlope() const;
};

#endif
