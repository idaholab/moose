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

#ifndef POSTPROCESSORNEUMANNBC_H
#define POSTPROCESSORNEUMANNBC_H

#include "IntegratedBC.h"

class PostprocessorNeumannBC;

template <>
InputParameters validParams<PostprocessorNeumannBC>();

/**
 * Implements a constant Neumann BC where grad(u) is a equal to a postprocessor on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class PostprocessorNeumannBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  PostprocessorNeumannBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Value of grad(u) on the boundary.
  const PostprocessorValue & _value;
};

#endif // POSTPROCESSORNEUMANNBC_H
