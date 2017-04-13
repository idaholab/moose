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

#ifndef POSTPROCESSORAUX_H
#define POSTPROCESSORAUX_H

#include "AuxKernel.h"

// Forward Declarations
class PostprocessorAux;

template <>
InputParameters validParams<PostprocessorAux>();

/**
 * Testing object that just utilizes the value of a Postprocessor for the value of the Aux Variable
 */
class PostprocessorAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  PostprocessorAux(const InputParameters & parameters);

  virtual ~PostprocessorAux();

protected:
  virtual Real computeValue();

  const PostprocessorValue & _pp_val;
};

#endif // POSTPROCESSORAUX_H
