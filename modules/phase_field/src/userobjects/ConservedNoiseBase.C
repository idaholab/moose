//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservedNoiseBase.h"

#include "libmesh/quadrature.h"

InputParameters
ConservedNoiseBase::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;
  return params;
}

ConservedNoiseBase::ConservedNoiseBase(const InputParameters & parameters)
  : ConservedNoiseInterface(parameters)
{
}

void
ConservedNoiseBase::initialize()
{
  _random_data.clear();
  _integral = 0.0;
  _volume = 0.0;
}

void
ConservedNoiseBase::execute()
{
  // reserve space for each quadrature point in the element
  std::vector<Real> & me = _random_data[_current_elem->id()] =
      std::vector<Real>(_qrule->n_points());

  // store a random number for each quadrature point
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    me[_qp] = getQpRandom();
    _integral += _JxW[_qp] * _coord[_qp] * me[_qp];
    _volume += _JxW[_qp] * _coord[_qp];
  }
}

void
ConservedNoiseBase::threadJoin(const UserObject & y)
{
  const ConservedNoiseBase & uo = static_cast<const ConservedNoiseBase &>(y);

  _random_data.insert(uo._random_data.begin(), uo._random_data.end());
  _integral += uo._integral;
  _volume += uo._volume;
}

void
ConservedNoiseBase::finalize()
{
  gatherSum(_integral);
  gatherSum(_volume);

  _offset = _integral / _volume;
}

Real
ConservedNoiseBase::getQpValue(dof_id_type element_id, unsigned int qp) const
{
  const auto it_pair = _random_data.find(element_id);

  if (it_pair == _random_data.end())
    mooseError("Element not found.");
  else
  {
    libmesh_assert_less(qp, it_pair->second.size());
    return it_pair->second[qp] - _offset;
  }
}
