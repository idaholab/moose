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

registerMooseObject("NavierStokesApp", INSFVOutletPressureBC);

InputParameters
INSFVOutletPressureBC::validParams()
{
  InputParameters params = FVDirichletBCBase::validParams();
  params += INSFVFullyDevelopedFlowBC::validParams();

  // Value may be specified by a functor (function) or a postprocessor
  params.addDeprecatedParam<FunctionName>("function",
                                          "The pressure as a function.",
                                          "Use functor instead");
  params.addParam<MooseFunctorName>("functor",
                                    "The boundary pressure as a functor (most often a function)");
  params.addParam<PostprocessorName>("postprocessor", "The boundary pressure as a postprocessor");

  return params;
}

INSFVOutletPressureBC::INSFVOutletPressureBC(const InputParameters & params)
  : FVDirichletBCBase(params),
  INSFVFullyDevelopedFlowBC(params),
  _functor(isParamValid("functor") ?
           &getFunctor<ADReal>("functor") :
           isParamValid("function") ?
           &getFunctor<ADReal>("function") :
           nullptr),
  _pp_value(isParamValid("postprocessor") ?
            &getPostprocessorValue("postprocessor") :
            nullptr)
{
  if (!dynamic_cast<INSFVPressureVariable *>(&_var))
    paramError(
        "variable",
        "The variable argument to INSFVOutletPressureBC must be of type INSFVPressureVariable");

  // Check parameters
  if ((_functor && _pp_value) || (!_functor && !_pp_value))
    mooseError(
        "One and only one of function/functor/postprocessor may be specified for the outlet pressure");
}

Real
INSFVOutletPressureBC::boundaryValue(const FaceInfo & fi) const
{
  if (_functor)
    return (*_functor)(singleSidedFaceArg(&fi)).value();
  else
    return *_pp_value;
}
