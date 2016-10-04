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

#include "VolumeHistogram.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<VolumeHistogram>()
{
  InputParameters params = validParams<ElementVectorPostprocessor>();
  params.addParam<unsigned int>("bin_number", 50, "Number of histogram bins");
  params.addCoupledVar("variable", "Variable to bin the volume of");
  params.addRequiredParam<Real>("min_value", "Minimum variable value");
  params.addRequiredParam<Real>("max_value", "Maximum variable value");
  return params;
}

VolumeHistogram::VolumeHistogram(const InputParameters & parameters) :
    ElementVectorPostprocessor(parameters),
    _nbins(getParam<unsigned int>("bin_number")),
    _min_value(getParam<Real>("min_value")),
    _max_value(getParam<Real>("max_value")),
    _deltaV((_max_value - _min_value) / _nbins),
    _value(coupledValue("variable")),
    _bin_center(declareVector(getVar("variable", 0)->name())),
    _volume_tmp(_nbins),
    _volume(declareVector("n"))
{
  if (coupledComponents("variable") != 1)
    mooseError("VolumeHistogram works on exactly one coupled variable");

  // initialize the bin center value vector
  _bin_center.resize(_nbins);
  for (unsigned i = 0; i < _nbins; ++i)
    _bin_center[i] = (i + 0.5) * _deltaV + _min_value;
}

void
VolumeHistogram::initialize()
{
  // reset the histogram
  _volume_tmp.assign(_nbins, 0.0);
}

void
VolumeHistogram::execute()
{
  // loop over quadrature points
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // compute target bin
    int bin = (_value[_qp] - _min_value) / _deltaV;

    // add the volume contributed by the current quadrature point
    if (bin >= 0 && bin < static_cast<int>(_nbins))
      _volume_tmp[bin] += computeVolume();
  }
}

void
VolumeHistogram::finalize()
{
  gatherSum(_volume_tmp);

  // copy into the MOOSE administered vector postprocessor vector
  _volume.resize(_nbins);
  mooseAssert(_volume_tmp.size() == _nbins, "Inconsistent volume vector lengths.");
  for (auto i = beginIndex(_volume_tmp); i < _volume_tmp.size(); ++i)
    _volume[i] = _volume_tmp[i];
}

void
VolumeHistogram::threadJoin(const UserObject & y)
{
  const VolumeHistogram & uo = static_cast<const VolumeHistogram &>(y);
  mooseAssert(uo._volume_tmp.size() == _volume_tmp.size(), "Inconsistent volume vector lengths across threads.");

  for (auto i = beginIndex(_volume_tmp); i < _volume_tmp.size(); ++i)
    _volume_tmp[i] += uo._volume_tmp[i];
}

Real
VolumeHistogram::computeVolume()
{
  // overwrite this method to multiply with phase fraction order parameters etc.
  return _JxW[_qp] * _coord[_qp];
}
