/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CNSFVMINMAXSLOPELIMITING_H
#define CNSFVMINMAXSLOPELIMITING_H

#include "SlopeLimitingBase.h"

// Forward Declarations
class CNSFVMinmaxSlopeLimiting;

template <>
InputParameters validParams<CNSFVMinmaxSlopeLimiting>();

/**
 * A user object that performs the min-max slope limiting to get the limited slopes of cell average
 * variables
 */
class CNSFVMinmaxSlopeLimiting : public SlopeLimitingBase
{
public:
  CNSFVMinmaxSlopeLimiting(const InputParameters & parameters);

  /// compute the limited slope of the cell
  virtual std::vector<RealGradient> limitElementSlope() const;
};

#endif
