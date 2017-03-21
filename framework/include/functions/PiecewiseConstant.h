/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PIECEWISECONSTANT_H
#define PIECEWISECONSTANT_H

#include "Piecewise.h"

// Forward declarations
class PiecewiseConstant;

template <>
InputParameters validParams<PiecewiseConstant>();

/**
 * Function which provides a piecewise continuous constant interpolation
 * of a provided (x,y) point data set.
 */
class PiecewiseConstant : public Piecewise
{
public:
  PiecewiseConstant(const InputParameters & parameters);

  /**
   * Get the value of the function (based on time only)
   * \param t The time
   * \param pt The point in space (x,y,z) (unused)
   * \return The value of the function at the specified time
   */
  virtual Real value(Real t, const Point & pt) override;

  /**
   * Get the time derivative of the function (based on time only)
   * \param t The time
   * \param pt The point in space (x,y,z) (unused)
   * \return The time derivative of the function at the specified time
   */
  virtual Real timeDerivative(Real t, const Point & pt) override;

  virtual Real integral() override;

  virtual Real average() override;

private:
  enum DirectionEnum
  {
    LEFT = 0,
    RIGHT,
    UNDEFINED
  };
  DirectionEnum getDirection(const std::string & direction);

  const DirectionEnum _direction;
};

#endif
