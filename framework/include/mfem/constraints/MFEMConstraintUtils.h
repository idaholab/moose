//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mfem-common.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{
/**
 * True-dof indices of @p pfes belonging to elements whose mesh attribute appears
 * in @p subdomain_attrs. Parallel-consistent: a dof shared across a processor
 * boundary that coincides with the subdomain boundary is listed on every rank
 * that touches it, even a rank whose own elements do not carry the attribute.
 * @p tdofs is overwritten. An empty @p subdomain_attrs means every subdomain,
 * matching the documented meaning of an empty 'block' parameter.
 */
void subdomainTrueDofs(mfem::ParFiniteElementSpace & pfes,
                       const mfem::Array<int> & subdomain_attrs,
                       mfem::Array<int> & tdofs);

/**
 * Overwrite the degrees of freedom of grid function @p gf that lie in elements
 * whose mesh attribute appears in @p subdomain_attrs with the projection of
 * coefficient @p coef. An empty @p subdomain_attrs means every subdomain. The
 * projected values are averaged across all ranks contributing to a shared dof, so
 * they agree on every rank including the one that owns the dof.
 *
 * The vector overload works for vector H1 as well as H(curl)/H(div) spaces.
 */
void projectCoefficientOnSubdomains(mfem::ParGridFunction & gf,
                                    mfem::Coefficient & coef,
                                    const mfem::Array<int> & subdomain_attrs);
void projectCoefficientOnSubdomains(mfem::ParGridFunction & gf,
                                    mfem::VectorCoefficient & coef,
                                    const mfem::Array<int> & subdomain_attrs);

/**
 * Strongly set the true dofs listed in @p tdofs of grid function @p gf to zero,
 * leaving its local vector consistent with the true vector.
 */
void zeroTrueDofs(mfem::ParGridFunction & gf, const mfem::Array<int> & tdofs);
}

#endif
