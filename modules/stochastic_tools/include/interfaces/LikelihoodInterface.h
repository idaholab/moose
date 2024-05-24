//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "FEProblemBase.h"
#include "LikelihoodFunctionBase.h"
#include "LikelihoodFunctionBaseVector.h"
enum class LikelihoodFunctionTypes
{
  SCALAR,
  VECTOR
};
class LikelihoodInterface
{
public:
  static InputParameters validParams();

  LikelihoodInterface(const InputParameters & parameters);
  LikelihoodFunctionTypes queryLikelihoodFunctionType(const UserObjectName & name);

protected:
  /// Lookup a LikelihoodFunction object by name and return pointer
  LikelihoodFunctionBase * getLikelihoodFunctionByName(const UserObjectName & name) const;
  LikelihoodFunctionBaseVector *
  getLikelihoodVectorFunctionByName(const UserObjectName & name) const;

private:
  /// Reference to FEProblemBase instance
  FEProblemBase & _likelihood_feproblem;
};
