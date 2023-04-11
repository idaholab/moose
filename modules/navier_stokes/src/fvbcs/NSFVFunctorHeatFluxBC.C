//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVFunctorHeatFluxBC.h"
#include "NS.h"
#include "NSEnums.h"

registerADMooseObject("NavierStokesApp", NSFVFunctorHeatFluxBC);

InputParameters
NSFVFunctorHeatFluxBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();

  params.addRequiredParam<Real>("value", "total heat flux");
  params.addRequiredParam<MooseEnum>(
      "phase", getPhaseEnum(), "'fluid' or 'solid' phase to which this BC is applied.");

  params.addRequiredParam<MooseEnum>("splitting", getSplittingEnum(), "type of splitting");

  params.addRequiredParam<MooseEnum>(
      "locality",
      getLocalityEnum(),
      "whether to use local (at the boundary) or global (domain-averaged) "
      "parameter values");

  params.addParam<MooseFunctorName>(NS::porosity, "porosity");
  params.addParam<PostprocessorName>("average_porosity",
                                     "postprocessor that provides domain-averaged proosity");

  // Name of the material properties, useful if not following the NS namespace conventions
  params.addParam<MooseFunctorName>(NS::k, NS::k, "Fluid phase thermal conductivity");
  params.addParam<MooseFunctorName>(NS::k_s, NS::k_s, "Solid phase thermal conductivity");
  params.addParam<MooseFunctorName>(
      NS::kappa, NS::kappa, "Fluid phase effective thermal conductivity");
  params.addParam<MooseFunctorName>(
      NS::kappa_s, NS::kappa_s, "Solid phase effective thermal conductivity");

  params.addParam<PostprocessorName>(
      "average_k_fluid", "postprocessor that provides domain-averaged fluid thermal conductivity");
  params.addParam<PostprocessorName>(
      "average_kappa", "postprocessor that provides domain-averaged fluid thermal dispersion");
  params.addParam<PostprocessorName>(
      "average_k_solid", "postprocessor that provides domain-averaged solid thermal conductivity");
  params.addParam<PostprocessorName>(
      "average_kappa_solid",
      "postprocessor that provides domain-averaged solid effective thermal "
      "conductivity");
  params.addClassDescription("Constant heat flux boundary condition with phase splitting "
                             "for fluid and solid energy equations");
  return params;
}

NSFVFunctorHeatFluxBC::NSFVFunctorHeatFluxBC(const InputParameters & parameters)
  : FVFluxBC(parameters),
    _value(getParam<Real>("value")),
    _phase(getParam<MooseEnum>("phase").getEnum<NS::phase::PhaseEnum>()),
    _split_type(getParam<MooseEnum>("splitting").getEnum<NS::splitting::SplittingEnum>()),
    _locality(getParam<MooseEnum>("locality").getEnum<NS::settings::LocalityEnum>()),
    // quantities needed for global evaluations
    _average_eps(_locality == NS::settings::global &&
                         _split_type != NS::splitting::thermal_conductivity
                     ? &getPostprocessorValue("average_porosity")
                     : nullptr),
    _average_k_f(_locality == NS::settings::global && _split_type != NS::splitting::porosity
                     ? &getPostprocessorValue("average_k_fluid")
                     : nullptr),
    _average_k_s(_locality == NS::settings::global &&
                         _split_type == NS::splitting::thermal_conductivity
                     ? &getPostprocessorValue("average_k_solid")
                     : nullptr),
    _average_kappa_s(_locality == NS::settings::global &&
                             _split_type == NS::splitting::effective_thermal_conductivity
                         ? &getPostprocessorValue("average_kappa_solid")
                         : nullptr),
    _average_kappa(_locality == NS::settings::global &&
                           _split_type == NS::splitting::effective_thermal_conductivity
                       ? &getPostprocessorValue("average_kappa")
                       : nullptr),
    // quantities needed for local evaluations
    _eps(_locality == NS::settings::local && _split_type != NS::splitting::thermal_conductivity
             ? &getFunctor<ADReal>(NS::porosity)
             : nullptr),
    _k_f(_locality == NS::settings::local && _split_type != NS::splitting::porosity
             ? &getFunctor<ADReal>(NS::k)
             : nullptr),
    _k_s(_locality == NS::settings::local && _split_type == NS::splitting::thermal_conductivity
             ? &getFunctor<ADReal>(NS::k_s)
             : nullptr),
    _kappa(_locality == NS::settings::local &&
                   _split_type == NS::splitting::effective_thermal_conductivity
               ? &getFunctor<ADRealVectorValue>(NS::kappa)
               : nullptr),
    _kappa_s(_locality == NS::settings::local &&
                     _split_type == NS::splitting::effective_thermal_conductivity
                 ? &getFunctor<ADReal>(NS::kappa_s)
                 : nullptr)
{
}

