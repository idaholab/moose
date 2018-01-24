//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMCutPlaneAux.h"

#include "XFEM.h"

template <>
InputParameters
validParams<XFEMCutPlaneAux>()
{
  InputParameters params = validParams<AuxKernel>();
  MooseEnum quantity("origin_x origin_y origin_z normal_x normal_y normal_z");
  params.addRequiredParam<MooseEnum>(
      "quantity", quantity, "The quantity to be extracted.  Choices: " + quantity.getRawNames());
  params.addParam<unsigned int>("plane_id", 0, "The index of the cut plane");
  return params;
}

XFEMCutPlaneAux::XFEMCutPlaneAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _quantity(Xfem::XFEM_CUTPLANE_QUANTITY(int(getParam<MooseEnum>("quantity")))),
    _plane_id(getParam<unsigned int>("plane_id"))
{
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblemBase in XFEMCutPlaneAux");
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());
  if (_xfem == NULL)
    mooseError("Problem casting to XFEM in XFEMCutPlaneAux");
  if (isNodal())
    mooseError("XFEMCutPlaneAux can only be run on an element variable");
}

Real
XFEMCutPlaneAux::computeValue()
{
  Real value = _xfem->getCutPlane(_current_elem, _quantity, _plane_id);

  return value;
}
