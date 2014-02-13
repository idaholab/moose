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

// LibMesh includes
#include "libmesh/parameters.h"

// System includes
#include <string>

// Forward Declarations
class ScalarConstantIC;

template<>
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
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   */
  ScalarConstantIC(const std::string & name, InputParameters parameters);

  /**
   * The value of the variable at a point.
   */
  virtual Real value();

protected:
  Real _value;
};

#endif //CONSTANTIC_H
