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

defineLegacyParams(Function);

template <typename T>
InputParameters
FunctionTempl<T>::validParams()
{
  InputParameters params = MooseFunctionBase::validParams();

  params.registerBase("Function");

  return params;
}

template <typename T>
FunctionTempl<T>::FunctionTempl(const InputParameters & parameters)
  : MooseFunctionBase(parameters),
    TransientInterface(this),
    PostprocessorInterface(this),
    UserObjectInterface(this),
    Restartable(this, "Functions"),
    MeshChangedInterface(parameters),
    ScalarCoupleable(this)
{
}

template <typename T>
FunctionTempl<T>::~FunctionTempl()
{
}

template <typename T>
Real
FunctionTempl<T>::value(Real /*t*/, const Point & /*p*/) const
{
  mooseError("value method not implemented");
  return 0.0;
}

template <typename T>
RealGradient
FunctionTempl<T>::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError("gradient method not implemented");
  return RealGradient(0, 0, 0);
}

template <typename T>
Real
FunctionTempl<T>::timeDerivative(Real /*t*/, const Point & /*p*/) const
{
  mooseError("timeDerivative method not implemented");
  return 0;
}

template <typename T>
RealVectorValue
FunctionTempl<T>::vectorValue(Real /*t*/, const Point & /*p*/) const
{
  mooseError("vectorValue method not implemented");
  return RealVectorValue(0, 0, 0);
}

template <typename T>
RealVectorValue
FunctionTempl<T>::vectorCurl(Real /*t*/, const Point & /*p*/) const
{
  mooseError("vectorCurl method not implemented");
  return RealVectorValue(0, 0, 0);
}

template <typename T>
Real
FunctionTempl<T>::integral() const
{
  mooseError("Integral method not implemented for function ", name());
  return 0;
}

template <typename T>
Real
FunctionTempl<T>::average() const
{
  mooseError("Average method not implemented for function ", name());
  return 0;
}

template <typename T>
Real
FunctionTempl<T>::getTime(const unsigned int state) const
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

template <typename T>
void
FunctionTempl<T>::determineElemXYZ(const ElemQpArg & elem_qp) const
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

template <typename T>
void
FunctionTempl<T>::determineElemSideXYZ(const ElemSideQpArg & elem_side_qp) const
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

template <typename T>
typename FunctionTempl<T>::ValueType
FunctionTempl<T>::evaluate(const ElemArg & elem_arg, const unsigned int state) const
{
  return value(getTime(state), elem_arg.elem->vertex_average());
}

template <typename T>
typename FunctionTempl<T>::ValueType
FunctionTempl<T>::evaluate(const ElemFromFaceArg & elem_from_face, const unsigned int state) const
{
  const auto * const elem = elem_from_face.elem;
  const auto * const fi = elem_from_face.fi;
  mooseAssert(fi, "We must have a non-null face information pointer");
  return value(getTime(state), (elem == &fi->elem()) ? fi->elemCentroid() : fi->neighborCentroid());
}

template <typename T>
typename FunctionTempl<T>::ValueType
FunctionTempl<T>::evaluate(const FaceArg & face, const unsigned int state) const
{
  return value(getTime(state), face.fi->faceCentroid());
}
template <typename T>
typename FunctionTempl<T>::ValueType
FunctionTempl<T>::evaluate(const SingleSidedFaceArg & face, const unsigned int state) const
{
  return value(getTime(state), face.fi->faceCentroid());
}

