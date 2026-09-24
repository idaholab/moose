//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNEML2BatchLayout.h"

namespace Moose::Kokkos
{

void
NEML2BatchLayout::build(const Assembly & assembly,
                        const Mesh & mesh,
                        const std::set<SubdomainID> & blocks)
{
  _subdomain_offset.create(mesh.getNumSubdomains());
  _subdomain_offset = libMesh::DofObject::invalid_id;

  _size = 0;

  // Accumulate in contiguous subdomain ID order so that the batch and the material property storage
  // it mirrors are ordered the same way
  std::set<ContiguousSubdomainID> contiguous;
  for (const auto subdomain : blocks)
    contiguous.insert(mesh.getContiguousSubdomainID(subdomain));

  for (const auto subdomain : contiguous)
  {
    _subdomain_offset[subdomain] = _size;
    _size += assembly.getNumQps(subdomain);
  }

  _subdomain_offset.copyToDevice();

  _built = true;
}

} // namespace Moose::Kokkos
