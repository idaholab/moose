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
  mooseAssert(std::get<1>(elem_and_face), "We must have a non-null face information pointer");
  return value(getTime(state), std::get<1>(elem_and_face)->faceCentroid());
}

Real
Function::evaluate(const FaceArg & face, const unsigned int state) const
{
  return value(getTime(state), std::get<0>(face)->faceCentroid());
}

Real
Function::evaluate(const QpArg & qp_arg, const unsigned int state) const
{
  const Elem * const elem = std::get<0>(qp_arg);
  const auto qp = std::get<1>(qp_arg);
  if (elem != _current_functor_elem)
  {
    _current_functor_elem = elem;
    const QBase * const qrule_template = std::get<2>(qp_arg);

    const FEFamily mapping_family = FEMap::map_fe_type(*elem);
    const FEType fe_type(elem->default_order(), mapping_family);

    std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), fe_type));
    std::unique_ptr<QBase> qrule(QBase::build(
        qrule_template->type(), qrule_template->get_dim(), qrule_template->get_order()));

    const auto & xyz = fe->get_xyz();
    fe->attach_quadrature_rule(qrule.get());
    fe->reinit(elem);
    _current_functor_xyz = xyz;
  }
  mooseAssert(qp < _current_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return value(getTime(state), _current_functor_xyz[qp]);
}

Real
Function::evaluate(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & /*tqp*/,
                   unsigned int /*state*/) const
{
  mooseError(
      "The ElementType evaluate overload is not supported by Function because there is no simple "
      "way to determine the location of quadrature points without being provided an element");
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
