//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AbaqusInputObjects.h"
#include "GeneralUserObject.h"
#include "BlockRestrictable.h"
#include "TaggingInterface.h"
#include "DynamicLibraryLoader.h"
#include "AbaqusUELMesh.h"

/**
 * This user-object is a testbed for implementing a custom element.
 */
class AbaqusUELMeshUserElement : public GeneralUserObject, public TaggingInterface
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
  AbaqusUELMeshUserElement(const InputParameters & params);

  virtual void timestepSetup() override;
  // virtual void initialSetup() override;
  // virtual void meshChanged() override;

  virtual void initialize() override final {}
  virtual void execute() override;
  virtual void finalize() override final {}

  const std::array<Real, 8> * getUELEnergy(dof_id_type element_id) const;

  std::pair<std::size_t, const std::unordered_map<Abaqus::AbaqusID, std::vector<Real>> &>
  getStateVars() const
  {
    return {_nstatev, _statev[_statev_index_current]};
  }
  const std::vector<Abaqus::Index> & getActiveElements() const { return _active_elements; }
  const std::vector<Abaqus::Element> & getElements() const { return _uel_elements; }

protected:
  /// setup the range of elements this object operates on
  void setupElementSet();

  /// Auxiliary system object
  AuxiliarySystem * _aux_sys;

  std::string _uel_type;

  /// The plugin file name
  FileName _plugin;

  /// The plugin library wrapper
  DynamicLibraryLoader _library;

  /// Function pointer to the dynamically loaded function
  const uel_t _uel;

  /// The \p MooseMesh that this user object operates on
  AbaqusUELMesh & _uel_mesh;

  /// definition of the UEL this object is operating on
  const Abaqus::UserElement & _uel_definition;

  /// all elements in the UEL mesh
  std::vector<Abaqus::Element> & _uel_elements;

  /// selected set names
  const std::vector<std::string> & _element_set_names;

  /// active elements for each element set
  std::vector<Abaqus::Index> _active_elements;

  /// Auxiliary variable names
  std::vector<AuxVariableName> _aux_variable_names;

  /// pointers to the variable objects
  std::vector<std::vector<const MooseVariableFieldBase *>> _variables;

  /// pointers to the auxiliary variable objects
  std::vector<const MooseVariableFieldBase *> _aux_variables;

  /// The subdomain ids this object operates on
  const std::set<SubdomainID> _sub_ids;

  /// properties for each element set
  std::vector<std::pair<std::vector<Real> *, std::vector<int> *>> _properties;

  /// stateful data
  int _nstatev;
  std::array<std::unordered_map<Abaqus::AbaqusID, std::vector<Real>>, 2> & _statev;
  std::size_t & _statev_index_current;
  std::size_t & _statev_index_old;

  /// energy data
  const bool _use_energy;
  std::map<dof_id_type, std::array<Real, 8>> & _energy;
  std::map<dof_id_type, std::array<Real, 8>> & _energy_old;

  /// timestep scaling factor
  Real _pnewdt;

  /// Residual contribution returned from the UEL plugin
  DenseVector<Real> _local_re;
  /// Jacobian contribution returned from the UEL plugin
  DenseMatrix<Real> _local_ke;
  /// transpose of the Jacobian inserted into the MOOSE non-linear system
  DenseMatrix<Real> _local_ke_T;
};
