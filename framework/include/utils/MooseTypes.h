//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MOOSETYPES_H
#define MOOSETYPES_H

#include "Moose.h"

#include "libmesh/libmesh.h"
#include "libmesh/id_types.h"
#include "libmesh/stored_range.h"
#include "libmesh/elem.h"
#include "libmesh/petsc_macro.h"
#include "libmesh/boundary_info.h"
#include "libmesh/parameters.h"
#include "libmesh/vector_value.h"
#include "libmesh/tensor_value.h"
#include "libmesh/type_n_tensor.h"

// BOOST include
#include "bitmask_operators.h"

#include <string>
#include <vector>
#include <memory>

// DO NOT USE (Deprecated)
#define MooseSharedPointer std::shared_ptr
#define MooseSharedNamespace std

/**
 * Macro for inferring the proper type of a normal loop index compatible
 * with the "auto" keyword.
 * Usage:
 *   for (auto i = beginIndex(v); i < v.size(); ++i)    // default index is zero
 *   for (auto i = beginIndex(v, 1); i < v.size(); ++i) // index is supplied
 */
// The multiple macros that you would need anyway [as per: Crazy Eddie (stack overflow)]
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

#define beginIndex_0() ERROR-- > "beginIndex() requires one or two arguments"
#define beginIndex_1(A) decltype(A.size())(0)
#define beginIndex_2(A, B) decltype(A.size())(B)
#define beginIndex_3(A, B, C) ERROR-- > "beginIndex() requires one or two arguments"
#define beginIndex_4(A, B, C, D) ERROR-- > "beginIndex() requires one or two arguments"

// The interim macro that simply strips the excess and ends up with the required macro
#define beginIndex_X(x, A, B, C, D, FUNC, ...) FUNC

// The macro that the programmer uses
#define beginIndex(...)                                                                            \
  beginIndex_X(,                                                                                   \
               ##__VA_ARGS__,                                                                      \
               beginIndex_4(__VA_ARGS__),                                                          \
               beginIndex_3(__VA_ARGS__),                                                          \
               beginIndex_2(__VA_ARGS__),                                                          \
               beginIndex_1(__VA_ARGS__),                                                          \
               beginIndex_0(__VA_ARGS__))
#ifdef __clang__
#pragma clang diagnostic pop
#endif

/**
 * forward declarations
 */
template <typename>
class MooseArray;

/**
 * MOOSE typedefs
 */
typedef Real PostprocessorValue;
typedef std::vector<Real> VectorPostprocessorValue;
typedef Real ScatterVectorPostprocessorValue;
typedef boundary_id_type BoundaryID;
typedef unsigned int InterfaceID;
typedef subdomain_id_type SubdomainID;
typedef unsigned int MooseObjectID;
typedef unsigned int THREAD_ID;
typedef unsigned int TagID;
typedef unsigned int PerfID;

typedef StoredRange<std::vector<dof_id_type>::iterator, dof_id_type> NodeIdRange;
typedef StoredRange<std::vector<const Elem *>::iterator, const Elem *> ConstElemPointerRange;

template <typename OutputType>
struct OutputTools
{
  typedef OutputType OutputShape;
  typedef OutputType OutputValue;
  typedef typename TensorTools::IncrementRank<OutputShape>::type OutputGradient;
  typedef typename TensorTools::IncrementRank<OutputGradient>::type OutputSecond;
  typedef typename TensorTools::DecrementRank<OutputShape>::type OutputDivergence;

  typedef MooseArray<OutputShape> VariableValue;
  typedef MooseArray<OutputGradient> VariableGradient;
  typedef MooseArray<OutputSecond> VariableSecond;
  typedef MooseArray<OutputShape> VariableCurl;
  typedef MooseArray<OutputDivergence> VariableDivergence;

  typedef MooseArray<std::vector<OutputShape>> VariablePhiValue;
  typedef MooseArray<std::vector<OutputGradient>> VariablePhiGradient;
  typedef MooseArray<std::vector<OutputSecond>> VariablePhiSecond;
  typedef MooseArray<std::vector<OutputShape>> VariablePhiCurl;
  typedef MooseArray<std::vector<OutputDivergence>> VariablePhiDivergence;

