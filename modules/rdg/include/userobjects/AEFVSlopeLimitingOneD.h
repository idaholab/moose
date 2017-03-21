/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef AEFVSLOPELIMITINGONED_H
#define AEFVSLOPELIMITINGONED_H

#include "SlopeLimitingBase.h"

// Forward Declarations
class AEFVSlopeLimitingOneD;

template <>
InputParameters validParams<AEFVSlopeLimitingOneD>();

/**
 * One-dimensional slope limiting to get
 * the limited slope of cell average variable
 * for the advection equation
 * using a cell-centered finite volume method
 */
class AEFVSlopeLimitingOneD : public SlopeLimitingBase
{
public:
  AEFVSlopeLimitingOneD(const InputParameters & parameters);

  /// compute the limited slope of the cell
  virtual std::vector<RealGradient> limitElementSlope() const override;

protected:
  /// the input variable
  MooseVariable * _u;

  /// One-D slope limiting scheme
  MooseEnum _scheme;
};

#endif
