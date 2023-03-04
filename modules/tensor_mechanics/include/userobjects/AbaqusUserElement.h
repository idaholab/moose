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
      Real RHS[],    // (MLVARX,*)      contributions of this element to the right-hand-side vector
      Real AMATRX[], // (NDOFEL,NDOFEL) contribution of this element to the Jacobian
      Real SVARS[],  // (NSVARS)        values of the solution-dependent state variables associated
                     //                 with this element
      Real ENERGY[], // (8)             element energy quantities at the start of the current
                     //                 increment (update this)
      int * NDOFEL,  // number of solution-dependent state variables associated with the element
      int * NRHS,    // NRHS=1: RHS should contain the residual vector,
                     // NRHS=2: not implemented (modified Riks static procedure)
      int * NSVARS,  //
      Real PROPS[],  // (NPROPS) real property values defined for use with this element.
      int * NPROPS,  //
      Real COORDS[], // (MCRD,NNODE)    array containing the original coordinates of the nodes of
                     //                 the element. COORDS(K1,K2) is the K1th coordinate of the
                     //                 K2th node of the element
      int * MCRD,    // maximum of the user-defined maximum number of coordinates needed at any node
                     // point
      int * NNODE,   //
      Real U[],      // (NDOFEL)        Total values of the variables
      Real DU[], // (MLVARX, *)      Incremental values of the variables for the current increment
                 //                  for right-hand-side
      Real V[],  // (NDOFEL)        Time rate of change of the variables (velocities,
                //                 rates of rotation). Defined for implicit dynamics only (LFLAGS(1)
                //                 11 or 12)
      Real A[],    // (NDOFEL)        Accelerations of the variables. Defined for implicit dynamics
                   //                 only (LFLAGS(1) 11 or 12).
      int * JTYPE, // Integer defining the element type. This is the user-defined integer value n in
                   // element type Un
      Real TIME[], // (2)
      Real * DTIME,  //
      int * KSTEP,   //
      int * KINC,    //
      int * JELEM,   // User-assigned element number
      Real PRAMS[],  // (*)            parameters associated with the solution procedure
      int * NDLOAD,  //
      int JDLTYP[],  // (MDLOAD, *) array containing the integers used to define distributed load
                     // types for the element
      Real ADLMAG[], // (MDLOAD,*)
      Real PREDEF[], // (2,NPREDF,NNODE) predefined field variables, such as temperature in an
                     // uncoupled stress/displacement analysis
      int * NPREDF,  // Number of predefined field variables, including temperature
      int LFLAGS[],  // (*)              flags that define the current solution procedure
      int * MLVARX,  // used when several displacement or right-hand-side vectors are used
      Real DDLMAG[], // (MDLOAD,*)
      int * MDLOAD,  // Total number of distributed loads and/or fluxes defined on this element
      Real * PNEWDT, //
      int JPROPS[],  // (*) integer array containing the NJPROP integer property values defined for
                     // use with this element
      int * NJPROP,  //
      Real * PERIOD  //
  );

  // UEL routine sets RHS, AMATRX, SVARS, ENERGY, and PNEWDT

  /*
   * ENERGY(1) Kinetic energy.
   * ENERGY(2) Elastic strain energy.
   * ENERGY(3) Creep dissipation.
   * ENERGY(4) Plastic dissipation.
   * ENERGY(5) Viscous dissipation.
   * ENERGY(6) “Artificial strain energy” associated with such effects as artificial stiffness
   * introduced to control hourglassing or other singular modes in the element. ENERGY(7)
   * Electrostatic energy. ENERGY(8) Incremental work done by loads applied within the user element.
   */

  static InputParameters validParams();
  AbaqusUserElement(const InputParameters & params);

  void initialSetup() override;
  void meshChanged() override;

  void initialize() override final;
  void execute() override;
  void finalize() override final {}

  /// getters for the loop class
  const std::vector<const MooseVariableFieldBase *> & getVariables() const { return _variables; }
  const uel_t & getPlugin() const { return _uel; }

protected:
  /// setup the range of elements this object operates on
  void setupElemRange();

  void setupRange();

  // The plugin file name
  FileName _plugin;

  // The plugin library wrapper
  DynamicLibraryLoader _library;

  // Function pointer to the dynamically loaded function
  const uel_t _uel;

  /// The \p MooseMesh that this user object operates on
  MooseMesh & _moose_mesh;

  /// The \p libMesh mesh that this object acts on
  const libMesh::MeshBase & _mesh;

  /// The dimension of the mesh, e.g. 3 for hexes and tets, 2 for quads and tris
  const unsigned int _dim;

  /// coupled variables to provide the DOF values
  std::vector<NonlinearVariableName> _variable_names;

  /// pointers to the variable objects
  std::vector<const MooseVariableFieldBase *> _variables;

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

  friend class UELThread;
};
