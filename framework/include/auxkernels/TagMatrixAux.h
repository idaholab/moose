//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TAGMATRIXAUX_H
#define TAGMATRIXAUX_H

#include "TagVectorAux.h"

// Forward Declarations
class TagMatrixAux;

template <>
InputParameters validParams<TagMatrixAux>();

/**
 * Constant auxiliary value
 */
class TagMatrixAux : public TagVectorAux
{
public:
  TagMatrixAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  SparseMatrix<Number> * _tag_matrix;
};

#endif // TAGMATRIXAUX_H