ADReal
NSFVFunctorHeatFluxBC::computeQpResidual()
{
  // we don't need default statements in the switch-case statements for
  // the SplittingEnum because we by default set the fraction to zero such that
  // all of the heat flux enters the solid phase (though this default is never
  // reached since the InputParameters class requires the MooseEnum to be
  // one of 'porosity', 'thermal_conductivity', or 'effective_thermal_conductivity'
  ADReal fraction = 0.0;
  ADReal tol = NS_DEFAULT_VALUES::k_epsilon;

  // Get the functor argument for the face
  const auto face_arg = singleSidedFaceArg();
  const auto state = determineState();

  if (_locality == NS::settings::local)
  {
    switch (_split_type)
    {
      case NS::splitting::porosity:
      {
        fraction = (*_eps)(face_arg, state);
        break;
      }
      case NS::splitting::thermal_conductivity:
      {
        ADReal d = (*_k_f)(face_arg, state) + (*_k_s)(face_arg, state);
        fraction = d > tol ? (*_k_f)(face_arg, state) / d : 0.5;
        break;
      }
      case NS::splitting::effective_thermal_conductivity:
      {
        // TODO: for the case of an anisotropic conductivity, we technically should
        // grab the component of kappa perpendicular to the boundary.

        // Need this logic to avoid AD failures when taking norms of zero. The
        // division by sqrt(3) ensures equivalence with a non-vector form of kappa with
        // 3 components
        ADReal kappa;
        if ((MooseUtils::absoluteFuzzyEqual((*_kappa)(face_arg, state)(0), 0)) &&
            (MooseUtils::absoluteFuzzyEqual((*_kappa)(face_arg, state)(1), 0)) &&
            (MooseUtils::absoluteFuzzyEqual((*_kappa)(face_arg, state)(2), 0)))
          kappa = 1e-42;
        else
          kappa = (*_kappa)(face_arg, state).norm() / std::sqrt(3.0);

        ADReal d = (*_eps)(face_arg, state) * kappa + (*_kappa_s)(face_arg, state);
        fraction = d > tol ? (*_eps)(face_arg, state) * kappa / d : 0.5;
        break;
      }
    }
  }
  else
  {
    switch (_split_type)
    {
      case NS::splitting::porosity:
      {
        fraction = *_average_eps;
        break;
      }
      case NS::splitting::thermal_conductivity:
      {
        ADReal d = *_average_k_f + *_average_k_s;
        fraction = d > tol ? *_average_k_f / d : 0.5;
        break;
      }
      case NS::splitting::effective_thermal_conductivity:
      {
        ADReal d = (*_average_eps) * (*_average_kappa) + (*_average_kappa_s);
        fraction = d > tol ? (*_average_eps) * (*_average_kappa) / d : 0.5;
        break;
      }
    }
  }

  if (_phase == NS::phase::solid)
    return (1.0 - fraction) * -_value;
  else
    return fraction * -_value;
}
