//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementDeletionGeneratorBase.h"
#include "FunctionParserUtils.h"

namespace libmesh
{
class Point;
}

/**
 * Deletes elements based on the evaluation of a parsed expression, involving the coordinates of
 * their vertex average and their volume
 */
class ParsedElementDeletionGenerator : public ElementDeletionGeneratorBase,
                                       public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  ParsedElementDeletionGenerator(const InputParameters & parameters);

protected:
  virtual bool shouldDelete(const Elem * elem) override;

private:
  /// the function
  SymFunctionPtr _function;
};
