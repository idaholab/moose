/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ConservedMaskedNoiseBase.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<ConservedMaskedNoiseBase>()
{
  InputParameters params = validParams<ElementUserObject>();

  MultiMooseEnum setup_options(SetupInterface::getExecuteOptions());
  setup_options = "timestep_begin";
  params.set<MultiMooseEnum>("execute_on") = setup_options;
  params.addParam<MaterialPropertyName>("mask", "Material property to multiply the random numbers with");
  return params;
}

ConservedMaskedNoiseBase::ConservedMaskedNoiseBase(const InputParameters & parameters) :
    ConservedNoiseInterface(parameters),
    _mask(getMaterialProperty<Real>("mask"))
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
  std::vector<std::pair<Real, Real> > & me = _random_data[_current_elem->id()] = std::vector<std::pair<Real, Real> >(_qrule->n_points());

  // store a random number for each quadrature point
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    me[_qp].first = getQpRandom();
    me[_qp].second = _mask[_qp];
    _integral += _JxW[_qp] * _coord[_qp] * me[_qp].first * me[_qp].second;
    _volume   += _JxW[_qp] * _coord[_qp] * me[_qp].second;
  }
}

void
ConservedMaskedNoiseBase::threadJoin(const UserObject &y)
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
  LIBMESH_BEST_UNORDERED_MAP<dof_id_type, std::vector<std::pair<Real, Real> > >::const_iterator me = _random_data.find(element_id);

  if (me == _random_data.end())
    mooseError("Element not found.");
  else
  {
    libmesh_assert_less(qp, me->second.size());
    return (me->second[qp].first - _offset) * me->second[qp].second;
  }
}

