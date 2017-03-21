/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
