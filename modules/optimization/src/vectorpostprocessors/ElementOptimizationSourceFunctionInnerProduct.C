//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementOptimizationSourceFunctionInnerProduct.h"

registerMooseObject("isopodApp", ElementOptimizationSourceFunctionInnerProduct);

InputParameters
ElementOptimizationSourceFunctionInnerProduct::validParams()
{
  InputParameters params = ElementVariableVectorPostprocessor::validParams();
  params.addClassDescription("Computes the inner product of variable with parameterized source "
                             "function for optimization gradient computation.");
  params.addRequiredParam<FunctionName>("function", "Optimization function used for source.");
  params.addParam<Real>(
      "reverse_time_end",
      0.0,
      "End time used for reversiing the time integration when evaluating function derivative.");
  return params;
}

ElementOptimizationSourceFunctionInnerProduct::ElementOptimizationSourceFunctionInnerProduct(
    const InputParameters & parameters)
  : ElementVariableVectorPostprocessor(parameters),
    _var(coupledValue("variable")),
    _function(dynamic_cast<const OptimizationFunction *>(&getFunction("function"))),
    _vec(declareVector("inner_product")),
    _reverse_time_end(getParam<Real>("reverse_time_end"))
{
  if (!_function)
    paramError("function", "Function must be a derived class of OptimizationFunction.");
}

void
ElementOptimizationSourceFunctionInnerProduct::initialize()
{
  _curr_time_ip = nullptr;
  for (auto & pr : _time_ip)
    if (MooseUtils::relativeFuzzyEqual(_t, pr.first))
      _curr_time_ip = &pr.second;
  if (!_curr_time_ip)
  {
    _time_ip.emplace_back(_t, std::vector<Real>());
    _curr_time_ip = &_time_ip.back().second;
  }

  _curr_time_ip->clear();
}

void
ElementOptimizationSourceFunctionInnerProduct::execute()
{
  const Real at =
      MooseUtils::absoluteFuzzyEqual(_reverse_time_end, 0.0) ? _t : _reverse_time_end - _t + _dt;
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    const std::vector<Real> pg = _function->parameterGradient(at, _q_point[qp]);
    _curr_time_ip->resize(std::max(pg.size(), _curr_time_ip->size()), 0.0);
    for (const auto & i : index_range(pg))
      (*_curr_time_ip)[i] += _JxW[qp] * _coord[qp] * _var[qp] * pg[i];
  }
}

void
ElementOptimizationSourceFunctionInnerProduct::threadJoin(const UserObject & y)
{
  const auto & vpp = static_cast<const ElementOptimizationSourceFunctionInnerProduct &>(y);
  _curr_time_ip->resize(std::max(_curr_time_ip->size(), vpp._curr_time_ip->size()), 0.0);
  for (const auto & i : index_range(*vpp._curr_time_ip))
    (*_curr_time_ip)[i] += (*vpp._curr_time_ip)[i];
}

void
ElementOptimizationSourceFunctionInnerProduct::finalize()
{
  std::size_t nvar = _curr_time_ip->size();
  gatherMax(nvar);
  _curr_time_ip->resize(nvar, 0.0);
  gatherSum(*_curr_time_ip);

  if (!_fe_problem.isTransient())
    _vec = (*_curr_time_ip);
  else
  {
    // Make sure everything is the same size
    for (const auto & it : _time_ip)
      nvar = std::max(nvar, it.second.size());
    for (auto & it : _time_ip)
      it.second.resize(nvar);
    _vec.assign(nvar, 0.0);

    // Integrate in time using quadrature rule
    std::sort(_time_ip.begin(),
              _time_ip.end(),
              [](const std::pair<Real, std::vector<Real>> & a,
                 const std::pair<Real, std::vector<Real>> & b) { return a.first < b.first; });
    for (std::size_t ti = 1; ti < _time_ip.size(); ++ti)
      for (const auto & i : make_range(nvar))
        _vec[i] += (_time_ip[ti].second[i] + _time_ip[ti - 1].second[i]) / 2.0 *
                   (_time_ip[ti].first - _time_ip[ti - 1].first);
  }
}
