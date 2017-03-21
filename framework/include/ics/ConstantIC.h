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

#ifndef CONSTANTIC_H
#define CONSTANTIC_H

#include "InitialCondition.h"
#include "InputParameters.h"

// System includes
#include <string>

// Forward Declarations
class ConstantIC;
namespace libMesh
{
class Point;
}

template <>
InputParameters validParams<ConstantIC>();

/**
 * ConstantIC just returns a constant value.
 */
class ConstantIC : public InitialCondition
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  ConstantIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  Real _value;
};

#endif // CONSTANTIC_H
