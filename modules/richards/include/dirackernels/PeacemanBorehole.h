/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef PEACEMANBOREHOLE_H
#define PEACEMANBOREHOLE_H

// Moose Includes
#include "DiracKernel.h"
#include "Function.h"
#include "RichardsSumQuantity.h"

class PeacemanBorehole;

template <>
InputParameters validParams<PeacemanBorehole>();

/**
 * Approximates a borehole by a sequence of Dirac Points
 */
class PeacemanBorehole : public DiracKernel
{
public:
  /**
   * Creates a new PeacemanBorehole
   * This reads the file containing the lines of the form
   * radius x y z
   * that defines the borehole geometry.
   * It also calculates segment-lengths and rotation matrices
   * needed for computing the borehole well constant
   */
  PeacemanBorehole(const InputParameters & parameters);

private:
  /// borehole constant
  const Real _re_constant;

  /// well constant
  const Real _well_constant;

  /// borehole length.  Note this is only used if there is only one borehole point
  const Real _borehole_length;

  /// borehole direction.  Note this is only used if there is only one borehole point
  const RealVectorValue _borehole_direction;

  /**
   * File defining the geometry of the borehole.   Each row has format
   * radius x y z
   * and the list of such points defines a polyline that is the borehole
   */
  const std::string _point_file; // private

protected:
  /**
   * If positive then the borehole acts as a sink (producion well) for porepressure > borehole
   * pressure, and does nothing otherwise
   * If negative then the borehole acts as a source (injection well) for porepressure < borehole
   * pressure, and does nothing otherwise
   * The flow rate to/from the borehole is multiplied by |character|, so usually character = +/- 1
   */
  Function & _character;

  /// bottomhole pressure of borehole
  const Real _p_bot;

  /// unit weight of fluid in borehole (for calculating bottomhole pressure at each Dirac Point)
  const RealVectorValue _unit_weight;

  /**
   * This is used to hold the total fluid flowing into the borehole
   * Hence, it is positive for production wells where fluid is flowing
   * from porespace into the borehole and removed from the model
   */
  RichardsSumQuantity & _total_outflow_mass;

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

  /// rotation matrix used in well_constant calculation
  std::vector<RealTensorValue> _rot_matrix;

  /// Add Dirac Points to the borehole
  virtual void addPoints();

  /// reads a space-separated line of floats from ifs and puts in myvec
  bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> & myvec);

  /**
   * Calculates Peaceman's form of the borehole well constant
   * Z Chen, Y Zhang, Well flow models for various numerical methods, Int J Num Analysis and
   * Modeling, 3 (2008) 375-388
   */
  Real wellConstant(const RealTensorValue & perm,
                    const RealTensorValue & rot,
                    const Real & half_len,
                    const Elem * ele,
                    const Real & rad);
};

#endif // PEACEMANBOREHOLE_H
