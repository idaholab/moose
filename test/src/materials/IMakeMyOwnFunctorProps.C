//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IMakeMyOwnFunctorProps.h"
#include "FunctorMaterialProperty.h"

registerMooseObject("MooseTestApp", IMakeMyOwnFunctorProps);

template <typename T>
class CustomFunctorProp : public FunctorMaterialProperty<T>
{
public:
  CustomFunctorProp(const std::string & name,
                    const FunctorInterface<T> & var_functor,
                    const FunctorInterface<T> * const prop_functor = nullptr)
    : FunctorMaterialProperty<T>(name), _var_functor(var_functor), _prop_functor(prop_functor)
  {
  }

  using typename FunctorInterface<T>::FaceArg;
  using typename FunctorInterface<T>::ElemFromFaceArg;
  using typename FunctorInterface<T>::ElemQpArg;
  using typename FunctorInterface<T>::ElemSideQpArg;
  using typename FunctorInterface<T>::FunctorType;
  using typename FunctorInterface<T>::FunctorReturnType;

private:
  template <typename Space, typename Time>
  T evaluateHelper(const Space & r, const Time & t) const
  {
    auto ret = 1. + _var_functor(r, t);
    if (_prop_functor)
      ret *= (*_prop_functor)(r, t);
    return ret;
  }

  T evaluate(const Elem * const & elem, unsigned int state) const override final
  {
    return evaluateHelper(elem, state);
  }
  T evaluate(const ElemFromFaceArg & elem_from_face, unsigned int state) const override final
  {
    return evaluateHelper(elem_from_face, state);
  }
  T evaluate(const FaceArg & face, unsigned int state) const override final
  {
    return evaluateHelper(face, state);
  }
  T evaluate(const ElemQpArg & elem_qp, unsigned int state) const override final
  {
    return evaluateHelper(elem_qp, state);
  }
  T evaluate(const ElemSideQpArg & elem_side_qp, unsigned int state) const override final
  {
    return evaluateHelper(elem_side_qp, state);
  }
  T evaluate(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
             unsigned int state) const override final
  {
    return evaluateHelper(tqp, state);
  }

  const FunctorInterface<T> & _var_functor;
  const FunctorInterface<T> * const _prop_functor;
};

InputParameters
IMakeMyOwnFunctorProps::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params += SetupInterface::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_NONLINEAR};
  params.addCoupledVar("fe_var", 1., "A coupled finite element variable.");
  params.addCoupledVar("fv_var", 1., "A coupled finite volume variable.");
  params.addParam<MaterialPropertyName>("retrieved_prop_name",
                                        "The name of the retrieved property.");
  return params;
}

IMakeMyOwnFunctorProps::IMakeMyOwnFunctorProps(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _fe_var(getFunctor<MooseVariableFE<Real>>("fe_var", 0)),
    _fv_var(getFunctor<MooseVariableFV<Real>>("fv_var", 0)),
    _retrieved_prop(isParamValid("retrieved_prop_name")
                        ? &getFunctorMaterialProperty<ADReal>("retrieved_prop_name")
                        : nullptr),
    _fe_prop(isCoupled("fe_var")
                 ? &declareFunctorProperty<ADReal, CustomFunctorProp>("fe_prop", _fe_var)
                 : nullptr),
    _fv_prop(isCoupled("fv_var") ? &declareFunctorProperty<ADReal, CustomFunctorProp>(
                                       "fv_prop", _fv_var, _retrieved_prop)
                                 : nullptr)
{
  if (_fe_prop)
    _fe_prop->setCacheClearanceSchedule(
        std::set<ExecFlagType>(_execute_enum.begin(), _execute_enum.end()));
  if (_fv_prop)
    _fv_prop->setCacheClearanceSchedule(
        std::set<ExecFlagType>(_execute_enum.begin(), _execute_enum.end()));
}
