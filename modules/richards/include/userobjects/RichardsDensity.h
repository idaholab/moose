/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSDENSITY_H
#define RICHARDSDENSITY_H

#include "GeneralUserObject.h"

class RichardsDensity;

template <>
InputParameters validParams<RichardsDensity>();

/**
 * Base class for fluid density as a function of porepressure
 * The functions density, ddensity and d2density must be
 * over-ridden in derived classes to provide actual values
 */
class RichardsDensity : public GeneralUserObject
{
public:
  RichardsDensity(const InputParameters & parameters);

  void initialize();
  void execute();
  void finalize();

  /**
   * fluid density as a function of porepressure
   * This must be over-ridden in derived classes to provide an actual value
   * @param p porepressure
   */
  virtual Real density(Real p) const = 0;

  /**
   * derivative of fluid density wrt porepressure
   * This must be over-ridden in derived classes to provide an actual value
   * @param p porepressure
   */
  virtual Real ddensity(Real p) const = 0;

  /**
   * second derivative of fluid density wrt porepressure
   * This must be over-ridden in derived classes to provide an actual value
   * @param p porepressure
   */
  virtual Real d2density(Real p) const = 0;
};

#endif // RICHARDSDENSITY_H
