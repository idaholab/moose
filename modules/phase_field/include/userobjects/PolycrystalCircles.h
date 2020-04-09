//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <array>
#include "PolycrystalUserObjectBase.h"
#include "DelimitedFileReader.h"

// Forward Declarations

/**
 * PolycrystalCircles creates a polycrystal made up of circles.
 * The locations and radii of the circles are given either
 * through a user input or by reading a .txt file.
 * The file is expected to have a one-line header labeling the
 * colums 'x y z r'.
 **/

class PolycrystalCircles : public PolycrystalUserObjectBase
{
public:
  static InputParameters validParams();

  PolycrystalCircles(const InputParameters & parameters);

  // Required functions from PolycrystalUserObjectBase
  virtual void precomputeGrainStructure() override;
  virtual void getGrainsBasedOnPoint(const Point & point,
                                     std::vector<unsigned int> & grains) const override;
  virtual Real getVariableValue(unsigned int op_index, const Point & p) const override;
  virtual unsigned int getNumGrains() const override { return _grain_num; }

protected:
  enum COLS
  {
    X,
    Y,
    Z,
    R
  }; // Names of columns in text file.
  /// Whether to use columns or spheres in 3D geometries
  const bool _columnar_3D;

  /// Interfacial width
  const Real _int_width;

  /// Number of crystal grains to create
  unsigned int _grain_num;

  /// x,y,z coordinates of circle centers
  std::vector<Point> _centerpoints;

  /// Radius for each circular grain created
  std::vector<Real> _radii;

  Real computeDiffuseInterface(const Point & p, const unsigned int & i) const;
};
