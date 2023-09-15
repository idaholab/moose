//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolycrystalVoronoiVoidIC.h"

/**
 * PolycrystalVoronoiTJVoidIC initializes either grain or void values for a
 * voronoi tesselation with voids distributed at/along the triple junctions.
 */
class PolycrystalVoronoiTJVoidIC : public PolycrystalVoronoiVoidIC
{
public:
  static InputParameters validParams();

  PolycrystalVoronoiTJVoidIC(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  bool _pbc = true; // default
  const MooseEnum _periodic_graincenters_option;

  const unsigned int _dim;
  std::vector<Point> _pbc_centerpoints;

  virtual void computeCircleCenters() override;

  virtual Real value(const Point & p) override;
  virtual RealGradient gradient(const Point & p) override;
};
