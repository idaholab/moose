//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarContactPressureProvider.h"
#include "MortarUserObject.h"

#include "libmesh/elem.h"

MortarContactPressureProvider::MortarContactPressureProvider(
    const MortarUserObject & provider_object)
  : _provider_object(provider_object)
{
}

ADReal
MortarContactPressureProvider::qpContactPressure(const Elem & lower_secondary_elem,
                                                 const unsigned int qp) const
{
  const auto & basis = tractionBasis();
  ADReal pressure = 0;
  for (const auto j : index_range(basis))
    pressure += basis[j][qp] * nodalContactPressure(lower_secondary_elem.node_ref(j));
  return pressure;
}

void
MortarContactPressureProvider::checkConsumer(const MooseObject & consumer,
                                             const std::string & param,
                                             const SubdomainID primary_subdomain,
                                             const SubdomainID secondary_subdomain,
                                             const bool use_displaced_mesh) const
{
  if (primary_subdomain != _provider_object.primarySubdomain() ||
      secondary_subdomain != _provider_object.secondarySubdomain())
    consumer.paramError(param,
                        "The contact pressure provider '",
                        _provider_object.name(),
                        "' must be defined on the same primary/secondary subdomain pair as '",
                        consumer.name(),
                        "'.");

  if (use_displaced_mesh != _provider_object.getParam<bool>("use_displaced_mesh"))
    consumer.paramError(param,
                        "The contact pressure provider '",
                        _provider_object.name(),
                        "' must use the same 'use_displaced_mesh' setting as '",
                        consumer.name(),
                        "'.");
}
