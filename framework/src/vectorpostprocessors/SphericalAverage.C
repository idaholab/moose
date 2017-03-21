/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SphericalAverage.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<SphericalAverage>()
{
  InputParameters params = validParams<ElementVectorPostprocessor>();
  params.addParam<unsigned int>("bin_number", 50, "Number of histogram bins");
  params.addCoupledVar("variable", "Variables to average radially");
  params.addRequiredParam<Real>("radius", "Radius to average out to");
  params.addParam<Real>(
      "empty_bin_value", 0.0, "Value to assign to bins into which no datapoints fall");
  return params;
}

SphericalAverage::SphericalAverage(const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    _nbins(getParam<unsigned int>("bin_number")),
    _radius(getParam<Real>("radius")),
    _deltaR(_radius / _nbins),
    _nvals(coupledComponents("variable")),
    _values(_nvals),
    _empty_bin_value(getParam<Real>("empty_bin_value")),
    _bin_center(declareVector("radius")),
    _counts(_nbins),
    _average(_nvals)
{
  if (coupledComponents("variable") != 1)
    mooseError("SphericalAverage works on exactly one coupled variable");

  // Note: We associate the local variable "i" with nbins and "j" with nvals throughout.

  // couple variables initialize vectors
  for (auto j = beginIndex(_average); j < _nvals; ++j)
  {
    _values[j] = &coupledValue("variable", j);
    _average[j] = &declareVector(getVar("variable", j)->name());
  }

  // initialize the bin center value vector
  _bin_center.resize(_nbins);
  for (auto i = beginIndex(_counts); i < _nbins; ++i)
    _bin_center[i] = (i + 0.5) * _deltaR;
}

void
SphericalAverage::initialize()
{
  // reset the histogram
  for (auto vec_ptr : _average)
    vec_ptr->assign(_nbins, 0.0);

  // reset bin counts
  _counts.assign(_nbins, 0.0);
}

void
SphericalAverage::execute()
{
  // loop over quadrature points
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // compute target bin
    auto bin = computeDistance() / _deltaR;

    // add the volume contributed by the current quadrature point
    if (bin >= 0 && bin < static_cast<int>(_nbins))
    {
      for (auto j = decltype(_nvals)(0); j < _nvals; ++j)
        (*_average[j])[bin] += (*_values[j])[_qp];

      _counts[bin]++;
    }
  }
}

void
SphericalAverage::finalize()
{
  gatherSum(_counts);

  for (auto j = beginIndex(_average); j < _nvals; ++j)
  {
    gatherSum(*_average[j]);

    for (auto i = beginIndex(_counts); i < _nbins; ++i)
      (*_average[j])[i] =
          _counts[i] > 0 ? (*_average[j])[i] / static_cast<Real>(_counts[i]) : _empty_bin_value;
  }
}

void
SphericalAverage::threadJoin(const UserObject & y)
{
  const SphericalAverage & uo = static_cast<const SphericalAverage &>(y);

  for (auto i = beginIndex(_counts); i < _nbins; ++i)
  {
    _counts[i] += uo._counts[i];

    for (auto j = beginIndex(_average); j < _nvals; ++j)
      (*_average[j])[i] += (*uo._average[j])[i];
  }
}

Real
SphericalAverage::computeDistance()
{
  // overwrite this method to implement cylindrical averages etc.
  return _q_point[_qp].norm();
}
