//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SIDESETSFROMPOINTSGENERATOR_H
#define SIDESETSFROMPOINTSGENERATOR_H

#include "GenerateSideSetsBase.h"

// Forward declarations
class SideSetsFromPointsGenerator;

template <>
InputParameters validParams<SideSetsFromPointsGenerator>();

/**
 * 
 */
class SideSetsFromPointsGenerator : public GenerateSideSetsBase
{
public:
  SideSetsFromPointsGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate();

protected:
  std::unique_ptr<MeshBase> & _input;

  std::vector<BoundaryName> _boundary_names;

  std::vector<Point> _points;
};

#endif // SIDESETSFROMPOINTSGENERATOR_H
