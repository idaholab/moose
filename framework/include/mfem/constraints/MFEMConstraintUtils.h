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
 * Degrees of freedom of @p pfes belonging to elements whose mesh attribute is set
 * in the attribute marker array @p subdomain_markers, as returned by
 * MFEMBlockRestrictable::getSubdomainMarkers. If @p include_boundary is false, the
 * dofs on the boundary of the subdomains - those shared with any other element and
 * those on the exterior boundary of the mesh - are left out.
 *
 * @p dof_marker is set to a local-dof (L-dof) marker of the selected dofs and
 * @p tdofs to the matching true-dof indices; both are overwritten. Both are
 * parallel-consistent: a dof shared across a processor boundary that coincides with
 * the subdomain boundary is selected on every rank that touches it, even a rank
 * whose own elements do not carry the attribute.
 */
void subdomainDofs(mfem::ParFiniteElementSpace & pfes,
                   const mfem::Array<int> & subdomain_markers,
                   bool include_boundary,
                   mfem::Array<int> & dof_marker,
                   mfem::Array<int> & tdofs);

/**
 * Overwrite the degrees of freedom of grid function @p gf marked in the L-dof
 * marker @p dof_marker with the projection of coefficient @p coef over the
 * elements whose mesh attribute is set in the attribute marker array
 * @p subdomain_markers. Every marked dof must belong to such an element. The
 * projected values are averaged across all elements and ranks contributing to a
 * shared dof, so they agree on every rank including the one that owns the dof.
 * @p coef must have the same number of components as the space of @p gf.
 *
 * The vector overload works for vector H1 as well as H(curl)/H(div) spaces.
 */
void projectCoefficientOnSubdomains(mfem::ParGridFunction & gf,
                                    mfem::Coefficient & coef,
                                    const mfem::Array<int> & subdomain_markers,
                                    const mfem::Array<int> & dof_marker);
void projectCoefficientOnSubdomains(mfem::ParGridFunction & gf,
                                    mfem::VectorCoefficient & coef,
                                    const mfem::Array<int> & subdomain_markers,
                                    const mfem::Array<int> & dof_marker);

/**
 * Strongly set the true dofs listed in @p tdofs of grid function @p gf to zero,
 * leaving its local vector consistent with the true vector.
 */
void zeroTrueDofs(mfem::ParGridFunction & gf, const mfem::Array<int> & tdofs);
}

#endif
