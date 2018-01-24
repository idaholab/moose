//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMMarkerAux.h"

#include "XFEM.h"

template <>
InputParameters
validParams<XFEMMarkerAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

XFEMMarkerAux::XFEMMarkerAux(const InputParameters & parameters) : AuxKernel(parameters)
{
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblemBase in XFEMMarkerAux");
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());
  if (_xfem == NULL)
    mooseError("Problem casting to XFEM in XFEMMarkerAux");
  if (isNodal())
    mooseError("XFEMMarkerAux can only be run on an element variable");
}

Real
XFEMMarkerAux::computeValue()
{
  bool isCTE = _xfem->isElemAtCrackTip(_current_elem);
  Real value = 0.0;
  if (isCTE)
  {
    value = 1.0;
  }

  return value;
}
