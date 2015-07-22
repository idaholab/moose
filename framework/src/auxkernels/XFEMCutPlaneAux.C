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
  MooseEnum quantity("origin_x origin_y origin_z normal_x normal_y normal_z");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "The quantity to be extracted.  Choices: "+quantity.getRawNames());
  params.addParam<unsigned int>("plane_id", 0, "The index of the cut plane");
  return params;
}

XFEMCutPlaneAux::XFEMCutPlaneAux(const InputParameters & parameters)
  :AuxKernel(parameters),
  _quantity(XFEM_CUTPLANE_QUANTITY(int(getParam<MooseEnum>("quantity")))),
  _plane_id(getParam<unsigned int>("plane_id"))
{
  FEProblem * fe_problem = dynamic_cast<FEProblem *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblem in XFEMCutPlaneAux");
  _xfem = fe_problem->get_xfem();
  if (isNodal())
    mooseError("XFEMCutPlaneAux can only be run on an element variable");
}

Real
XFEMCutPlaneAux::computeValue()
{
  Real value = _xfem->get_cut_plane(_current_elem, _quantity, _plane_id);

  return value;
}

// DEPRECATED CONSTRUCTOR
XFEMCutPlaneAux::XFEMCutPlaneAux(const std::string & deprecated_name, InputParameters parameters)
  :AuxKernel(deprecated_name, parameters),
  _quantity(XFEM_CUTPLANE_QUANTITY(int(getParam<MooseEnum>("quantity")))),
  _plane_id(getParam<unsigned int>("plane_id"))
{
  FEProblem * fe_problem = dynamic_cast<FEProblem *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblem in XFEMCutPlaneAux");
  _xfem = fe_problem->get_xfem();
  if (isNodal())
    mooseError("XFEMCutPlaneAux can only be run on an element variable");
}
