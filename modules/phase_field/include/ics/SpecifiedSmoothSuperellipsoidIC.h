/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SPECIFIEDSMOOTHSUPERELLIPSOIDIC_H
#define SPECIFIEDSMOOTHSUPERELLIPSOIDIC_H

#include "SmoothSuperellipsoidBaseIC.h"

// Forward Declarations
class SpecifiedSmoothSuperellipsoidIC;

template <>
InputParameters validParams<SpecifiedSmoothSuperellipsoidIC>();

/**
 * SpecifiedSmoothSuperellipsoidIC creates multiple SmoothSuperellipsoids (number = size of
 * x_positions) that are positioned in the
 * set locations with the set semiaxes a, b, c and exponents n
 */
class SpecifiedSmoothSuperellipsoidIC : public SmoothSuperellipsoidBaseIC
{
public:
  SpecifiedSmoothSuperellipsoidIC(const InputParameters & parameters);

protected:
  virtual void computeSuperellipsoidCenters();
  virtual void computeSuperellipsoidSemiaxes();
  virtual void computeSuperellipsoidExponents();

  std::vector<Real> _x_positions;
  std::vector<Real> _y_positions;
  std::vector<Real> _z_positions;
  std::vector<Real> _input_as;
  std::vector<Real> _input_bs;
  std::vector<Real> _input_cs;
  std::vector<Real> _input_ns;
};

#endif // SPECIFIEDSMOOTHSUPERELLIPSOIDIC_H
