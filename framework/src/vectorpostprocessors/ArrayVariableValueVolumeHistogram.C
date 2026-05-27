//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayVariableValueVolumeHistogram.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", ArrayVariableValueVolumeHistogram);

InputParameters
ArrayVariableValueVolumeHistogram::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params.addClassDescription("Compute histograms of volume fractions binned according to component "
                             "values of an array variable.");
  params.addParam<unsigned int>("bin_number", 50, "Number of histogram bins");
  params.addCoupledVar("variable", "Variable to bin the volume of");
  params.addRequiredParam<Real>("min_value", "Minimum variable value");
  params.addRequiredParam<Real>("max_value", "Maximum variable value");
  return params;
}

ArrayVariableValueVolumeHistogram::ArrayVariableValueVolumeHistogram(
    const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    _nbins(getParam<unsigned int>("bin_number")),
    _min_value(getParam<Real>("min_value")),
    _max_value(getParam<Real>("max_value")),
    _deltaV((_max_value - _min_value) / _nbins),
    _value(coupledArrayValue("variable")),
    _var(*getArrayVar("variable", 0)),
    _bin_center(declareVector("value"))
{
  if (coupledComponents("variable") != 1)
    mooseError("ArrayVariableValueVolumeHistogram works on exactly one coupled variable");

  for (const unsigned int i : make_range(_var.count()))
    _volumes.push_back(&declareVector(_var.arrayVariableComponent(i)));

  // initialize the bin center value vector
  _bin_center.resize(_nbins);
  for (const unsigned int i : make_range(_nbins))
    _bin_center[i] = (i + 0.5) * _deltaV + _min_value;
}

void
ArrayVariableValueVolumeHistogram::initialize()
{
  // reset the histogram
  for (auto & volume : _volumes)
    volume->assign(_nbins, 0.0);
}

void
ArrayVariableValueVolumeHistogram::execute()
{
  // loop over quadrature points
  for (auto _qp : make_range(_qrule->n_points()))
  {
    for (const unsigned int i : make_range(_var.count()))
    {
      // compute target bin
      int bin = (_value[_qp](i) - _min_value) / _deltaV;

      // add the volume contributed by the current quadrature point
      if (bin >= 0 && static_cast<unsigned int>(bin) < _nbins)
        (*_volumes[i])[bin] += _JxW[_qp] * _coord[_qp];
    }
  }
}

void
ArrayVariableValueVolumeHistogram::finalize()
{
  for (const unsigned int i : make_range(_var.count()))
    gatherSum(*_volumes[i]);
}

void
ArrayVariableValueVolumeHistogram::threadJoin(const UserObject & y)
{
  const auto & uo = static_cast<const ArrayVariableValueVolumeHistogram &>(y);
  mooseAssert(uo._volumes.size() == _volumes.size(),
              "Inconsistent number of array variable components across threads.");

  for (const unsigned int i : make_range(_var.count()))
  {
    mooseAssert(uo._volumes[i]->size() == _volumes[i]->size(),
                "Inconsistent volume vector lengths across threads.");
    for (const unsigned int j : index_range(*_volumes[i]))
      (*_volumes[i])[j] += (*uo._volumes[i])[j];
  }
}
