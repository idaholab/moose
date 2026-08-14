//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeLineSearchObjectWrapper.h"

#include "FEProblemBase.h"
#include "LineSearch.h"

ComputeLineSearchObjectWrapper::ComputeLineSearchObjectWrapper(FEProblemBase & fe_problem,
                                                                unsigned int nl_sys_num)
  : _fe_problem(fe_problem), _nl_sys_num(nl_sys_num)
{
}

void ComputeLineSearchObjectWrapper::linesearch(SNESLineSearch /*line_search_object*/)
{
  _fe_problem.getNonlinearSystemBase(_nl_sys_num).getLineSearch()->lineSearch();
}
