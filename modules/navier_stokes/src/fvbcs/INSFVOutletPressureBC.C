//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVOutletPressureBC.h"
#include "INSFVPressureVariable.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", INSFVOutletPressureBC);

template <class T>
InputParameters
INSFVOutletPressureBCTempl<T>::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params += T::validParams();

  // Value may be specified by a AD functor (typically a variable), a function or a postprocessor
  params.addParam<FunctionName>("function", "The boundary pressure as a regular function");
  params.addParam<MooseFunctorName>("functor", "The boundary pressure as an AD functor");
  params.addParam<PostprocessorName>("postprocessor", "The boundary pressure as a postprocessor");

  return params;
}

template <class T>
INSFVOutletPressureBCTempl<T>::INSFVOutletPressureBCTempl(const InputParameters & params)
  : FVDirichletBCBase(params),
    T(params),
    _functor(isParamValid("functor") ? &this->template getFunctor<ADReal>("functor") : nullptr),
    _function(isParamValid("function") ? &getFunction("function") : nullptr),
    _pp_value(isParamValid("postprocessor") ? &getPostprocessorValue("postprocessor") : nullptr)
{
  if (!dynamic_cast<INSFVPressureVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to INSFVOutletPressureBC must be of type INSFVPressureVariable");

  // Check parameters
  if ((_functor && (_pp_value || _function)) || (_function && _pp_value) ||
      (!_functor && !_pp_value && !_function))
    mooseError("One and only one of function/functor/postprocessor may be specified for the outlet "
               "pressure");
}

template <class T>
ADReal
INSFVOutletPressureBCTempl<T>::boundaryValue(const FaceInfo & fi,
                                             const Moose::StateArg & state) const
{
  if (_functor)
    return (*_functor)(singleSidedFaceArg(&fi), state);
  else if (_function)
  {
    if (state.state != 0 && state.iteration_type == Moose::SolutionIterationType::Time)
    {
      mooseAssert(state.state == 1, "We cannot access values beyond the previous time step.");
      return _function->value(_t_old, fi.faceCentroid());
    }
    else
      return _function->value(_t, fi.faceCentroid());
  }
  else
    return *_pp_value;
}

template class INSFVOutletPressureBCTempl<INSFVFlowBC>;
template class INSFVOutletPressureBCTempl<INSFVFullyDevelopedFlowBC>;
