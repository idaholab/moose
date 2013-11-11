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
#include "LinearInterpolation.h"

class PiecewiseConstant : public Piecewise
{
public:
  PiecewiseConstant(const std::string & name, InputParameters parameters);
  virtual ~PiecewiseConstant();

  /**
   * This function will return a value based on the first input argument only.
   */
  virtual Real value(Real t, const Point & pt);
  /**
   * This function will return a value based on the first input argument only.
   */
  virtual Real timeDerivative(Real t, const Point & pt);

  virtual Real integral();

  virtual Real average();

private:

  enum DirectionEnum {
    LEFT = 0,
    RIGHT,
    UNDEFINED
  };
  DirectionEnum getDirection( const std::string & direction );

  const DirectionEnum _direction;

};

template<>
InputParameters validParams<PiecewiseConstant>();

#endif
