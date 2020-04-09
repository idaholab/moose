//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMVolFracAux.h"

#include "XFEM.h"

registerMooseObject("XFEMApp", XFEMVolFracAux);

InputParameters
XFEMVolFracAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Computes the volume fraction of the physical material in each partial element.");
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
  if (_xfem == nullptr)
    mooseError("Problem casting to XFEM in XFEMVolFracAux");
}

Real
XFEMVolFracAux::computeValue()
{
  return _xfem->getPhysicalVolumeFraction(_current_elem);
}
