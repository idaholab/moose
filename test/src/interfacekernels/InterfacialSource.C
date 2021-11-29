//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfacialSource.h"

// MOOSE
#include "Function.h"
#include "SystemBase.h"
#include "Assembly.h"

registerMooseObject("MooseTestApp", InterfacialSource);

InputParameters
InterfacialSource::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addClassDescription(
      "Demonstrates the multiple ways that scalar values can be introduced "
      "into interface kernels, e.g. (controllable) constants, functions, and "
      "postprocessors.");
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
  params.addParam<FunctionName>("function", "1", "A function that describes the body force");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.set<bool>("_use_undisplaced_reference_points") = true;
  params.declareControllable("value");
  return params;
}

InterfacialSource::InterfacialSource(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _scale(getParam<Real>("value")),
    _function(getFunction("function")),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _neighbor_JxW(_assembly.JxWNeighbor())
{
}

Real
InterfacialSource::computeQpResidual(Moose::DGResidualType type)
{
  Real residual = -_scale * _postprocessor * _function.value(_t, _q_point[_qp]);

  switch (type)
  {
    case Moose::Element:
      residual *= _test[_i][_qp];
      break;

    case Moose::Neighbor:
      residual *= _test_neighbor[_i][_qp];
      break;

    default:
      mooseError("Unrecognized Moose::DGResidualType type");
  }

  return residual;
}

void
InterfacialSource::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  const MooseArray<Real> * JxW;

  if (type == Moose::Element)
  {
    is_elem = true;
    JxW = &_JxW;
  }
  else
  {
    is_elem = false;
    JxW = &_neighbor_JxW;
  }

  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;

  if (is_elem)
    prepareVectorTag(_assembly, _var.number());
  else
    prepareVectorTagNeighbor(_assembly, _neighbor_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      _local_re(_i) += (*JxW)[_qp] * _coord[_qp] * computeQpResidual(type);

  accumulateTaggedLocalResidual();

  if (_has_primary_residuals_saved_in && is_elem)
  {
    Threads::spin_mutex::scoped_lock lock(_resid_vars_mutex);
    for (const auto & var : _primary_save_in_residual_variables)
    {
      var->sys().solution().add_vector(_local_re, var->dofIndices());
    }
  }
  else if (_has_secondary_residuals_saved_in && !is_elem)
  {
    Threads::spin_mutex::scoped_lock lock(_resid_vars_mutex);
    for (const auto & var : _secondary_save_in_residual_variables)
      var->sys().solution().add_vector(_local_re, var->dofIndicesNeighbor());
  }
}

void InterfacialSource::computeElemNeighJacobian(Moose::DGJacobianType) {}
