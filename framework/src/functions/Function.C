//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Function.h"

using namespace Moose;

InputParameters
Function::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += SetupInterface::validParams();

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
    ScalarCoupleable(this),
    Moose::FunctorBase<Real>(name())
{
}

Function::~Function() {}

Real
Function::value(Real /*t*/, const Point & /*p*/) const
{
  mooseError("value method not implemented");
  return 0.0;
}

ADReal
Function::value(const ADReal & t, const ADPoint & p) const
{
  const auto rt = MetaPhysicL::raw_value(t);
  const auto rp = MetaPhysicL::raw_value(p);
  const auto grad = gradient(rt, rp);
  ADReal ret = value(rt, rp);
  ret.derivatives() = grad(0) * p(0).derivatives()
#if LIBMESH_DIM > 1
                      + grad(1) * p(1).derivatives()
#endif
#if LIBMESH_DIM > 2
                      + grad(2) * p(2).derivatives()
#endif
                      + timeDerivative(rt, rp) * t.derivatives();
  return ret;
}

ChainedReal
Function::value(const ChainedReal & t) const
{
  static const Point p;
  return {value(t.value(), p), timeDerivative(t.value(), p) * t.derivatives()};
}

RealGradient
Function::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError("gradient method not implemented");
  return RealGradient(0, 0, 0);
}

Real
Function::timeDerivative(Real /*t*/, const Point & /*p*/) const
{
  mooseError("timeDerivative method not implemented");
  return 0;
}

RealVectorValue
Function::vectorValue(Real /*t*/, const Point & /*p*/) const
{
  mooseError("vectorValue method not implemented");
  return RealVectorValue(0, 0, 0);
}

RealVectorValue
Function::vectorCurl(Real /*t*/, const Point & /*p*/) const
{
  mooseError("vectorCurl method not implemented");
  return RealVectorValue(0, 0, 0);
}

Real
Function::integral() const
{
  mooseError("Integral method not implemented for function ", name());
  return 0;
}

Real
Function::average() const
{
  mooseError("Average method not implemented for function ", name());
  return 0;
}

Real
Function::getTime(const Moose::StateArg & state) const
{
  if (state.iteration_type != Moose::SolutionIterationType::Time)
    // If we are any iteration type other than time (e.g. nonlinear), then temporally we are still
    // in the present time
    return _ti_feproblem.time();

  switch (state.state)
  {
    case 0:
      return _ti_feproblem.time();

    case 1:
      return _ti_feproblem.timeOld();

    default:
      mooseError("unhandled state ", state.state, " in Function::getTime");
  }
}

void
Function::determineElemXYZ(const ElemQpArg & elem_qp) const
{
  const Elem * const elem = std::get<0>(elem_qp);
  if (elem != _current_elem_qp_functor_elem)
  {
    _current_elem_qp_functor_elem = elem;
    const QBase * const qrule_template = std::get<2>(elem_qp);

    const FEFamily mapping_family = FEMap::map_fe_type(*elem);
    const FEType fe_type(elem->default_order(), mapping_family);

    std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), fe_type));
    std::unique_ptr<QBase> qrule(QBase::build(
        qrule_template->type(), qrule_template->get_dim(), qrule_template->get_order()));

    auto & xyz = fe->get_xyz();
    fe->attach_quadrature_rule(qrule.get());
    fe->reinit(elem);
    _current_elem_qp_functor_xyz = std::move(xyz);
  }
}

void
Function::determineElemSideXYZ(const ElemSideQpArg & elem_side_qp) const
{
  const Elem * const elem = std::get<0>(elem_side_qp);
  const auto side = std::get<1>(elem_side_qp);
  if (elem != _current_elem_side_qp_functor_elem_side.first ||
      side != _current_elem_side_qp_functor_elem_side.second)
  {
    _current_elem_side_qp_functor_elem_side = std::make_pair(elem, side);
    const QBase * const qrule_template = std::get<3>(elem_side_qp);

    const FEFamily mapping_family = FEMap::map_fe_type(*elem);
    const FEType fe_type(elem->default_order(), mapping_family);

    std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), fe_type));
    std::unique_ptr<QBase> qrule(QBase::build(
        qrule_template->type(), qrule_template->get_dim(), qrule_template->get_order()));

    auto & xyz = fe->get_xyz();
    fe->attach_quadrature_rule(qrule.get());
    fe->reinit(elem, side);
    _current_elem_side_qp_functor_xyz = std::move(xyz);
  }
}

typename Function::ValueType
Function::evaluate(const ElemArg & elem_arg, const Moose::StateArg & state) const
{
  return value(getTime(state), elem_arg.elem->vertex_average());
}