  typedef MooseArray<std::vector<OutputShape>> VariableTestValue;
  typedef MooseArray<std::vector<OutputGradient>> VariableTestGradient;
  typedef MooseArray<std::vector<OutputSecond>> VariableTestSecond;
  typedef MooseArray<std::vector<OutputShape>> VariableTestCurl;
  typedef MooseArray<std::vector<OutputDivergence>> VariableTestDivergence;
};

typedef MooseArray<Real> VariableValue;
typedef MooseArray<VectorValue<Real>> VariableGradient;
typedef MooseArray<TensorValue<Real>> VariableSecond;
typedef MooseArray<Real> VariableCurl;

typedef MooseArray<std::vector<Real>> VariablePhiValue;
typedef MooseArray<std::vector<VectorValue<Real>>> VariablePhiGradient;
typedef MooseArray<std::vector<TensorValue<Real>>> VariablePhiSecond;
typedef MooseArray<std::vector<Real>> VariablePhiCurl;

typedef MooseArray<std::vector<Real>> VariableTestValue;
typedef MooseArray<std::vector<VectorValue<Real>>> VariableTestGradient;
typedef MooseArray<std::vector<TensorValue<Real>>> VariableTestSecond;
typedef MooseArray<std::vector<Real>> VariableTestCurl;

typedef MooseArray<VectorValue<Real>> VectorVariableValue;
typedef MooseArray<TensorValue<Real>> VectorVariableGradient;
typedef MooseArray<TypeNTensor<3, Real>> VectorVariableSecond;
typedef MooseArray<VectorValue<Real>> VectorVariableCurl;

typedef MooseArray<std::vector<VectorValue<Real>>> VectorVariablePhiValue;
typedef MooseArray<std::vector<TensorValue<Real>>> VectorVariablePhiGradient;
typedef MooseArray<std::vector<TypeNTensor<3, Real>>> VectorVariablePhiSecond;
typedef MooseArray<std::vector<VectorValue<Real>>> VectorVariablePhiCurl;

typedef MooseArray<std::vector<VectorValue<Real>>> VectorVariableTestValue;
typedef MooseArray<std::vector<TensorValue<Real>>> VectorVariableTestGradient;
typedef MooseArray<std::vector<TypeNTensor<3, Real>>> VectorVariableTestSecond;
typedef MooseArray<std::vector<VectorValue<Real>>> VectorVariableTestCurl;

