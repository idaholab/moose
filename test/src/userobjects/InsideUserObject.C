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

#include "InsideUserObject.h"

template<>
InputParameters validParams<InsideUserObject>()
{
  InputParameters params = validParams<InternalSideUserObject>();
  params.addRequiredCoupledVar("variable", "the variable name");

  return params;
}

InsideUserObject::InsideUserObject(const std::string & name, InputParameters parameters) :
    InternalSideUserObject(name, parameters),
    _u(coupledValue("variable")),
    _u_neighbor(coupledNeighborValue("variable")),
    _value(0.)
{
}

InsideUserObject::~InsideUserObject()
{
}

void
InsideUserObject::initialize()
{
  _value = 0;
}

void
InsideUserObject::execute()
{
  for (unsigned int qp = 0; qp < _q_point.size(); ++qp)
    _value += std::pow(_u[qp] - _u_neighbor[qp], 2);
}

void
InsideUserObject::destroy()
{
}

void
InsideUserObject::finalize()
{
  gatherSum(_value);
  _value = std::sqrt(_value);
}

void
InsideUserObject::threadJoin(const UserObject & uo)
{
  const InsideUserObject & u = dynamic_cast<const InsideUserObject &>(uo);
  _value += u._value;
}
