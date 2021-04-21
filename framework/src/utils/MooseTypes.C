//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"
#include "libmesh/elem.h"
#include "libmesh/libmesh.h"

namespace Moose
{
const processor_id_type INVALID_PROCESSOR_ID = libMesh::DofObject::invalid_processor_id;
const SubdomainID ANY_BLOCK_ID = libMesh::Elem::invalid_subdomain_id - 1;
const SubdomainID INTERNAL_SIDE_LOWERD_ID = libMesh::Elem::invalid_subdomain_id - 2;
const SubdomainID BOUNDARY_SIDE_LOWERD_ID = libMesh::Elem::invalid_subdomain_id - 3;
const SubdomainID INVALID_BLOCK_ID = libMesh::Elem::invalid_subdomain_id;
const BoundaryID ANY_BOUNDARY_ID = static_cast<BoundaryID>(-1);
const BoundaryID INVALID_BOUNDARY_ID = libMesh::BoundaryInfo::invalid_id;
const TagID INVALID_TAG_ID = static_cast<TagID>(-1);
const TagTypeID INVALID_TAG_TYPE_ID = static_cast<TagTypeID>(-1);
const TagName SOLUTION_TAG = "SOLUTION";
const TagName OLD_SOLUTION_TAG = "SOLUTION_STATE_1";
const TagName OLDER_SOLUTION_TAG = "SOLUTION_STATE_2";
const TagName PREVIOUS_NL_SOLUTION_TAG = "U_PREVIOUS_NL_NEWTON";
}
