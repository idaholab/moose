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

#include "XFEMSecondCutPlaneAux.h"

#include "XFEM.h"

template<>
InputParameters validParams<XFEMSecondCutPlaneAux>()
{
  InputParameters params = validParams<AuxKernel>();
  MooseEnum quantity("origin_x origin_y origin_z normal_x normal_y normal_z");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "The quantity to be extracted.  Choices: "+quantity.getRawNames());
  return params;
}

XFEMSecondCutPlaneAux::XFEMSecondCutPlaneAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
  _quantity(XFEM_CUTPLANE_QUANTITY(int(getParam<MooseEnum>("quantity"))))
{
  FEProblem * fe_problem = dynamic_cast<FEProblem *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblem in XFEMSecondCutPlaneAux");
  _xfem = fe_problem->get_xfem();
  if (isNodal())
    mooseError("XFEMSecondCutPlaneAux can only be run on an element variable");
}

Real
XFEMSecondCutPlaneAux::computeValue()
{
  Real value = _xfem->get_cut_plane(_current_elem, _quantity, 1);

  return value;
}
