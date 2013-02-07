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

#include <string>
#include <vector>

// libMesh includes
#include "libmesh/id_types.h"
#include "libmesh/stored_range.h"
#include "libmesh/elem.h"

/**
 * MOOSE typedefs
 */
typedef Real                     PostprocessorValue;
typedef boundary_id_type         BoundaryID;
typedef subdomain_id_type        SubdomainID;

typedef StoredRange<std::vector<unsigned int>::iterator, unsigned int> NodeIdRange;
typedef StoredRange<std::vector<const Elem *>::iterator, const Elem *> ConstElemPointerRange;

namespace Moose
{
const SubdomainID ANY_BLOCK_ID = (SubdomainID) -1;

const BoundaryID ANY_BOUNDARY_ID = (BoundaryID) -1;

/**
 * Framework-wide stuff
 */

enum VarKindType
{
  VAR_NONLINEAR,
  VAR_AUXILIARY
};

enum TimeSteppingScheme
{
  IMPLICIT_EULER,
  EXPLICIT_EULER,
  BDF2,
  CRANK_NICOLSON,
  PETSC_TS
};

enum KernelType
{
  KT_TIME,
  KT_NONTIME,
  KT_ALL
};

// Bit mask flags to be able to combine them through or-operator (|)
enum PostprocessorType
{
  PPS_RESIDUAL = 0x01,
  PPS_JACOBIAN = 0x02,
  PPS_TIMESTEP = 0x04,
  PPS_NEWTONIT = 0x08
};

enum CouplingType
{
  COUPLING_DIAG,
  COUPLING_FULL,
  COUPLING_CUSTOM
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

enum PPSOutputType
{
  PPS_OUTPUT_NONE,
  PPS_OUTPUT_AUTO,
  PPS_OUTPUT_SCREEN,
  PPS_OUTPUT_FILE,
  PPS_OUTPUT_BOTH
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

/// This type is used for objects that expect Moose Function objects
DerivativeStringClass(FunctionName);

/// This type is used for objects that expect "UserObject" names
DerivativeStringClass(UserObjectName);

/// This type is used for objects that expect an Indicator object name
DerivativeStringClass(IndicatorName);

/// This type is used for objects that expect an Marker object name
DerivativeStringClass(MarkerName);

#endif // MOOSETYPES_H
