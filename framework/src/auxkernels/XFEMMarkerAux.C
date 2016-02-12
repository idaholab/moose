/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "XFEMMarkerAux.h"

#include "XFEM.h"

template<>
InputParameters validParams<XFEMMarkerAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

XFEMMarkerAux::XFEMMarkerAux(const InputParameters & parameters)
  :AuxKernel(parameters)
{
  FEProblem * fe_problem = dynamic_cast<FEProblem *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblem in XFEMMarkerAux");
  _xfem = fe_problem->getXFEM();
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
