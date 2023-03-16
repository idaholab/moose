//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpatialAverageBase.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

InputParameters
SpatialAverageBase::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params.addParam<unsigned int>("bin_number", 50, "Number of histogram bins");
  params.addCoupledVar("variable", "Variables to average radially");
  params.addRequiredParam<Real>("radius", "Radius to average out to");
  params.addParam<Point>("origin", Point(), "Origin of the cylinder");
  params.addParam<Real>(
      "empty_bin_value", 0.0, "Value to assign to bins into which no datapoints fall");
  return params;
}

SpatialAverageBase::SpatialAverageBase(const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    _nbins(getParam<unsigned int>("bin_number")),
    _radius(getParam<Real>("radius")),
    _origin(getParam<Point>("origin")),
    _deltaR(_radius / _nbins),
    _nvals(coupledComponents("variable")),
    _values(coupledValues("variable")),
    _empty_bin_value(getParam<Real>("empty_bin_value")),
    _bin_center(declareVector("radius")),
    _counts(_nbins),
    _average(_nvals)
{
  if (coupledComponents("variable") != 1)
    mooseError("SpatialAverageBase works on exactly one coupled variable");

  // Note: We associate the local variable "i" with nbins and "j" with nvals throughout.

  // couple variables initialize vectors
  for (MooseIndex(_average) j = 0; j < _nvals; ++j)
    _average[j] = &declareVector(coupledName("variable", j));

  // initialize the bin center value vector
  _bin_center.resize(_nbins);
  for (MooseIndex(_counts) i = 0; i < _nbins; ++i)
    _bin_center[i] = (i + 0.5) * _deltaR;
}

void
SpatialAverageBase::initialize()
{
  // reset the histogram
  for (auto vec_ptr : _average)
    vec_ptr->assign(_nbins, 0.0);

  // reset bin counts
  _counts.assign(_nbins, 0.0);
}

void
SpatialAverageBase::execute()
{
  // loop over quadrature points
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // compute target bin
    auto bin = computeDistance() / _deltaR;

    // add the volume contributed by the current quadrature point
    if (bin >= 0 && bin < static_cast<int>(_nbins))
    {
      for (MooseIndex(_nvals) j = 0; j < _nvals; ++j)
        (*_average[j])[bin] += (*_values[j])[_qp];

      _counts[bin]++;
    }
  }
}

void
SpatialAverageBase::finalize()
{
  gatherSum(_counts);

  for (MooseIndex(_average) j = 0; j < _nvals; ++j)
  {
    gatherSum(*_average[j]);

    for (MooseIndex(_counts) i = 0; i < _nbins; ++i)
      (*_average[j])[i] =
          _counts[i] > 0 ? (*_average[j])[i] / static_cast<Real>(_counts[i]) : _empty_bin_value;
  }
}

void
SpatialAverageBase::threadJoin(const UserObject & y)
{
  const SpatialAverageBase & uo = static_cast<const SpatialAverageBase &>(y);

  for (MooseIndex(_counts) i = 0; i < _nbins; ++i)
  {
    _counts[i] += uo._counts[i];

    for (MooseIndex(_average) j = 0; j < _nvals; ++j)
      (*_average[j])[i] += (*uo._average[j])[i];
  }
}
