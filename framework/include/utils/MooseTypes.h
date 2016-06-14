/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef MOOSETYPES_H
#define MOOSETYPES_H

#include "Moose.h"

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/id_types.h"
#include "libmesh/stored_range.h"
#include "libmesh/elem.h"
#include "libmesh/petsc_macro.h"
#include "libmesh/boundary_info.h"

#include <string>
#include <vector>

#ifdef LIBMESH_HAVE_CXX11_SHARED_PTR
#  include <memory>
#  define MooseSharedPointer std::shared_ptr
#  define MooseSharedNamespace std
#else
#  include "boost/shared_ptr.hpp"
#  define MooseSharedPointer boost::shared_ptr
#  define MooseSharedNamespace boost
#endif

/**
 * MOOSE typedefs
 */
typedef Real                     PostprocessorValue;
typedef std::vector<Real>        VectorPostprocessorValue;
typedef boundary_id_type         BoundaryID;
typedef unsigned int             InterfaceID;
typedef subdomain_id_type        SubdomainID;
typedef unsigned int             MooseObjectID;
typedef unsigned int             THREAD_ID;


typedef StoredRange<std::vector<dof_id_type>::iterator, dof_id_type> NodeIdRange;
typedef StoredRange<std::vector<const Elem *>::iterator, const Elem *> ConstElemPointerRange;

/// Execution flags - when is the object executed/evaluated
// Note: If this enum is changed, make sure to modify:
//   (1) the local function populateExecTypes in Moose.C.
//   (2) the function in Conversion.C: initExecStoreType()
//   (3) the method SetupInterface::getExecuteOptions
//   (4) the function Output::getExecuteOptions
enum ExecFlagType {
  EXEC_NONE              = 0x00, // 0
  /// Object is evaluated only once at the beginning of the simulation
  EXEC_INITIAL           = 0x01, // 1
  /// Object is evaluated in every residual computation
  EXEC_LINEAR            = 0x02, // 2
  /// Object is evaluated in every jacobian computation
  EXEC_NONLINEAR         = 0x04, // 4
  /// Object is evaluated at the end of every time step
  EXEC_TIMESTEP_END      = 0x08, // 8
  /// Object is evaluated at the beginning of every time step
  EXEC_TIMESTEP_BEGIN    = 0x10, // 16
  /// Object is evaluated at the end of the simulations (output only)
  EXEC_FINAL             = 0x20, // 32
  /// Forces execution to occur (output only)
  EXEC_FORCED            = 0x40, // 64
  /// Forces execution on failed solve (output only)
  EXEC_FAILED            = 0x80, // 128
  /// For use with custom executioners that want to fire objects at a specific time
  EXEC_CUSTOM            = 0x100, // 256
  /// Objects is evaluated on subdomain
  EXEC_SUBDOMAIN         = 0x200  // 512
};


namespace Moose
{
const SubdomainID ANY_BLOCK_ID = libMesh::Elem::invalid_subdomain_id - 1;
const SubdomainID INVALID_BLOCK_ID = libMesh::Elem::invalid_subdomain_id;
const BoundaryID ANY_BOUNDARY_ID = static_cast<BoundaryID>(-1);
const BoundaryID INVALID_BOUNDARY_ID = libMesh::BoundaryInfo::invalid_id;

/**
 * MaterialData types
 *
 * @see FEProblem, MaterialPropertyInterface
 */
enum MaterialDataType {
  BLOCK_MATERIAL_DATA,
  BOUNDARY_MATERIAL_DATA,
  FACE_MATERIAL_DATA,
  NEIGHBOR_MATERIAL_DATA,
  DIRAC_MATERIAL_DATA
};

/**
 * Flag for AuxKernel related exeuction type.
 */
enum AuxGroup
{
  PRE_AUX = 0,
  POST_AUX = 1,
  ALL = 2
};

/**
 * A static list of all the exec types.
 */
extern const std::vector<ExecFlagType> exec_types;

/**
 * Framework-wide stuff
 */
enum VarKindType
{
  VAR_NONLINEAR,
  VAR_AUXILIARY
};

enum KernelType
{
  KT_TIME = 0,
  KT_NONTIME = 1,
  KT_ALL
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
  PCS_SYMMETRIC
};

/**
 * Type of the solve
 */
enum SolveType
{
  ST_PJFNK,            ///< Preconditioned Jacobian-Free Newton Krylov
  ST_JFNK,             ///< Jacobian-Free Newton Krylov
  ST_NEWTON,           ///< Full Newton Solve
  ST_FD,               ///< Use finite differences to compute Jacobian
  ST_LINEAR            ///< Solving a linear problem
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
  LS_INVALID,           ///< means not set
  LS_DEFAULT,
  LS_NONE,
  LS_BASIC,
#ifdef LIBMESH_HAVE_PETSC
#if PETSC_VERSION_LESS_THAN(3,3,0)
  LS_CUBIC,
  LS_QUADRATIC,
  LS_BASICNONORMS,
#else
  LS_SHELL,
  LS_L2,
  LS_BT,
  LS_CP
#endif
#endif
};

}

/**
 * This Macro is used to generate std::string derived types useful for
 * strong type checking and special handling in the GUI.  It does not
 * extend std::string in any way so it is generally "safe"
 */
#define DerivativeStringClass(TheName)                                  \
  class TheName : public std::string                                    \
  {                                                                     \
  public:                                                               \
    TheName(): std::string() {}                                         \
    TheName(const std::string& str): std::string(str) {}                \
    TheName(const std::string& str, size_t pos, size_t n = npos):       \
      std::string(str, pos, n) {}                                       \
    TheName(const char * s, size_t n): std::string(s,n) {}              \
    TheName(const char * s): std::string(s) {}                          \
    TheName(size_t n, char c): std::string(n, c) {}                     \
  } /* No semicolon here because this is a macro */

// Instantiate new Types

/// This type is for expected filenames, it can be used to trigger open file dialogs in the GUI
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

#endif // MOOSETYPES_H
