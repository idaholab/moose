//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
