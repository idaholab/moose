//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// This is the minimal set of headers to make all the types below that cannot be
// forward-declared as classes available.
// If we want to reduce this list, we would have to forward-declare functions as well
#include "libmesh/libmesh.h"
#include "libmesh/id_types.h"
#include "libmesh/simple_range.h"
#include "libmesh/int_range.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/elem_range.h"
#include "libmesh/node_range.h"
#include "libmesh/remote_elem.h"

// Added for convenience
#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_fe_family.h"
#include "libmesh/enum_order.h"

// needed to print print--trace.h without triggering precheck
#include "MooseError.h"

// README
// If you hit a compilation error that the XYZ class shown here is not defined, and it is declared
// in this header then you forgot to include the libMesh header in your header/source that actually
// declared the class. We only forward declare it here.

// We want all the types below to be available everywhere
// Element types
namespace libMesh
{
class Edge;
class Edge2;
class Edge3;
class Edge4;
class Hex;
class Hex8;
class Hex20;
class Hex27;
class Prism;
class Prism15;
class Prism18;
class Prism21;
class Prism6;
class Pyramid;
class Pyramid5;
class Pyramid13;
class Pyramid14;
class Quad;
class Quad4;
class Quad8;
class Quad9;
class Tet;
class Tet4;
class Tet10;
class Tet14;
class Tri;
class Tri3;
class Tri6;
class Tri7;
class RemoteElem;
}

using libMesh::Edge;
using libMesh::Edge2;
using libMesh::EDGE2;
using libMesh::Edge3;
using libMesh::EDGE3;
using libMesh::Edge4;
using libMesh::EDGE4;
using libMesh::ElemType;
using libMesh::Hex;
using libMesh::Hex20;
using libMesh::HEX20;
using libMesh::Hex27;
using libMesh::HEX27;
using libMesh::Hex8;
using libMesh::HEX8;
using libMesh::Prism;
using libMesh::Prism15;
using libMesh::PRISM15;
using libMesh::Prism18;
using libMesh::PRISM18;
using libMesh::Prism21;
using libMesh::PRISM21;
using libMesh::Prism6;
using libMesh::PRISM6;
using libMesh::Pyramid;
using libMesh::PYRAMID13;
using libMesh::PYRAMID14;
using libMesh::PYRAMID5;
using libMesh::Quad;
using libMesh::Quad4;
using libMesh::QUAD4;
using libMesh::Quad8;
using libMesh::QUAD8;
using libMesh::Quad9;
using libMesh::QUAD9;
using libMesh::remote_elem;
using libMesh::RemoteElem;
using libMesh::Tet;
using libMesh::Tet10;
using libMesh::TET10;
using libMesh::Tet14;
using libMesh::TET14;
using libMesh::Tet4;
using libMesh::TET4;
using libMesh::Tri;
using libMesh::Tri3;
using libMesh::TRI3;
using libMesh::TRI6;
using libMesh::TRI7;

// Ranges
namespace libMesh
{
template <typename T, typename P>
class StoredRange;
}

using libMesh::ConstElemRange;
using libMesh::ConstNodeRange;
using libMesh::NodeRange;
using libMesh::StoredRange;

// Mesh classes
namespace libMesh
{
class DistributedMesh;
class ReplicatedMesh;
class UnstructuredMesh;
class MeshBase;
class BoundingBox;
}

using libMesh::BoundingBox;
using libMesh::DistributedMesh;
using libMesh::MeshBase;
using libMesh::ReplicatedMesh;
using libMesh::UnstructuredMesh;

// Solver classes
namespace libMesh
{
class Partitioner;
class DofObject;
class OrderWrapper;
class PetscSolverException;
}
using libMesh::DofObject;
using libMesh::OrderWrapper;
using libMesh::Partitioner;
using libMesh::PetscSolverException;

// Common data types
using libMesh::boundary_id_type;
using libMesh::dof_id_type;
using libMesh::Number;
using libMesh::numeric_index_type;
using libMesh::processor_id_type;
using libMesh::Real;
using libMesh::RealTensor;
using libMesh::subdomain_id_type;
using libMesh::unique_id_type;

// Common templated types
namespace libMesh
{
template <typename T>
class NumericVector;
template <typename T>
class SparseMatrix;
template <typename T>
class TensorValue;
template <typename>
class TypeVector;
template <unsigned int, typename>
class TypeNTensor;
template <typename T>
class TypeTensor;
template <typename T>
class VectorValue;
}

using libMesh::NumericVector;
using libMesh::SparseMatrix;
using libMesh::TensorValue;
using libMesh::TypeNTensor;
using libMesh::TypeTensor;
using libMesh::TypeVector;
using libMesh::VectorValue;

// Common FE families
using libMesh::L2_HIERARCHIC;
using libMesh::L2_LAGRANGE;
using libMesh::LAGRANGE;
using libMesh::MONOMIAL;
using libMesh::SCALAR;

// Counting from 0 to 20
using libMesh::CONSTANT;
using libMesh::EIGHTEENTH;
using libMesh::EIGHTH;
using libMesh::ELEVENTH;
using libMesh::FIFTEENTH;
using libMesh::FIFTH;
using libMesh::FIRST;
using libMesh::FOURTEENTH;
using libMesh::FOURTH;
using libMesh::INVALID_ORDER;
using libMesh::NINETEENTH;
using libMesh::NINTH;
using libMesh::Order;
using libMesh::SECOND;
using libMesh::SEVENTEENTH;
using libMesh::SEVENTH;
using libMesh::SIXTEENTH;
using libMesh::SIXTH;
using libMesh::TENTH;
using libMesh::THIRD;
using libMesh::THIRTEENTH;
using libMesh::TWELFTH;
using libMesh::TWENTIETH;

// Common mesh classes
namespace libMesh
{
class BoundaryInfo;
class Elem;
class Node;
class Point;
}

using libMesh::BoundaryInfo;
using libMesh::Elem;
using libMesh::Node;
using libMesh::Point;

// Common utilities
using libMesh::as_range;
using libMesh::cast_int;
using libMesh::cast_ptr;
using libMesh::cast_ref;
using libMesh::demangle;
using libMesh::index_range;
using libMesh::libmesh_ignore;
using libMesh::make_range;

// Common defaults
using libMesh::invalid_uint;
using libMesh::TOLERANCE;

// Declare all the nested namespaces we use in MOOSE without prefacing with
// libMesh::
namespace libMesh
{
namespace MeshTools
{
}
namespace Predicates
{
}
namespace TensorTools
{
}
namespace Threads
{
}
namespace Utility
{
}
namespace Parallel
{
}
}
namespace MeshTools = libMesh::MeshTools;
namespace Predicates = libMesh::Predicates;
namespace TensorTools = libMesh::TensorTools;
namespace Threads = libMesh::Threads;
namespace Utility = libMesh::Utility;
namespace Parallel = libMesh::Parallel;