typename Function::ValueType
Function::evaluate(const FaceArg & face, const Moose::StateArg & state) const
{
  if (face.face_side && face.fi->neighborPtr() &&
      (face.fi->elem().subdomain_id() != face.fi->neighbor().subdomain_id()))
  {
    // Some users like to put discontinuities in their functions at subdomain changes in which case
    // in order to always get the proper discontinuous effect we should evaluate ever so slightly
    // off the face. Consider evaluation of: if(x < 0, -1, 1) if the face centroid is right at x ==
    // 0 for example. The user likely doesn't want you to return 1 if they've asked for a 0-
    // evaluation
    //
    // I can't quite tell but I think the tolerance for comparing coordinates (x, y, z, t) in
    // fparser is ~1e-9 so we need to use something larger than that. The comparison is absolute
    static constexpr Real offset_tolerance = 1e-8;
    auto offset = offset_tolerance * face.fi->normal();
    if (face.face_side == face.fi->elemPtr())
      offset *= -1;
    return value(getTime(state), face.fi->faceCentroid() + offset);
  }
  else
    return value(getTime(state), face.fi->faceCentroid());
}

typename Function::ValueType
Function::evaluate(const ElemQpArg & elem_qp, const Moose::StateArg & state) const
{
  determineElemXYZ(elem_qp);
  const auto qp = std::get<1>(elem_qp);
  mooseAssert(qp < _current_elem_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return value(getTime(state), _current_elem_qp_functor_xyz[qp]);
}

typename Function::ValueType
Function::evaluate(const ElemSideQpArg & elem_side_qp, const Moose::StateArg & state) const
{
  determineElemSideXYZ(elem_side_qp);
  const auto qp = std::get<2>(elem_side_qp);
  mooseAssert(qp < _current_elem_side_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return value(getTime(state), _current_elem_side_qp_functor_xyz[qp]);
}

typename Function::ValueType
Function::evaluate(const ElemPointArg & elem_point_arg, const Moose::StateArg & state) const
{
  return value(getTime(state), elem_point_arg.point);
}

typename Function::GradientType
Function::evaluateGradient(const ElemArg & elem_arg, const Moose::StateArg & state) const
{
  return gradient(getTime(state), elem_arg.elem->vertex_average());
}

typename Function::GradientType
Function::evaluateGradient(const FaceArg & face, const Moose::StateArg & state) const
{
  return gradient(getTime(state), face.fi->faceCentroid());
}

typename Function::GradientType
Function::evaluateGradient(const ElemQpArg & elem_qp, const Moose::StateArg & state) const
{
  determineElemXYZ(elem_qp);
  const auto qp = std::get<1>(elem_qp);
  mooseAssert(qp < _current_elem_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return gradient(getTime(state), _current_elem_qp_functor_xyz[qp]);
}

typename Function::GradientType
Function::evaluateGradient(const ElemSideQpArg & elem_side_qp, const Moose::StateArg & state) const
{
  determineElemSideXYZ(elem_side_qp);
  const auto qp = std::get<2>(elem_side_qp);
  mooseAssert(qp < _current_elem_side_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return gradient(getTime(state), _current_elem_side_qp_functor_xyz[qp]);
}

typename Function::GradientType
Function::evaluateGradient(const ElemPointArg & elem_point_arg, const Moose::StateArg & state) const
{
  return gradient(getTime(state), elem_point_arg.point);
}

typename Function::DotType
Function::evaluateDot(const ElemArg & elem_arg, const Moose::StateArg & state) const
{
  return timeDerivative(getTime(state), elem_arg.elem->vertex_average());
}

typename Function::DotType
Function::evaluateDot(const FaceArg & face, const Moose::StateArg & state) const
{
  return timeDerivative(getTime(state), face.fi->faceCentroid());
}

typename Function::DotType
Function::evaluateDot(const ElemQpArg & elem_qp, const Moose::StateArg & state) const
{
  determineElemXYZ(elem_qp);
  const auto qp = std::get<1>(elem_qp);
  mooseAssert(qp < _current_elem_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return timeDerivative(getTime(state), _current_elem_qp_functor_xyz[qp]);
}

typename Function::DotType
Function::evaluateDot(const ElemSideQpArg & elem_side_qp, const Moose::StateArg & state) const
{
  determineElemSideXYZ(elem_side_qp);
  const auto qp = std::get<2>(elem_side_qp);
  mooseAssert(qp < _current_elem_side_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return timeDerivative(getTime(state), _current_elem_side_qp_functor_xyz[qp]);
}

typename Function::DotType
Function::evaluateDot(const ElemPointArg & elem_point_arg, const Moose::StateArg & state) const
{
  return timeDerivative(getTime(state), elem_point_arg.point);
}

void
Function::timestepSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  FunctorBase<Real>::timestepSetup();
}

void
Function::residualSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  FunctorBase<Real>::residualSetup();
}

void
Function::jacobianSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  FunctorBase<Real>::jacobianSetup();
}

void
Function::customSetup(const ExecFlagType & exec_type)
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  FunctorBase<Real>::customSetup(exec_type);
}
