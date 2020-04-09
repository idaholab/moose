//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PolycrystalUserObjectBase.h"

// Forward Declarations

class PolycrystalVoronoi : public PolycrystalUserObjectBase
{
public:
  static InputParameters validParams();

  PolycrystalVoronoi(const InputParameters & parameters);

  virtual void precomputeGrainStructure() override;
  virtual void getGrainsBasedOnPoint(const Point & point,
                                     std::vector<unsigned int> & grains) const override;
  virtual Real getVariableValue(unsigned int op_index, const Point & p) const override;

  virtual unsigned int getNumGrains() const override { return _grain_num; }
  virtual std::vector<Point> getGrainCenters() const { return _centerpoints; }

protected:
  /// The number of grains to create
  unsigned int _grain_num;

  const bool _columnar_3D;

  const unsigned int _rand_seed;
  const Real _int_width;

  Point _bottom_left;
  Point _top_right;
  Point _range;

  std::vector<Point> _centerpoints;

  const FileName _file_name;

private:
  Real computeDiffuseInterface(const Point & point,
                               const unsigned int & gr_index,
                               const std::vector<unsigned int> & grain_ids) const;
  Point findNormalVector(const Point & point, const Point & p1, const Point & p2) const;
  Point findCenterPoint(const Point & point, const Point & p1, const Point & p2) const;
  Real findLinePoint(const Point & point,
                     const Point & N,
                     const Point & cntr,
                     const unsigned int dim) const;
};
