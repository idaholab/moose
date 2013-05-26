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

#ifndef DISPLACEDPROBLEM_H
#define DISPLACEDPROBLEM_H

#include "SubProblem.h"
#include "MooseMesh.h"
#include "DisplacedSystem.h"
#include "Assembly.h"
#include "GeometricSearchData.h"
#include "FEProblem.h"

// libMesh
#include "libmesh/equation_systems.h"
#include "libmesh/explicit_system.h"
#include "libmesh/numeric_vector.h"

class SubProblem;
class MooseVariable;
class AssemblyData;
class DisplacedProblem;
class ExodusOutput;

template<>
InputParameters validParams<DisplacedProblem>();


class DisplacedProblem : public SubProblem
{
public:
  DisplacedProblem(FEProblem & mproblem, MooseMesh & displaced_mesh, InputParameters params);
  virtual ~DisplacedProblem();

  virtual EquationSystems & es() { return _eq; }
  virtual MooseMesh & mesh() { return _mesh; }
  MooseMesh & refMesh();

  DisplacedSystem & nlSys() { return _displaced_nl; }
  DisplacedSystem & auxSys() { return _displaced_aux; }

  virtual void createQRules(QuadratureType type, Order order);

  /**
   * Whether or not this problem should utilize FE shape function caching.
   *
   * @param fe_cache True for using the cache false for not.
   */
  virtual void useFECache(bool fe_cache);

  virtual void init();
  virtual void solve();
  virtual bool converged();

  virtual void syncSolutions(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);
  virtual void updateMesh(const NumericVector<Number> & soln, const NumericVector<Number> & aux_soln);

  virtual bool isTransient() { return _mproblem.isTransient(); }
  virtual Moose::CoordinateSystemType getCoordSystem(SubdomainID sid) { return _mproblem.getCoordSystem(sid); }

  // Variables /////
  virtual bool hasVariable(const std::string & var_name);
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name);
  virtual bool hasScalarVariable(const std::string & var_name);
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name);
  virtual void addVariable(const std::string & var_name, const FEType & type, Real scale_factor, const std::set< SubdomainID > * const active_subdomains = NULL);
  virtual void addAuxVariable(const std::string & var_name, const FEType & type, const std::set< SubdomainID > * const active_subdomains = NULL);
  virtual void addScalarVariable(const std::string & var_name, Order order, Real scale_factor = 1.);
  virtual void addAuxScalarVariable(const std::string & var_name, Order order, Real scale_factor = 1.);

  // Output /////
  virtual void output(bool force = false);

  // Adaptivity /////
  virtual void initAdaptivity();
  virtual void meshChanged();

  // reinit /////

  virtual void prepare(const Elem * elem, THREAD_ID tid);
  virtual void prepareFace(const Elem * elem, THREAD_ID tid);
  virtual void prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid);
  virtual void prepareAssembly(THREAD_ID tid);
  virtual void prepareAssemblyNeighbor(THREAD_ID tid);

  virtual bool reinitDirac(const Elem * elem, THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemPhys(const Elem * elem, std::vector<Point> phys_points_in_elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid);
  virtual void reinitNodes(const std::vector<unsigned int> & nodes, THREAD_ID tid);
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid);
  virtual void reinitNeighborPhys(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid);
  virtual void reinitNodeNeighbor(const Node * node, THREAD_ID tid);
  virtual void reinitScalars(THREAD_ID tid);

  /// Fills "elems" with the elements that should be looped over for Dirac Kernels
  virtual void getDiracElements(std::set<const Elem *> & elems);
  virtual void clearDiracInfo();

  virtual void addResidual(THREAD_ID tid);
  virtual void addResidualNeighbor(THREAD_ID tid);

  virtual void cacheResidual(THREAD_ID tid);
  virtual void cacheResidualNeighbor(THREAD_ID tid);
  virtual void addCachedResidual(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void setResidual(NumericVector<Number> & residual, THREAD_ID tid);
  virtual void setResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid);

  virtual void addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid);
  virtual void addJacobianBlock(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, THREAD_ID tid);
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, unsigned int ivar, unsigned int jvar, const DofMap & dof_map, std::vector<unsigned int> & dof_indices, std::vector<unsigned int> & neighbor_dof_indices, THREAD_ID tid);

  virtual void cacheJacobian(THREAD_ID tid);
  virtual void cacheJacobianNeighbor(THREAD_ID tid);
  virtual void addCachedJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid);

  virtual void prepareShapes(unsigned int var, THREAD_ID tid);
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid);
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid);

  virtual Assembly & assembly(THREAD_ID tid) { return *_assembly[tid]; }

  // Geom Search /////
  virtual void updateGeomSearch();
  virtual GeometricSearchData & geomSearchData() { return _geometric_search_data; }

  virtual bool computingInitialResidual();

  virtual void onTimestepBegin();
  virtual void onTimestepEnd();

  virtual Order getQuadratureOrder();

  // Postprocessors /////
  virtual void outputPps(const FormattedTable & table);

  /**
   * Set which variables will be written in output files
   * @param output_variables The list of variable names to write in the output files
   */
  virtual void setOutputVariables(std::vector<std::string> output_variables);

  /**
   * Will make sure that all dofs connected to elem_id are ghosted to this processor
   */
  virtual void addGhostedElem(unsigned int elem_id);
  /**
   * Will make sure that all necessary elements from boundary_id are ghosted to this processor
   * @param boundary_id Boundary ID
   */
  virtual void addGhostedBoundary(BoundaryID boundary_id);

protected:
  FEProblem & _mproblem;
  MooseMesh & _mesh;
  EquationSystems _eq;
  /// reference mesh
  MooseMesh & _ref_mesh;
  std::vector<std::string> _displacements;

  DisplacedSystem _displaced_nl;
  DisplacedSystem _displaced_aux;

  const NumericVector<Number> * _nl_solution;
  const NumericVector<Number> * _aux_solution;

  std::vector<Assembly *> _assembly;

  GeometricSearchData _geometric_search_data;

  ExodusOutput * _ex;
  bool _seq;


  friend class UpdateDisplacedMeshThread;
};

#endif /* DISPLACEDPROBLEM_H */
