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

#ifndef RANDOMIC_H
#define RANDOMIC_H

#include "InitialCondition.h"

// System includes
#include <string>

// Forward Declarations
class InputParameters;
class RandomIC;
namespace libMesh
{
class Point;
}

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<RandomIC>();

/**
 * RandomIC just returns a Random value.
 */
class RandomIC : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  RandomIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  Real _min;
  Real _max;
  Real _range;
};

#endif // RANDOMIC_H
