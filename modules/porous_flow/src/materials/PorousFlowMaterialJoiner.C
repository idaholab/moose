/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorousFlowMaterialJoiner.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorousFlowMaterialJoiner>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredParam<UserObjectName>("PorousFlowDictator_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addRequiredParam<std::string>("material_property", "The property that you want joined into a std::vector.  Old values are not considered.");
  params.addClassDescription("This Material forms a std::vectors of properties and derivatives (but not Old values) out of the individual phase properties");
  return params;
}

PorousFlowMaterialJoiner::PorousFlowMaterialJoiner(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _porflow_name_UO(getUserObject<PorousFlowDictator>("PorousFlowDictator_UO")),
    _num_phases(_porflow_name_UO.num_phases()),
    _pf_prop(getParam<std::string>("material_property")),

    _property(declareProperty<std::vector<Real> >(_pf_prop)),
    _dproperty_dvar(declareProperty<std::vector<std::vector<Real> > >("d" + _pf_prop + "_dvar"))
{
  _phase_property.resize(_num_phases);
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _phase_property[ph] = &getMaterialProperty<Real>(_pf_prop + Moose::stringify(ph));

  _dphase_property_dvar.resize(_num_phases);
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _dphase_property_dvar[ph] = &getMaterialProperty<std::vector<Real> >("d" + _pf_prop + Moose::stringify(ph) + "_dvar");
}

void
PorousFlowMaterialJoiner::initQpStatefulProperties()
{
}

void
PorousFlowMaterialJoiner::computeQpProperties()
{
  _property[_qp].resize(_num_phases);
  _dproperty_dvar[_qp].resize(_num_phases);
  const unsigned int num_var = _porflow_name_UO.num_v();
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _dproperty_dvar[_qp][ph].resize(num_var);

  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _property[_qp][ph] = (*_phase_property[ph])[_qp];
    for (unsigned v = 0; v < num_var; ++v)
      _dproperty_dvar[_qp][ph][v] = 1; //(*_dphase_property_dvar[ph])[_qp][v];
  }
}

