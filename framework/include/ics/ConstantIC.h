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
namespace libMesh { class Point; }

template<>
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
   * @param name The name given to the initial condition in the input file.
   * @param parameters The parameters object holding data for the class to use.
   * @param var_name The variable this InitialCondtion is supposed to provide values for.
   */
  ConstantIC(const std::string & name,
             InputParameters parameters);

  /**
   * The value of the variable at a point.
   *
   * This must be overridden by derived classes.
   */
  virtual Real value(const Point & p);

protected:
  Real _value;
};

#endif //CONSTANTIC_H
