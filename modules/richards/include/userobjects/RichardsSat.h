/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSSAT_H
#define RICHARDSSAT_H

#include "GeneralUserObject.h"

class RichardsSat;

template <>
InputParameters validParams<RichardsSat>();

/**
 * Saturation of a phase as a function of
 * effective saturation of that phase,
 * and its derivatives wrt effective saturation
 */
class RichardsSat : public GeneralUserObject
{
public:
  RichardsSat(const InputParameters & parameters);

  void initialize();
  void execute();
  void finalize();

  /**
   * saturation as a function of effective saturation
   * @param seff effective saturation
   */
  Real sat(Real seff) const;

  /// derivative of saturation wrt effective saturation
  Real dsat(Real /*seff*/) const;

protected:
  /// residual saturation for this phase
  Real _s_res;

  /// sum of the residual saturations for every phase
  Real _sum_s_res;
};

#endif // RICHARDSSAT_H
