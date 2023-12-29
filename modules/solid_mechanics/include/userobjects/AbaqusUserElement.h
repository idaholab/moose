//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "BlockRestrictable.h"
#include "TaggingInterface.h"
#include "DynamicLibraryLoader.h"

class MooseMesh;
namespace libMesh
{
class Elem;
class MeshBase;
}

/**
 * This user-object is a testbed for implementing a custom element.
 */
class AbaqusUserElement : public GeneralUserObject,
                          public BlockRestrictable,
                          public TaggingInterface
{
public:
  /// function type for the external UMAT function
  typedef void (*uel_t)(
      Real RHS[],    // (MLVARX,*)      Residual vector contribution for the current element
      Real AMATRX[], // (NDOFEL,NDOFEL) Jacobian contribution for the current element
      Real SVARS[],  // (NSVARS)        Persistent state variable values for the current element
      Real ENERGY[], // (8)             Energy quantities at the start of the current
                     //                 increment (to be updated by the UEL routine)
      int * NDOFEL,  // Number of degrees of freedom (DOFs) for the current element
      int * NRHS,    // NRHS=1: RHS should contain the residual vector,
                     // NRHS=2: not implemented (modified Riks static procedure)
      int * NSVARS,  // Number of persistent state variables for the element
      Real PROPS[],  // (NPROPS) Static property values (parameters) defined for use with this
                     // element.
      int * NPROPS,  //
      Real COORDS[], // (MCRD,NNODE) Undisplaced coordinates of the element nodes
                     //              COORDS(K1,K2) is the K1th coordinate of the
                     //              K2th node of the element
      int * MCRD,  // Maximum number of coordinates needed at any node point (COORDINATES keyword -
                   // unsupported)
      int * NNODE, // Number of nodes in the current element
      Real U[],    // (NDOFEL)   Total values of the variables
      Real DU[],   // (MLVARX,*) Incremental values of the variables for the current increment
                   //            for right-hand-side
      Real V[],    // (NDOFEL) Time rate of change of the variables (velocities,
                   //          rates of rotation). Defined for implicit dynamics only (LFLAGS(1)
                   //          11 or 12)
      Real A[],    // (NDOFEL) Accelerations of the variables. Defined for implicit dynamics
                   //          only (LFLAGS(1) 11 or 12).
      int * JTYPE, // Integer defining the element type. This is the user-defined integer value n in
                   // element type Un
      Real TIME[], // (2) step time and total time
      Real * DTIME,  // Time increment
      int * KSTEP,   // Step number (as per Abaqus definition) can be set by the user
      int * KINC,    // Increment number (MOOSE time step)
      int * JELEM,   // User-defined element number
      Real PRAMS[],  // (*) parameters associated with the solution procedure
      int * NDLOAD,  // Number of applied loads to the element (unused)
      int JDLTYP[],  // (MDLOAD, *) array containing the integers used to define distributed load
                     //             types for the element
      Real ADLMAG[], // (MDLOAD,*)
      Real PREDEF[], // (2,NPREDF,NNODE) predefined field variables, such as temperature in an
                     //                  uncoupled stress/displacement analysis
      int * NPREDF,  // Number of predefined field (auxiliary) variables, including temperature
      int LFLAGS[],  // (*) flags that define the current solution procedure
      int * MLVARX,  // used when several displacement or right-hand-side vectors are used
      Real DDLMAG[], // (MDLOAD,*)
      int * MDLOAD,  // Total number of distributed loads and/or fluxes defined on this element
      Real * PNEWDT, // Recommended new timestep (unused)
      int JPROPS[],  // (NJPROP) NJPROP integer property values defined for the current element
      int * NJPROP,  // Number of user defined integer properties
      Real * PERIOD  // Current step time period (unused)
  );

  static InputParameters validParams();
  AbaqusUserElement(const InputParameters & params);

  virtual void initialSetup() override;
  virtual void meshChanged() override;

  virtual void initialize() override final;
  virtual void execute() override;
  virtual void finalize() override final {}

  /// getters for the loop class
  const std::vector<const MooseVariableFieldBase *> & getVariables() const { return _variables; }
  const std::vector<const MooseVariableFieldBase *> & getAuxVariables() const
  {
    return _aux_variables;
  }

  const uel_t & getPlugin() const { return _uel; }

protected:
  /// setup the range of elements this object operates on
  void setupElemRange();

  /// The plugin file name
  FileName _plugin;

  /// The plugin library wrapper
  DynamicLibraryLoader _library;

  /// Function pointer to the dynamically loaded function
  const uel_t _uel;

  /// The \p MooseMesh that this user object operates on
  MooseMesh & _moose_mesh;

  /// The \p libMesh mesh that this object acts on
  const libMesh::MeshBase & _mesh;

  /// The dimension of the mesh, e.g. 3 for hexes and tets, 2 for quads and tris
  const unsigned int _dim;

  /// coupled variables to provide the DOF values
  std::vector<NonlinearVariableName> _variable_names;

  /// Auxiliary variable names
  std::vector<AuxVariableName> _aux_variable_names;

  /// pointers to the variable objects
  std::vector<const MooseVariableFieldBase *> _variables;

  /// pointers to the auxiliary variable objects
  std::vector<const MooseVariableFieldBase *> _aux_variables;

  /// The subdomain ids this object operates on
  const std::set<SubdomainID> _sub_ids;

  /// All the active and elements local to this process that exist on this object's subdomains
  std::unique_ptr<ConstElemRange> _elem_range;

  /// props
  std::vector<Real> _props;
  int _nprops;

  /// stateful data
  int _nstatev;
  std::array<std::map<dof_id_type, std::vector<Real>>, 2> _statev;
  std::size_t _statev_index_current;
  std::size_t _statev_index_old;

  /// Abaqus element type
  const int _jtype;

  friend class UELThread;
};
