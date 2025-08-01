//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVMixtureFunctorMaterialTempl.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", WCNSLinearFVMixtureFunctorMaterial);
registerMooseObject("NavierStokesApp", NSFVMixtureFunctorMaterial);
registerMooseObjectRenamed("NavierStokesApp",
                           NSFVMixtureMaterial,
                           "08/01/2024 00:00",
                           NSFVMixtureFunctorMaterial);

template <bool is_ad>
InputParameters
NSFVMixtureFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addClassDescription(
      "Compute the arithmetic mean of material properties using a phase fraction.");
  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "phase_1_names", "The names of the properties for phase 1.");
  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "phase_2_names", "The names of the properties for phase 2.");
  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "prop_names", "The name of the mixture properties output from the material.");
  params.addRequiredParam<MooseFunctorName>("phase_1_fraction", "Phase 1 fraction.");
  params.addParam<bool>(
      "limit_phase_fraction", false, "Whether to bound the phase fraction between 0 and 1");
  return params;
}

template <bool is_ad>
NSFVMixtureFunctorMaterialTempl<is_ad>::NSFVMixtureFunctorMaterialTempl(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _phase_1_names(getParam<std::vector<MooseFunctorName>>("phase_1_names")),
    _phase_2_names(getParam<std::vector<MooseFunctorName>>("phase_2_names")),
    _mixture_names(getParam<std::vector<MooseFunctorName>>("prop_names")),
    _phase_1_fraction(getFunctor<GenericReal<is_ad>>("phase_1_fraction")),
    _limit_pf(getParam<bool>("limit_phase_fraction"))
{

  if (_phase_1_names.size() != _phase_2_names.size())
    paramError("phase_2_names", "Phase 1 and Phase 2 names do not have the same length.");

  if (_phase_1_names.size() != _mixture_names.size())
    paramError("prop_names", "Phase 1 and mixture property names do not have the same length.");

  for (const auto prop_index : index_range(_phase_1_names))
  {
    _phase_1_properties.push_back(&getFunctor<GenericReal<is_ad>>(_phase_1_names[prop_index]));
    _phase_2_properties.push_back(&getFunctor<GenericReal<is_ad>>(_phase_2_names[prop_index]));

    addFunctorProperty<GenericReal<is_ad>>(
        _mixture_names[prop_index],
        [this, prop_index](const auto & r, const auto & t) -> GenericReal<is_ad>
        {
          // Avoid messing up the fluid properties, but keep the same dependencies
          // AD-version needs to preserve sparsity pattern
          const auto phase_1_fraction =
              _limit_pf
                  ? (is_ad ? std::max(std::min(_phase_1_fraction(r, t), (GenericReal<is_ad>)1),
                                      (GenericReal<is_ad>)0) +
                                 0 * _phase_1_fraction(r, t)
                           : std::max(std::min(_phase_1_fraction(r, t), (GenericReal<is_ad>)1),
                                      (GenericReal<is_ad>)0))
                  : _phase_1_fraction(r, t);
          return phase_1_fraction * (*_phase_1_properties[prop_index])(r, t) +
                 (1.0 - phase_1_fraction) * (*_phase_2_properties[prop_index])(r, t);
        });
  }
}

template class NSFVMixtureFunctorMaterialTempl<false>;
template class NSFVMixtureFunctorMaterialTempl<true>;
