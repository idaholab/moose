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

  unsigned int _numbub;
  Real _bubspac;

  unsigned int _numtries;
  Real _exponent;
  Real _semiaxis_a;
  Real _semiaxis_b;
  Real _semiaxis_c;
  Real _semiaxis_a_variation;
  Real _semiaxis_b_variation;
  Real _semiaxis_c_variation;
  MooseEnum _semiaxis_variation_type;

  Point _bottom_left;
  Point _top_right;
  Point _range;

};

#endif //MULTISMOOTHSUPERELLIPSOIDIC_H
