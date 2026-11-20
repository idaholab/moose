//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesPhysicsBase.h"
#include "INSFVRhieChowInterpolator.h"
#include "NSFVUtils.h"
#include "RelationshipManager.h"
#include "WCNSFVFlowPhysics.h"

InputParameters
NavierStokesPhysicsBase::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription(
      "Base class to define the Navier Stokes incompressible and weakly-compressible equation");

  params.addParam<bool>(
      "define_variables",
      true,
      "Whether to define variables if the variables with the specified names do not exist. Note "
      "that if the variables are defined externally from the Physics, the initial conditions will "
      "not be created in the Physics either.");

  params.addParam<unsigned short>(
      "ghost_layers", 2, "Number of layers of elements to ghost near process domain boundaries");
  params.addParamNamesToGroup("define_variables ghost_layers", "Advanced");

  return params;
}

NavierStokesPhysicsBase::NavierStokesPhysicsBase(const InputParameters & parameters)
  : PhysicsBase(parameters), _define_variables(getParam<bool>("define_variables"))
{
}

void
NavierStokesPhysicsBase::addFVAdvectedInterpolationMethod(const MooseEnum & interpolation_method)
{
  const std::string method_name = interpolation_method;
  if (getProblem().hasFVInterpolationMethod(method_name))
    return;

  const auto method_type = NS::fvAdvectedInterpolationMethodType(interpolation_method);

  InputParameters params = getFactory().getValidParams(method_type);
  getProblem().addFVInterpolationMethod(method_type, method_name, params);
}

MooseFunctorName
NavierStokesPhysicsBase::addPorosityScaledFunctor(const MooseFunctorName & porosity,
                                                  const MooseFunctorName & functor)
{
  const MooseFunctorName product_name = prefix() + porosity + "_times_" + functor;
  if (getProblem().hasFunctor(product_name, /*thread_id=*/0))
    return product_name;

  // Numbers go directly in the expression. Functors get symbols because their names, such as
  // the turbulent diffusivity 'mu_t/Sc_t', need not be valid parser identifiers.
  std::vector<std::string> functor_names;
  std::vector<std::string> functor_symbols;
  std::vector<std::string> factors;
  for (const auto & name : {porosity, functor})
    if (MooseUtils::parsesToReal(name))
      factors.push_back(name);
    else
    {
      functor_names.push_back(name);
      functor_symbols.push_back("f" + std::to_string(functor_symbols.size()));
      factors.push_back(functor_symbols.back());
    }

  const std::string object_type = "ParsedFunctorMaterial";
  InputParameters params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  params.set<std::string>("expression") = factors[0] + " * " + factors[1];
  params.set<std::vector<std::string>>("functor_names") = functor_names;
  params.set<std::vector<std::string>>("functor_symbols") = functor_symbols;
  params.set<std::string>("property_name") = product_name;
  getProblem().addMaterial(object_type, product_name + "_material", params);

  return product_name;
}

InputParameters
NavierStokesPhysicsBase::getAdditionalRMParams() const
{
  unsigned short necessary_layers = getParam<unsigned short>("ghost_layers");
  necessary_layers = std::max(necessary_layers, getNumberAlgebraicGhostingLayersNeeded());

  // Just an object that has a ghost_layers parameter
  const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.template set<unsigned short>("ghost_layers") = necessary_layers;

  return params;
}
