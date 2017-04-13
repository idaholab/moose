/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMVolFracAux.h"

#include "XFEM.h"

template <>
InputParameters
validParams<XFEMVolFracAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

XFEMVolFracAux::XFEMVolFracAux(const InputParameters & parameters) : AuxKernel(parameters)
{
  if (isNodal())
    mooseError("XFEMVolFracAux must be run on an element variable");
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblemBase in XFEMVolFracAux");
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());
  if (_xfem == NULL)
    mooseError("Problem casting to XFEM in XFEMVolFracAux");
}

Real
XFEMVolFracAux::computeValue()
{
  return _xfem->getPhysicalVolumeFraction(_current_elem);
}
