//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CombinedScalarDamage.h"

registerMooseObject("TensorMechanicsApp", CombinedScalarDamage);
registerMooseObject("TensorMechanicsApp", ADCombinedScalarDamage);

template <bool is_ad>
InputParameters
CombinedScalarDamageTempl<is_ad>::validParams()
{
  InputParameters params = ScalarDamageBaseTempl<is_ad>::validParams();

  params.addClassDescription(
      "Scalar damage model which is computed as a function of multiple scalar damage models");

  params.addRequiredParam<std::vector<MaterialName>>("damage_models",
                                                     "Name of the damage models used to compute "
                                                     "the damage index");

  MooseEnum combination_type("Maximum Product", "Maximum");
  params.addParam<MooseEnum>(
      "combination_type", combination_type, "How the damage models are combined");

  return params;
}

template <bool is_ad>
CombinedScalarDamageTempl<is_ad>::CombinedScalarDamageTempl(const InputParameters & parameters)
  : ScalarDamageBaseTempl<is_ad>(parameters),
    _combination_type(
        this->template getParam<MooseEnum>("combination_type").template getEnum<CombinationType>()),
    _damage_models_names(this->template getParam<std::vector<MaterialName>>("damage_models"))
{
}

template <bool is_ad>
void
CombinedScalarDamageTempl<is_ad>::initialSetup()
{
  for (unsigned int i = 0; i < _damage_models_names.size(); ++i)
  {
    ScalarDamageBaseTempl<is_ad> * model = dynamic_cast<ScalarDamageBaseTempl<is_ad> *>(
        &this->getMaterialByName(_damage_models_names[i]));

    if (model)
      _damage_models.push_back(model);
    else
      this->template paramError("damage_model",
                                "Damage Model " + _damage_models_names[i] +
                                    " is not compatible with CombinedScalarDamage");
  }
}

template <bool is_ad>
void
CombinedScalarDamageTempl<is_ad>::updateQpDamageIndex()
{
  switch (_combination_type)
  {
    case CombinationType::Maximum:
      _damage_index[_qp] = _damage_index_old[_qp];
      for (unsigned int i = 0; i < _damage_models.size(); ++i)
        _damage_index[_qp] = std::max(_damage_index[_qp], _damage_models[i]->getQpDamageIndex(_qp));
      break;
    case CombinationType::Product:
      _damage_index[_qp] = 1.0;
      for (unsigned int i = 0; i < _damage_models.size(); ++i)
        _damage_index[_qp] *= 1.0 - _damage_models[i]->getQpDamageIndex(_qp);
      _damage_index[_qp] = 1.0 - _damage_index[_qp];
      break;
  }

  _damage_index[_qp] =
      std::max(_damage_index_old[_qp], std::max(0.0, std::min(1.0, _damage_index[_qp])));
}
