/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWLINEGEOMETRY_H
#define POROUSFLOWLINEGEOMETRY_H

#include "DiracKernel.h"

class PorousFlowLineGeometry;

template<>
InputParameters validParams<PorousFlowLineGeometry>();

/**
 * Approximates a borehole by a sequence of Dirac Points
 */
class PorousFlowLineGeometry : public DiracKernel
{
public:

  /**
   * Creates a new PorousFlowLineGeometry
   * This reads the file containing the lines of the form
   * weight x y z
   * that defines the line geometry.
   * It also calculates segment-lengths between the points
   */
  PorousFlowLineGeometry(const InputParameters & parameters);

protected:
  /// line length.  This is only used if there is only one borehole point
  const Real _line_length;

  /// line direction.  This is only used if there is only one borehole point
  const RealVectorValue _line_direction;

  /**
   * File defining the geometry of the borehole.   Each row has format
   * weight x y z
   * and the list of such points defines a polyline that is the line sink
   */
  const std::string _point_file;

  /// radii of the borehole
  std::vector<Real> _rs;

  /// x points of the borehole
  std::vector<Real> _xs;

  /// y points of the borehole
  std::vector<Real> _ys;

  /// z points of borehole
  std::vector<Real> _zs;

  /// the bottom point of the borehole (where bottom_pressure is defined)
  Point _bottom_point;

  /// 0.5*(length of polyline segments between points)
  std::vector<Real> _half_seg_len;

  /// Add Dirac Points to the line sink
  virtual void addPoints();

  /// reads a space-separated line of floats from ifs and puts in myvec
  bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> & myvec);
};

#endif //POROUSFLOWLINEGEOMETRY_H
