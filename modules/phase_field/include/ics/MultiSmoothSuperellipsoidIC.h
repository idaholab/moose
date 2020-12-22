//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SmoothSuperellipsoidBaseIC.h"

/**
 * MultismoothSuperellipsoidIC creates multiple SmoothSuperellipsoid (number = numbub) that are
 * randomly
 * positioned around the domain, with a minimum spacing equal to bubspac
 */
class MultiSmoothSuperellipsoidIC : public SmoothSuperellipsoidBaseIC
{
public:
  static InputParameters validParams();

  MultiSmoothSuperellipsoidIC(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual void computeSuperellipsoidSemiaxes();
  virtual void computeSuperellipsoidCenters();
  virtual void computeSuperellipsoidExponents();

  virtual bool ellipsoidsOverlap(unsigned int i, unsigned int j);
  virtual bool checkExtremes(unsigned int i, unsigned int j);

  const unsigned int _max_num_tries;
  unsigned int _gk;

  const MooseEnum _semiaxis_variation_type;
  const bool _prevent_overlap;
  const bool _check_extremes;
  const bool _vary_axes_independently;

  Point _bottom_left;
  Point _top_right;
  Point _range;

  std::vector<unsigned int> _numbub;
  std::vector<Real> _bubspac;
  std::vector<Real> _exponent;
  std::vector<Real> _semiaxis_a;
  std::vector<Real> _semiaxis_b;
  std::vector<Real> _semiaxis_c;
  std::vector<Real> _semiaxis_a_variation;
  std::vector<Real> _semiaxis_b_variation;
  std::vector<Real> _semiaxis_c_variation;
};
