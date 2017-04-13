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

#ifndef SCALARCONSTANTIC_H
#define SCALARCONSTANTIC_H

#include "ScalarInitialCondition.h"

// Forward Declarations
class ScalarConstantIC;

template <>
InputParameters validParams<ScalarConstantIC>();

/**
 * ScalarConstantIC just returns a constant value.
 */
class ScalarConstantIC : public ScalarInitialCondition
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   */
  ScalarConstantIC(const InputParameters & parameters);

  virtual Real value() override;

protected:
  Real _value;
};

#endif // CONSTANTIC_H
