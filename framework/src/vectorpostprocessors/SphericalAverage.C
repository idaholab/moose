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

template<>
InputParameters validParams<SphericalAverage>()
{
  InputParameters params = validParams<ElementVectorPostprocessor>();
  params.addParam<unsigned int>("bin_number", 50, "Number of histogram bins");
  params.addCoupledVar("variable", "Variables to average radially");
  params.addRequiredParam<Real>("radius", "Radius to average out to");
  params.addParam<Real>("empty_bin_value", 0.0, "Value to assign to bins into which no datapoints fall");
  return params;
}

SphericalAverage::SphericalAverage(const InputParameters & parameters) :
    ElementVectorPostprocessor(parameters),
    _nbins(getParam<unsigned int>("bin_number")),
    _radius(getParam<Real>("radius")),
    _deltaR(_radius / _nbins),
    _nvals(coupledComponents("variable")),
    _values(_nvals),
    _empty_bin_value(getParam<Real>("empty_bin_value")),
    _bin_center(declareVector("radius")),
    _sum_tmp(_nvals),
    _count_tmp(_nbins),
    _average(_nvals)
{
  if (coupledComponents("variable") != 1)
    mooseError("SphericalAverage works on exactly one coupled variable");

  // couple variables initialize vectors
  for (unsigned int j = 0; j < _nvals; ++j)
  {
    _values[j] = &coupledValue("variable", j);
    _average[j] = &declareVector(getVar("variable", j)->name());
  }

  // initialize the bin center value vector
  _bin_center.resize(_nbins);
  for (unsigned i = 0; i < _nbins; ++i)
    _bin_center[i] = (i + 0.5) * _deltaR;
}

void
SphericalAverage::initialize()
{
  // reset the histogram
  for (unsigned int j = 0; j < _nvals; ++j)
    _sum_tmp[j].assign(_nbins, 0.0);

  // reset bin counts
  _count_tmp.assign(_nbins, 0.0);
}

void
SphericalAverage::execute()
{
  // loop over quadrature points
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // compute target bin
    int bin = computeDistance() / _deltaR;

    // add the volume contributed by the current quadrature point
    if (bin >= 0 && bin < static_cast<int>(_nbins))
    {
      for (unsigned int j = 0; j < _nvals; ++j)
        _sum_tmp[j][bin] += (*_values[j])[_qp];

      _count_tmp[bin]++;
    }
  }
}

void
SphericalAverage::finalize()
{
  gatherSum(_count_tmp);

  for (unsigned int j = 0; j < _nvals; ++j)
  {
    gatherSum(_sum_tmp[j]);
    (*_average[j]).resize(_nbins);

    for (unsigned int i = 0; i < _nbins; ++i)
      (*_average[j])[i] = _count_tmp[i] > 0 ? _sum_tmp[j][i] / _count_tmp[i] : _empty_bin_value;
  }
}

void
SphericalAverage::threadJoin(const UserObject & y)
{
  const SphericalAverage & uo = static_cast<const SphericalAverage &>(y);

  for (unsigned int i = 0; i < _nbins; ++i)
  {
    _count_tmp[i] += uo._count_tmp[i];

    for (unsigned int j = 0; j < _nvals; ++j)
      _sum_tmp[j][i] += uo._sum_tmp[j][i];
  }
}

Real
SphericalAverage::computeDistance()
{
  // overwrite this method to implement cylindrical averages etc.
  return _q_point[_qp].norm();
}
