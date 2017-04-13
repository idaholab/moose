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

#ifndef POSTPROCESSORDIRICHLETBC_H
#define POSTPROCESSORDIRICHLETBC_H

#include "NodalBC.h"

class PostprocessorDirichletBC;

template <>
InputParameters validParams<PostprocessorDirichletBC>();

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the value at the node to the value of a Postprocessor
 */
class PostprocessorDirichletBC : public NodalBC
{
public:
  PostprocessorDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// The value for this BC
  const PostprocessorValue & _postprocessor_value;
};

#endif /* POSTPROCESSORDIRICHLETBC_H */
