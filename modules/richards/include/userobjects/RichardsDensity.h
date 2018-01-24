//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