template <typename T>
typename FunctionTempl<T>::ValueType
FunctionTempl<T>::evaluate(const ElemQpArg & elem_qp, const unsigned int state) const
{
  determineElemXYZ(elem_qp);
  const auto qp = std::get<1>(elem_qp);
  mooseAssert(qp < _current_elem_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return value(getTime(state), _current_elem_qp_functor_xyz[qp]);
}

template <typename T>
typename FunctionTempl<T>::ValueType
FunctionTempl<T>::evaluate(const ElemSideQpArg & elem_side_qp, const unsigned int state) const
{
  determineElemSideXYZ(elem_side_qp);
  const auto qp = std::get<2>(elem_side_qp);
  mooseAssert(qp < _current_elem_side_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return value(getTime(state), _current_elem_side_qp_functor_xyz[qp]);
}

template <typename T>
typename FunctionTempl<T>::GradientType
FunctionTempl<T>::evaluateGradient(const ElemArg & elem_arg, const unsigned int state) const
{
  return gradient(getTime(state), elem_arg.elem->vertex_average());
}

template <typename T>
typename FunctionTempl<T>::GradientType
FunctionTempl<T>::evaluateGradient(const ElemFromFaceArg & elem_from_face,
                                   const unsigned int state) const
{
  const auto * const elem = elem_from_face.elem;
  const auto * const fi = elem_from_face.fi;
  mooseAssert(fi, "We must have a non-null face information pointer");
  return gradient(getTime(state),
                  (elem == &fi->elem()) ? fi->elemCentroid() : fi->neighborCentroid());
}

template <typename T>
typename FunctionTempl<T>::GradientType
FunctionTempl<T>::evaluateGradient(const FaceArg & face, const unsigned int state) const
{
  return gradient(getTime(state), face.fi->faceCentroid());
}

template <typename T>
typename FunctionTempl<T>::GradientType
FunctionTempl<T>::evaluateGradient(const SingleSidedFaceArg & face, const unsigned int state) const
{
  return gradient(getTime(state), face.fi->faceCentroid());
}

template <typename T>
typename FunctionTempl<T>::GradientType
FunctionTempl<T>::evaluateGradient(const ElemQpArg & elem_qp, const unsigned int state) const
{
  determineElemXYZ(elem_qp);
  const auto qp = std::get<1>(elem_qp);
  mooseAssert(qp < _current_elem_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return gradient(getTime(state), _current_elem_qp_functor_xyz[qp]);
}

template <typename T>
typename FunctionTempl<T>::GradientType
FunctionTempl<T>::evaluateGradient(const ElemSideQpArg & elem_side_qp,
                                   const unsigned int state) const
{
  determineElemSideXYZ(elem_side_qp);
  const auto qp = std::get<2>(elem_side_qp);
  mooseAssert(qp < _current_elem_side_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return gradient(getTime(state), _current_elem_side_qp_functor_xyz[qp]);
}

template <typename T>
typename FunctionTempl<T>::DotType
FunctionTempl<T>::evaluateDot(const ElemArg & elem_arg, const unsigned int state) const
{
  return timeDerivative(getTime(state), elem_arg.elem->vertex_average());
}

template <typename T>
typename FunctionTempl<T>::DotType
FunctionTempl<T>::evaluateDot(const ElemFromFaceArg & elem_from_face,
                              const unsigned int state) const
{
  const auto * const elem = elem_from_face.elem;
  const auto * const fi = elem_from_face.fi;
  mooseAssert(fi, "We must have a non-null face information pointer");
  return timeDerivative(getTime(state),
                        (elem == &fi->elem()) ? fi->elemCentroid() : fi->neighborCentroid());
}

template <typename T>
typename FunctionTempl<T>::DotType
FunctionTempl<T>::evaluateDot(const FaceArg & face, const unsigned int state) const
{
  return timeDerivative(getTime(state), face.fi->faceCentroid());
}

template <typename T>
typename FunctionTempl<T>::DotType
FunctionTempl<T>::evaluateDot(const SingleSidedFaceArg & face, const unsigned int state) const
{
  return timeDerivative(getTime(state), face.fi->faceCentroid());
}

template <typename T>
typename FunctionTempl<T>::DotType
FunctionTempl<T>::evaluateDot(const ElemQpArg & elem_qp, const unsigned int state) const
{
  determineElemXYZ(elem_qp);
  const auto qp = std::get<1>(elem_qp);
  mooseAssert(qp < _current_elem_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return timeDerivative(getTime(state), _current_elem_qp_functor_xyz[qp]);
}

template <typename T>
typename FunctionTempl<T>::DotType
FunctionTempl<T>::evaluateDot(const ElemSideQpArg & elem_side_qp, const unsigned int state) const
{
  determineElemSideXYZ(elem_side_qp);
  const auto qp = std::get<2>(elem_side_qp);
  mooseAssert(qp < _current_elem_side_qp_functor_xyz.size(),
              "The requested " << qp << " is outside our xyz size");
  return timeDerivative(getTime(state), _current_elem_side_qp_functor_xyz[qp]);
}

template <typename T>
void
FunctionTempl<T>::timestepSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  FunctorBase<T>::timestepSetup();
}

template <typename T>
void
FunctionTempl<T>::residualSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  FunctorBase<T>::residualSetup();
}

template <typename T>
void
FunctionTempl<T>::jacobianSetup()
{
  _current_elem_qp_functor_elem = nullptr;
  _current_elem_side_qp_functor_elem_side = std::make_pair(nullptr, libMesh::invalid_uint);
  FunctorBase<T>::jacobianSetup();
}

template class FunctionTempl<Real>;
template class FunctionTempl<ADReal>;
