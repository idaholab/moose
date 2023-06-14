//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurrogateModelAuxKernel.h"

#include "Function.h"

template class AuxKernelTempl<Real>;
template class AuxKernelTempl<RealEigenVector>;

registerMooseObject("StochasticToolsApp", SurrogateModelAuxKernel);
registerMooseObject("StochasticToolsApp", SurrogateModelArrayAuxKernel);

template <typename ComputeValueType>
InputParameters
SurrogateModelAuxKernelTempl<ComputeValueType>::validParams()
{
  InputParameters params = AuxKernelTempl<ComputeValueType>::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription("Sets a value of a variable based on a surrogate model.");
  params.addRequiredParam<UserObjectName>("model", "Name of surrogate models.");
  params.addRequiredParam<std::vector<std::string>>(
      "parameters",
      "Parameter values at which the surrogate is evaluated. These can be post-processors, "
      "functions, variables, or constant numbers.");
  params.addParam<std::vector<std::string>>(
      "scalar_parameters",
      std::vector<std::string>(),
      "Parameters in 'parameters' that are post-processors or functions.");
  params.addCoupledVar("coupled_variables",
                       "Parameters in 'parameters' that are standard variables.");
  params.addCoupledVar("coupled_array_variables",
                       "Parameters in 'parameters' that are array variables.");
  if constexpr (!std::is_same<ComputeValueType, RealEigenVector>::value)
    params.suppressParameter<std::vector<VariableName>>("coupled_array_variables");
  return params;
}

template <typename ComputeValueType>
SurrogateModelAuxKernelTempl<ComputeValueType>::SurrogateModelAuxKernelTempl(
    const InputParameters & parameters)
  : AuxKernelTempl<ComputeValueType>(parameters),
    SurrogateModelInterface(this),
    _model(getSurrogateModel("model")),
    _n_params(this->template getParam<std::vector<std::string>>("parameters").size())
{
  const auto & params = this->template getParam<std::vector<std::string>>("parameters");
  const auto & scalar_params =
      this->template getParam<std::vector<std::string>>("scalar_parameters");
  const auto var_params = this->coupledNames("coupled_variables");
  const auto array_var_params = this->coupledNames("coupled_array_variables");

  for (const auto & p : make_range(_n_params))
  {
    const auto & param = params[p];

    // Iterators if the name was found
    auto sit = std::find(scalar_params.begin(), scalar_params.end(), param);
    auto vit = std::find(var_params.begin(), var_params.end(), param);
    auto ait = std::find(array_var_params.begin(), array_var_params.end(), param);

    // Scalar parameters
    if (sit != scalar_params.end())
    {
      // Postprocessor
      if (this->hasPostprocessorByName(param))
        _pp_params[p] = &this->getPostprocessorValueByName(param);
      // Function
      else if (this->hasFunctionByName(param))
        _function_params[p] = &this->getFunctionByName(param);
      else
        this->paramError(
            "scalar_parameters", "'", param, "' is not a postprocessor or a function.");
    }
    // Standard variable
    else if (vit != var_params.end())
    {
      auto index = std::distance(var_params.begin(), vit);
      _var_params[p] = &this->coupledValue("coupled_variables", index);
    }
    // Array variable
    else if (ait != array_var_params.end())
    {
      auto index = std::distance(array_var_params.begin(), ait);
      _array_var_params[p] = &this->coupledArrayValue("coupled_array_variables", index);
      const auto & cvar = *this->getArrayVar("coupled_array_variables", index);
      if (cvar.count() != this->_var.count())
        this->paramError("coupled_array_variables",
                         "The number of components in '",
                         cvar.name(),
                         "' (",
                         cvar.count(),
                         ") does not match the number of components in '",
                         this->_var.name(),
                         "' (",
                         this->_var.count(),
                         ").");
    }
    else
    {
      Real & val = _constant_params[p];
      try
      {
        val = MooseUtils::convert<Real>(param, true);
      }
      catch (const std::invalid_argument &)
      {
        this->paramError("parameters",
                         "'",
                         param,
                         "' is not listed in 'scalar_parameters', 'coupled_variables', or "
                         "'coupled_array_variables'.");
      }
    }
  }
}

template <typename ComputeValueType>
ComputeValueType
SurrogateModelAuxKernelTempl<ComputeValueType>::computeValue()
{
  std::vector<Real> x(_n_params);
  // insert postprocessors
  for (const auto & [p, pp_ptr] : _pp_params)
    x[p] = *pp_ptr;
  // insert functions
  for (const auto & [p, fun_ptr] : _function_params)
    x[p] = fun_ptr->value(this->_t, this->_q_point[this->_qp]);
  // insert variables
  for (const auto & [p, var_ptr] : _var_params)
    x[p] = (*var_ptr)[this->_qp];
  // insert constant values
  for (const auto & [p, v] : _constant_params)
    x[p] = v;

  return _model.evaluate(x);
}

template <>
RealEigenVector
SurrogateModelAuxKernelTempl<RealEigenVector>::computeValue()
{
  std::vector<Real> x(_n_params);

  // insert postprocessors
  for (const auto & [p, pp_ptr] : _pp_params)
    x[p] = *pp_ptr;
  // insert functions
  for (const auto & [p, fun_ptr] : _function_params)
    x[p] = fun_ptr->value(this->_t, this->_q_point[this->_qp]);
  // insert variables
  for (const auto & [p, var_ptr] : _var_params)
    x[p] = (*var_ptr)[this->_qp];
  // insert constant values
  for (const auto & [p, v] : _constant_params)
    x[p] = v;

  RealEigenVector val(_var.count());
  for (const auto & c : make_range(_var.count()))
  {
    for (const auto & [p, avar_ptr] : _array_var_params)
      x[p] = (*avar_ptr)[this->_qp](c);
    val(c) = _model.evaluate(x);
  }

  return val;
}
