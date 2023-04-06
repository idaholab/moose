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

InputParameters
INSFVOutletPressureBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params += INSFVFullyDevelopedFlowBC::validParams();

  // Value may be specified by a AD functor (typically a variable), a function or a postprocessor
  params.addParam<FunctionName>("function", "The boundary pressure as a regular function");
  params.addParam<MooseFunctorName>("functor", "The boundary pressure as an AD functor");
  params.addParam<PostprocessorName>("postprocessor", "The boundary pressure as a postprocessor");

  return params;
}

INSFVOutletPressureBC::INSFVOutletPressureBC(const InputParameters & params)
  : FVDirichletBCBase(params),
    INSFVFullyDevelopedFlowBC(params),
    _functor(isParamValid("functor") ? &getFunctor<ADReal>("functor") : nullptr),
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

ADReal
INSFVOutletPressureBC::boundaryValue(const FaceInfo & fi) const
{
  if (_functor)
    return (*_functor)(singleSidedFaceArg(&fi), determineState());
  else if (_function)
    return _function->value(_t, fi.faceCentroid());
  else
    return *_pp_value;
}
