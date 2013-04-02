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

#include "NodalNormalComponentAux.h"

template<>
InputParameters validParams<NodalNormalComponentAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("component", "The component of the normal (0 = x, 1 = y, 2 = z)");

  return params;
}

NodalNormalComponentAux::NodalNormalComponentAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _nx(_aux_sys.getVector("nx")),
    _ny(_aux_sys.getVector("ny")),
    _nz(_aux_sys.getVector("nz")),
    _component(getParam<unsigned int>("component"))
{
}

NodalNormalComponentAux::~NodalNormalComponentAux()
{
}

Real
NodalNormalComponentAux::computeValue()
{
  dof_id_type dof = _current_node->id();
  switch (_component)
  {
  case 0: return _nx(dof);
  case 1: return _ny(dof);
  case 2: return _nz(dof);
  default: mooseError("Component in '" + name() + "' can be only 0, 1 or 2.");
  }
}
