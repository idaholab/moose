//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLineSearchObjectWrapper.h"

#include "FEProblemBase.h"

ComputeLineSearchObjectWrapper::ComputeLineSearchObjectWrapper(FEProblemBase & fe_problem)
  : _fe_problem(fe_problem)
{
}

void ComputeLineSearchObjectWrapper::linesearch(SNESLineSearch /*line_search_object*/)
{
  _fe_problem.lineSearch();
}
