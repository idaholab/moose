//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiracKernel.h"
#include "ReporterInterface.h"

/**
 * Approximates a borehole by a sequence of Dirac Points
 */
class PorousFlowLineGeometry : public DiracKernel, public ReporterInterface
{
public:
  /**
   * Creates a new PorousFlowLineGeometry
   * This reads the file containing the lines of the form
   * weight x y z
   * that defines the line geometry.
   * It also calculates segment-lengths between the points
   */
  static InputParameters validParams();

  PorousFlowLineGeometry(const InputParameters & parameters);

  void virtual initialSetup() override;

protected:
  /// Line length.  This is only used if there is only one borehole point
  const Real _line_length;

  /// Line direction.  This is only used if there is only one borehole point
  const RealVectorValue _line_direction;

  /**
   * File defining the geometry of the borehole.   Each row has format
   * weight x y z
   * and the list of such points defines a polyline that is the line sink
   */
  const std::string _point_file;

  /// Radii of the borehole
  std::vector<Real> _rs;

  /// x points of the borehole
  std::vector<Real> _xs;

  /// y points of the borehole
  std::vector<Real> _ys;

  /// z points of borehole
  std::vector<Real> _zs;

  /// The bottom point of the borehole (where bottom_pressure is defined)
  Point _bottom_point;

  /// 0.5*(length of polyline segments between points)
  std::vector<Real> _half_seg_len;

  /// Add Dirac Points to the line sink
  virtual void addPoints() override;

  /// regenerate points in each cell if using line_base
  virtual void meshChanged() override;

  /// Reads a space-separated line of floats from ifs and puts in myvec
  bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> & myvec);

  ///@{
  /// reporter input alternative (to the point file and line_base data)
  const std::vector<Real> * const _x_coord;
  const std::vector<Real> * const _y_coord;
  const std::vector<Real> * const _z_coord;
  const std::vector<Real> * const _weight;
  const bool _usingReporter;
  ///@}
private:
  void calcLineLengths();
  void regenPoints();

  /// alternative (to the point file data) line weight and start point.
  std::vector<Real> _line_base;
};
