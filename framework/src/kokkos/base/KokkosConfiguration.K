//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConfiguration.h"
#include "KokkosVariableValue.h"

#include "Coupleable.h"
#include "InputParameters.h"

namespace Moose::Kokkos
{

void
addConfigurationParams(InputParameters & params)
{
  const std::string doc = "Whether the object is posed on the current configuration given by the "
                          "displacements of the [Mesh] block. Kokkos objects integrate on the "
                          "reference mesh and compose the deformation into their geometry.";

  // Bases that already declare the parameter keep their group placement
  if (params.have_parameter<bool>("use_displaced_mesh"))
  {
    params.set<bool>("use_displaced_mesh") = false;
    params.setDocString("use_displaced_mesh", doc);
  }
  else
  {
    params.addParam<bool>("use_displaced_mesh", false, doc);
    params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  }

  params.addCoupledVar("_displacements", "The displacements of the current configuration");
  params.suppressParameter<std::vector<VariableName>>("_displacements");
}

Configuration::Configuration(const Coupleable & object)
  : _displacements(
        object.kokkosCoupledVectorTagValuesByName("_displacements", Moose::SOLUTION_TAG).variable())
{
}

} // namespace Moose::Kokkos
