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

#ifndef ELEMENTH1ERROR_H
#define ELEMENTH1ERROR_H

#include "ElementW1pError.h"

// Forward Declarations
class ElementH1Error;

template <>
InputParameters validParams<ElementH1Error>();

/**
 * This postprocessor will print out the H^1-norm of the difference
 * between the computed solution and the passed function, where the
 * norm is defined as:
 *
 * ||u-f||_{H^1} = sqrt( \int ( |u-f|^2 + |grad u - grad f|^2 ) dx )
 */
class ElementH1Error : public ElementW1pError
{
public:
  ElementH1Error(const InputParameters & parameters);
};

#endif // ELEMENTH1ERROR_H
