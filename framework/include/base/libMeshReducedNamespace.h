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

// These headers add constants that we want to add to the namespace.
#include "libmesh/libmesh.h"
#include "libmesh/remote_elem.h"

// This header declares functions that we want to add to the namespace. If we chose not to include
// this header, we will need to repeat the function declaration here
#include "libmesh/int_range.h" // for make_range + several typedefs

// These headers introduce typedefs that we are adding to the local namespace. If we chose not
// to include these headers, we will need to repeat the typedef here
#include "libmesh/id_types.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"

// These are required for the range typedefs they contain, which cannot be forward-declared
// without re-defining the typedef. This allows us to add the ranges in the local namespace
// but at the cost of defining them, including very many headers
#include "libmesh/elem_range.h"
#include "libmesh/node_range.h"

// Added for convenience
#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_fe_family.h"
#include "libmesh/enum_order.h"
#include "libmesh/enum_parallel_type.h"
#include "libmesh/enum_point_locator_type.h"

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
using libMesh::NODEELEM;
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
using libMesh::Pyramid13;
using libMesh::PYRAMID13;
using libMesh::Pyramid14;
using libMesh::PYRAMID14;
using libMesh::Pyramid5;
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
using libMesh::Tri6;
using libMesh::TRI6;
using libMesh::Tri7;
using libMesh::TRI7;

// Continuity types
using libMesh::C_ONE;
using libMesh::C_ZERO;
using libMesh::DISCONTINUOUS;
using libMesh::H_CURL;
using libMesh::H_DIV;
using libMesh::SIDE_DISCONTINUOUS;

// Finite element helper classes
namespace libMesh
{
class FEType;
class FEInterface;
class FEMap;
template <typename T>
class FEGenericBase;
// "Forward-declaring" typedefs must match exactly how they were defined
typedef FEGenericBase<Real> FEBase;
typedef FEGenericBase<RealGradient> FEVectorBase;
}

using libMesh::FEBase;
using libMesh::FEGenericBase;
using libMesh::FEInterface;
using libMesh::FEMap;
using libMesh::FEType;
using libMesh::FEVectorBase;

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
using libMesh::HIERARCHIC;
using libMesh::L2_HIERARCHIC;
using libMesh::L2_LAGRANGE;
using libMesh::LAGRANGE;
using libMesh::LAGRANGE_VEC;
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

// Parallel types
using libMesh::AUTOMATIC;
using libMesh::GHOSTED;
using libMesh::PARALLEL;
using libMesh::SERIAL;

// Point locator types
using libMesh::NANOFLANN;
using libMesh::TREE;
using libMesh::TREE_ELEMENTS;
using libMesh::TREE_LOCAL_ELEMENTS;

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

// Added to pass downstream app tests
// All these are all already prefaced with 'libMesh::' in the framework and modules
#include "libmesh/enum_norm_type.h"
using libMesh::DISCRETE_L2;

using libMesh::pi;
using libMesh::zero;

namespace libMesh
{
class DoFMap;
class PerfLog;
class System;
class ExplicitSystem;
class ImplicitSystem;
class LinearImplicitSystem;
class NonlinearImplicitSystem;
class EigenSystem;
class ExodusII_IO;
class PointLocatorBase;
enum FEFamily : int;
enum QuadratureType : int;
template <typename T>
struct ScalarTraits;
class Mesh;
class MeshSerializer;
class Poly2TriTriangulator;
template <typename T>
class SimpleRange;
template <typename Output, typename OutputGradient>
class ParsedFunction;
template <typename T>
class PetscVector;
template <typename T>
class PetscMatrix;
class PeriodicBoundaries;
class QTrap;
class QGauss;
class TriangulatorInterface;
template <typename T>
class Preconditioner;
enum PreconditionerType : int;
template <typename T>
class ReferenceCountedObject;
struct SyncElementIntegers;
}
using libMesh::DoFMap;
using libMesh::EigenSystem;
using libMesh::ExodusII_IO;
using libMesh::ExplicitSystem;
using libMesh::FEFamily;
using libMesh::ImplicitSystem;
using libMesh::LinearImplicitSystem;
using libMesh::Mesh;
using libMesh::MeshSerializer;
using libMesh::NonlinearImplicitSystem;
using libMesh::ParsedFunction;
using libMesh::PerfLog;
using libMesh::PeriodicBoundaries;
using libMesh::PetscMatrix;
using libMesh::PetscVector;
using libMesh::PointLocatorBase;
using libMesh::Poly2TriTriangulator;
using libMesh::Preconditioner;
using libMesh::PreconditionerType;
using libMesh::QGauss;
using libMesh::QTrap;
using libMesh::QuadratureType;
using libMesh::ReferenceCountedObject;
using libMesh::ScalarTraits;
using libMesh::SimpleRange;
using libMesh::SyncElementIntegers;
using libMesh::System;
using libMesh::TriangulatorInterface;
