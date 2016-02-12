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

#include "XFEMVolFracAux.h"

#include "XFEM.h"

template<>
InputParameters validParams<XFEMVolFracAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

XFEMVolFracAux::XFEMVolFracAux(const InputParameters & parameters)
  :AuxKernel(parameters)
{
  if (isNodal())
    mooseError("XFEMVolFracAux must be run on an element variable");
  FEProblem * fe_problem = dynamic_cast<FEProblem *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblem in XFEMVolFracAux");
  _xfem = fe_problem->getXFEM();
}

Real
XFEMVolFracAux::computeValue()
{
  return _xfem->getPhysicalVolumeFraction(_current_elem);
}
