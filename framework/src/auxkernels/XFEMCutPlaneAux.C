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

#include "XFEMCutPlaneAux.h"

#include "XFEM.h"

template<>
InputParameters validParams<XFEMCutPlaneAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("index", "The index of the local node for which the distance is to be output (0-n).");
  return params;
}

XFEMCutPlaneAux::XFEMCutPlaneAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters)
{
  FEProblem * fe_problem = dynamic_cast<FEProblem *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblem in XFEMCutPlaneAux");
  _xfem = fe_problem->get_xfem();
  if (isNodal())
    mooseError("XFEMCutPlaneAux can only be run on an element variable");
  _index=getParam<unsigned int>("index");
}

Real
XFEMCutPlaneAux::computeValue()
{
  Real value = _xfem->get_cut_plane(_current_elem, _index);

  return value;
}
