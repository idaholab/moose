//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"
#include "PolycrystalICTools.h"

// Forward Declarationsc
class GrainTrackerInterface;
class PolycrystalVoronoi;

/**
 * PolycrystalVoronoiCoupledVoidIC initializes either grain or void values for a
 * voronoi tesselation with voids distributed at specified x, y, z positions
 * inherited from MultiSmoothCircleIC
 */
class PolycrystalVoronoiCoupledVoidIC : public InitialCondition
{
public:
  static InputParameters validParams();

  PolycrystalVoronoiCoupledVoidIC(const InputParameters & parameters);

  virtual void initialSetup() override;

  static InputParameters actionParameters();

protected:
  const unsigned int _op_num;
  const unsigned int _op_index;

  const bool _columnar_3D;

  const PolycrystalVoronoi & _poly_ic_uo;

  const FileName _file_name;

  virtual Real value(const Point & p) override;

  unsigned int _grain_num;
  std::vector<Point> _centerpoints;

  const VariableValue & _var_val;
  Real _invalue;
  Real _outvalue;
};