namespace Moose
{
const SubdomainID ANY_BLOCK_ID = libMesh::Elem::invalid_subdomain_id - 1;
const SubdomainID INVALID_BLOCK_ID = libMesh::Elem::invalid_subdomain_id;
const BoundaryID ANY_BOUNDARY_ID = static_cast<BoundaryID>(-1);
const BoundaryID INVALID_BOUNDARY_ID = libMesh::BoundaryInfo::invalid_id;
const std::set<SubdomainID> EMPTY_BLOCK_IDS = {};
const std::set<BoundaryID> EMPTY_BOUNDARY_IDS = {};

/**
 * MaterialData types
 *
 * @see FEProblemBase, MaterialPropertyInterface
 */
enum MaterialDataType
{
  BLOCK_MATERIAL_DATA,
  BOUNDARY_MATERIAL_DATA,
  FACE_MATERIAL_DATA,
  NEIGHBOR_MATERIAL_DATA
};

/**
 * Flag for AuxKernel related execution type.
 */
enum AuxGroup
{
  PRE_IC = 0,
  PRE_AUX = 1,
  POST_AUX = 2,
  ALL = 3
};

/**
 * Framework-wide stuff
 */
enum VarKindType
{
  VAR_NONLINEAR,
  VAR_AUXILIARY,
  VAR_ANY
};

enum VarFieldType
{
  VAR_FIELD_STANDARD,
  VAR_FIELD_SCALAR,
  VAR_FIELD_VECTOR,
  VAR_FIELD_ANY
};

enum CouplingType
{
  COUPLING_DIAG,
  COUPLING_FULL,
  COUPLING_CUSTOM
};

enum ConstraintSideType
{
  SIDE_MASTER,
  SIDE_SLAVE
};

enum DGResidualType
{
  Element,
  Neighbor
};

enum DGJacobianType
{
  ElementElement,
  ElementNeighbor,
  NeighborElement,
  NeighborNeighbor
};

enum ConstraintType
{
  Slave = Element,
  Master = Neighbor
};

enum ConstraintJacobianType
{
  SlaveSlave = ElementElement,
  SlaveMaster = ElementNeighbor,
  MasterSlave = NeighborElement,
  MasterMaster = NeighborNeighbor
};

enum CoordinateSystemType
{
  COORD_XYZ,
  COORD_RZ,
  COORD_RSPHERICAL
};

/**
 * Preconditioning side
 */
enum PCSideType
{
  PCS_LEFT,
  PCS_RIGHT,
  PCS_SYMMETRIC,
  PCS_DEFAULT ///< Use whatever we have in PETSc
};

/**
 * Norm type for converge test
 */
enum MooseKSPNormType
{
  KSPN_NONE,
  KSPN_PRECONDITIONED,
  KSPN_UNPRECONDITIONED,
  KSPN_NATURAL,
  KSPN_DEFAULT ///< Use whatever we have in PETSc
};

/**
 * Type of the solve
 */
enum SolveType
{
  ST_PJFNK,  ///< Preconditioned Jacobian-Free Newton Krylov
  ST_JFNK,   ///< Jacobian-Free Newton Krylov
  ST_NEWTON, ///< Full Newton Solve
  ST_FD,     ///< Use finite differences to compute Jacobian
  ST_LINEAR  ///< Solving a linear problem
};

/**
 * Type of the eigen solve
 */
enum EigenSolveType
{
  EST_POWER,              ///< Power / Inverse / RQI
  EST_ARNOLDI,            ///< Arnoldi
  EST_KRYLOVSCHUR,        ///< Krylov-Schur
  EST_JACOBI_DAVIDSON,    ///< Jacobi-Davidson
  EST_NONLINEAR_POWER,    ///< Nonlinear inverse power
  EST_MF_NONLINEAR_POWER, ///< Matrix-free nonlinear inverse power
  EST_MONOLITH_NEWTON,    ///< Newton-based eigen solver
  EST_MF_MONOLITH_NEWTON, ///< Matrix-free Newton-based eigen solver
};

/**
 * Type of the eigen problem
 */
enum EigenProblemType
{
  EPT_HERMITIAN,             ///< Hermitian
  EPT_NON_HERMITIAN,         ///< Non-Hermitian
  EPT_GEN_HERMITIAN,         ///< Generalized Hermitian
  EPT_GEN_INDEFINITE,        ///< Generalized Hermitian indefinite
  EPT_GEN_NON_HERMITIAN,     ///< Generalized Non-Hermitian
  EPT_POS_GEN_NON_HERMITIAN, ///< Generalized Non-Hermitian with positive (semi-)definite B
  EPT_SLEPC_DEFAULT          ///< use whatever SLPEC has by default
};

/**
 * Which eigen pairs
 */
enum WhichEigenPairs
{
  WEP_LARGEST_MAGNITUDE,  ///< largest magnitude
  WEP_SMALLEST_MAGNITUDE, ///< smallest magnitude
  WEP_LARGEST_REAL,       ///< largest real
  WEP_SMALLEST_REAL,      ///< smallest real
  WEP_LARGEST_IMAGINARY,  ///< largest imaginary
  WEP_SMALLEST_IMAGINARY, ///< smallest imaginary
  WEP_TARGET_MAGNITUDE,   ///< target magnitude
  WEP_TARGET_REAL,        ///< target real
  WEP_TARGET_IMAGINARY,   ///< target imaginary
  WEP_ALL_EIGENVALUES,    ///< all eigenvalues
  WEP_SLEPC_DEFAULT       ///< use whatever we have in SLEPC
};

/**
 * Time integrators
 */
enum TimeIntegratorType
{
  TI_IMPLICIT_EULER,
  TI_EXPLICIT_EULER,
  TI_CRANK_NICOLSON,
  TI_BDF2,
  TI_EXPLICIT_MIDPOINT,
  TI_LSTABLE_DIRK2,
  TI_EXPLICIT_TVD_RK_2,
};

/**
 * Type of constraint formulation
 */
enum ConstraintFormulationType
{
  Penalty,
  Kinematic
};
/**
 * Type of the line search
 */
enum LineSearchType
{
  LS_INVALID, ///< means not set
  LS_DEFAULT,
  LS_NONE,
  LS_BASIC,
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3, 3, 0)
  LS_CUBIC,
  LS_QUADRATIC,
  LS_BASICNONORMS,
#else
  LS_SHELL,
  LS_CONTACT,
  LS_L2,
  LS_BT,
  LS_CP
#endif
#endif
};

/**
 * Type of the matrix-free finite-differencing parameter
 */
