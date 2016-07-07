/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MULTISMOOTHSUPERELLIPSOIDIC_H
#define MULTISMOOTHSUPERELLIPSOIDIC_H

#include "SmoothSuperellipsoidBaseIC.h"

// Forward Declarations
class MultiSmoothSuperellipsoidIC;

template<>
InputParameters validParams<MultiSmoothSuperellipsoidIC>();

/**
 * MultismoothSuperellipsoidIC creates multiple SmoothSuperellipsoid (number = numbub) that are randomly
 * positioned around the domain, with a minimum spacing equal to bubspac
 **/
class MultiSmoothSuperellipsoidIC : public SmoothSuperellipsoidBaseIC
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  MultiSmoothSuperellipsoidIC(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual void computeSuperellipsoidSemiaxes();
  virtual void computeSuperellipsoidCenters();
  virtual void computeSuperellipsoidExponents();
  virtual bool overlapCheck(const Point & newcenter, Real nc_as, Real nc_bs, Real nc_cs, Real nc_ns);

  unsigned int _max_num_tries;
  Real _gk;

  MooseEnum _semiaxis_variation_type;
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

#endif //MULTISMOOTHSUPERELLIPSOIDIC_H
