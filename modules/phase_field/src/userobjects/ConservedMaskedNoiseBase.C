//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservedMaskedNoiseBase.h"

#include "libmesh/quadrature.h"

InputParameters
ConservedMaskedNoiseBase::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  params.addParam<MaterialPropertyName>("mask",
                                        "Material property to multiply the random numbers with");
  return params;
}

ConservedMaskedNoiseBase::ConservedMaskedNoiseBase(const InputParameters & parameters)
  : ConservedNoiseInterface(parameters), _mask(getMaterialProperty<Real>("mask"))
{
}

void
ConservedMaskedNoiseBase::initialize()
{
  _random_data.clear();
  _integral = 0.0;
  _volume = 0.0;
}

void
ConservedMaskedNoiseBase::execute()
{
  // reserve space for each quadrature point in the element
  std::vector<std::pair<Real, Real>> & me = _random_data[_current_elem->id()] =
      std::vector<std::pair<Real, Real>>(_qrule->n_points());

  // store a random number for each quadrature point
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    me[_qp].first = getQpRandom();
    me[_qp].second = _mask[_qp];
    _integral += _JxW[_qp] * _coord[_qp] * me[_qp].first * me[_qp].second;
    _volume += _JxW[_qp] * _coord[_qp] * me[_qp].second;
  }
}

void
ConservedMaskedNoiseBase::threadJoin(const UserObject & y)
{
  const ConservedMaskedNoiseBase & uo = static_cast<const ConservedMaskedNoiseBase &>(y);

  _random_data.insert(uo._random_data.begin(), uo._random_data.end());
  _integral += uo._integral;
  _volume += uo._volume;
}

void
ConservedMaskedNoiseBase::finalize()
{
  gatherSum(_integral);
  gatherSum(_volume);

  // TODO check that _volume is >0
  _offset = _integral / _volume;
}

Real
ConservedMaskedNoiseBase::getQpValue(dof_id_type element_id, unsigned int qp) const
{
  const auto it_pair = _random_data.find(element_id);

  if (it_pair == _random_data.end())
    mooseError("Element not found.");
  else
  {
    libmesh_assert_less(qp, it_pair->second.size());
    return (it_pair->second[qp].first - _offset) * it_pair->second[qp].second;
  }
}