enum MffdType
{
  MFFD_INVALID, ///< means not set
  MFFD_WP,
  MFFD_DS
};

/**
 * Type of patch update strategy for modeling node-face constraints or contact
 */
enum PatchUpdateType
{
  Never,
  Always,
  Auto,
  Iteration
};

/**
 * Main types of Relationship Managers
 */
enum class RelationshipManagerType : unsigned char
{
  DEFAULT = 0,
  GEOMETRIC = 1 << 0,
  ALGEBRAIC = 1 << 1,
  COUPLING = 1 << 2
};

std::string stringify(const Moose::RelationshipManagerType & t);
}

namespace libMesh
{
template <>
inline void
print_helper(std::ostream & os, const Moose::RelationshipManagerType * param)
{
  // Specialization so that we don't print out unprintable characters
  os << Moose::stringify(*param);
}

// End of Moose Namespace
}

template <>
struct enable_bitmask_operators<Moose::RelationshipManagerType>
{
  static const bool enable = true;
};

/**
 * This Macro is used to generate std::string derived types useful for
 * strong type checking and special handling in the GUI.  It does not
 * extend std::string in any way so it is generally "safe"
 */
#define DerivativeStringClass(TheName)                                                             \
  class TheName : public std::string                                                               \
  {                                                                                                \
  public:                                                                                          \
    TheName() : std::string() {}                                                                   \
    TheName(const std::string & str) : std::string(str) {}                                         \
    TheName(const std::string & str, size_t pos, size_t n = npos) : std::string(str, pos, n) {}    \
    TheName(const char * s, size_t n) : std::string(s, n) {}                                       \
    TheName(const char * s) : std::string(s) {}                                                    \
    TheName(size_t n, char c) : std::string(n, c) {}                                               \
  } /* No semicolon here because this is a macro */

// Instantiate new Types

/// This type is for expected (i.e. input) file names or paths that your simulation needs.  If
/// relative paths are assigned to this type, they are treated/modified to be relative to the
/// location of the simulation's main input file's directory.  It can be used to trigger open file
/// dialogs in the GUI.
DerivativeStringClass(FileName);

/// This type is for expected filenames where the extension is unwanted, it can be used to trigger open file dialogs in the GUI
DerivativeStringClass(FileNameNoExtension);

/// This type is similar to "FileName", but is used to further filter file dialogs on known file mesh types
DerivativeStringClass(MeshFileName);

/// This type is for output file base
DerivativeStringClass(OutFileBase);

/// This type is used for objects that expect nonlinear variable names (i.e. Kernels, BCs)
DerivativeStringClass(NonlinearVariableName);

/// This type is used for objects that expect Auxiliary variable names (i.e. AuxKernels, AuxBCs)
DerivativeStringClass(AuxVariableName);

/// This type is used for objects that expect either Nonlinear or Auxiliary Variables such as postprocessors
DerivativeStringClass(VariableName);

/// This type is used for objects that expect Boundary Names/Ids read from or generated on the current mesh
DerivativeStringClass(BoundaryName);

/// This type is similar to BoundaryName but is used for "blocks" or subdomains in the current mesh
DerivativeStringClass(SubdomainName);

/// This type is used for objects that expect Postprocessor objects
DerivativeStringClass(PostprocessorName);

/// This type is used for objects that expect VectorPostprocessor objects
DerivativeStringClass(VectorPostprocessorName);

/// This type is used for objects that expect Moose Function objects
DerivativeStringClass(FunctionName);

/// This type is used for objects that expect Moose Distribution objects
DerivativeStringClass(DistributionName);

/// This type is used for objects that expect Moose Sampler objects
DerivativeStringClass(SamplerName);

/// This type is used for objects that expect "UserObject" names
DerivativeStringClass(UserObjectName);

/// This type is used for objects that expect an Indicator object name
DerivativeStringClass(IndicatorName);

/// This type is used for objects that expect an Marker object name
DerivativeStringClass(MarkerName);

/// This type is used for objects that expect an MultiApp object name
DerivativeStringClass(MultiAppName);

/// Used for objects the require Output object names
DerivativeStringClass(OutputName);

/// Used for objects that expect MaterialProperty names
DerivativeStringClass(MaterialPropertyName);

/// User for accessing Material objects
DerivativeStringClass(MaterialName);

/// Tag Name
DerivativeStringClass(TagName);

#endif // MOOSETYPES_H
