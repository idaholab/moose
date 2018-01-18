/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRACKFRONTPOINTSPROVIDER_H
#define CRACKFRONTPOINTSPROVIDER_H

#include "ElementUserObject.h"

class CrackFrontPointsProvider;

template <>
InputParameters validParams<CrackFrontPointsProvider>();

/**
 * Base class for crack front points provider
 */
class CrackFrontPointsProvider : public ElementUserObject
{
public:
  CrackFrontPointsProvider(const InputParameters & parameters);

  /** get a set of points along a crack front from a XFEM GeometricCutUserObject
   * @return A vector which contains all crack front points
   */
  virtual const std::vector<Point>
  getCrackFrontPoints(unsigned int /*num_crack_front_points*/) const = 0;
};

#endif /* CRACKFRONTPOINTSPROVIDER_H */
