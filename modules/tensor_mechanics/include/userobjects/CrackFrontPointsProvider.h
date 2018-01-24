//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CRACKFRONTPOINTSPROVIDER_H
#define CRACKFRONTPOINTSPROVIDER_H

#include "GeneralUserObject.h"

class CrackFrontPointsProvider;

template <>
InputParameters validParams<CrackFrontPointsProvider>();

/**
 * Base class for crack front points provider
 */
class CrackFrontPointsProvider : public GeneralUserObject
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
