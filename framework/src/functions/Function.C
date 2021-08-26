//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Function.h"

defineLegacyParams(Function);

InputParameters
Function::validParams()
{
  InputParameters params = MooseObject::validParams();

  params.registerBase("Function");

  return params;
}

Function::Function(const InputParameters & parameters)
  : MooseObject(parameters),
    SetupInterface(this),
    TransientInterface(this),
    PostprocessorInterface(this),
    UserObjectInterface(this),
    Restartable(this, "Functions"),
    MeshChangedInterface(parameters),
    ScalarCoupleable(this)
{
}

Function::~Function() {}

Real
Function::value(Real /*t*/, const Point & /*p*/) const
{
  return 0.0;
}

RealGradient
Function::gradient(Real /*t*/, const Point & /*p*/) const
{
  return RealGradient(0, 0, 0);
}

Real
Function::timeDerivative(Real /*t*/, const Point & /*p*/) const
{
  mooseError("timeDerivative method not defined for function ", name());
  return 0;
}

RealVectorValue
Function::vectorValue(Real /*t*/, const Point & /*p*/) const
{
  return RealVectorValue(0, 0, 0);
}

RealVectorValue
Function::vectorCurl(Real /*t*/, const Point & /*p*/) const
{
  return RealVectorValue(0, 0, 0);
}

Real
Function::integral() const
{
  mooseError("Integral method not defined for function ", name());
  return 0;
}

Real
Function::average() const
{
  mooseError("Average method not defined for function ", name());
  return 0;
}

Real
Function::getTime(const unsigned int state) const
{
  switch (state)
  {
    case 0:
      return _ti_feproblem.time();

    case 1:
      return _ti_feproblem.timeOld();

    default:
      mooseError("unhandled state ", state, " in Function::getTime");
  }
}

Real
Function::evaluate(const Elem * const & elem, const unsigned int state) const
{
  return value(getTime(state), elem->centroid());
}

Real
Function::evaluate(const ElemAndFaceArg & elem_and_face, const unsigned int state) const
{
  return value(getTime(state), std::get<0>(elem_and_face)->centroid());
}

Real
Function::evaluate(const FaceArg & face, const unsigned int state) const
{
  return value(getTime(state), std::get<0>(face)->faceCentroid());
}

Real
Function::evaluate(const QpArg & /*qp*/, unsigned int /*state*/) const
{
  mooseError("Not yet implemented");
}

Real
Function::evaluate(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & /*tqp*/,
                   unsigned int /*state*/) const
{
  mooseError("Not yet implemented");
}

void
Function::timestepSetup()
{
  FunctorInterface<Real>::timestepSetup();
}

void
Function::residualSetup()
{
  FunctorInterface<Real>::residualSetup();
}

void
Function::jacobianSetup()
{
  FunctorInterface<Real>::jacobianSetup();
}
