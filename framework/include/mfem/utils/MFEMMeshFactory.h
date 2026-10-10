//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

#include <memory>

class MooseMesh;

namespace Moose::MFEM
{
/**
 * Returns an MFEM mesh object corresponding to the provided MooseMesh argument. If the argument
 * is an MFEMMesh then the MFEM mesh it contains is returned. Otherwise the MFEM mesh is
 * constructed from the libMesh data.
 *
 * The boolean arguments only have an effect if the mesh is not already an MFEMMesh. In that case,
 * \p fallback indicates whether element types which can not be represented in MFEM should be
 * represented by simpler ones which can be, and \p first_order indicates whether the mesh should
 * be converted to have only first-order elements.
 */
std::shared_ptr<mfem::ParMesh> buildMFEMMesh(MooseMesh & mesh, bool fallback, bool first_order);
}

#endif
