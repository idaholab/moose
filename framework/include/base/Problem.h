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

#ifndef PROBLEM_H
#define PROBLEM_H

#include "Moose.h"
#include "ParallelUniqueId.h"
#include "InputParameters.h"
#include "ExecStore.h"
#include "MooseMesh.h"
#include "MooseArray.h"
#include "Function.h"
#include "UserObjectWarehouse.h"

// libMesh
#include "libmesh_common.h"
#include "equation_systems.h"
#include "quadrature.h"
#include "elem.h"
#include "node.h"
#include "nonlinear_implicit_system.h"

class MooseVariable;
class MooseVariableScalar;
class Output;
class Material;

class Problem;

template<>
InputParameters validParams<Problem>();

/**
 * Data structure to keep info about the time period
 */
struct TimePeriod
{
  /// Time when this period starts (included)
  Real _start;
  /// Time when this period ends (excluded)
  Real _end;
};

/**
 * Class that hold the whole problem being solved.
 */
class Problem
{
public:
  Problem(const std::string & name, InputParameters parameters);
  virtual ~Problem();

  virtual EquationSystems & es() = 0;
  virtual Problem * parent() = 0;

  /**
   * Get reference to all-purpose parameters
   */
  InputParameters & parameters() { return _pars; }

  // Variables /////
  virtual bool hasVariable(const std::string & var_name) = 0;
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name) = 0;
  virtual bool hasScalarVariable(const std::string & var_name) = 0;
  virtual MooseVariableScalar & getScalarVariable(THREAD_ID tid, const std::string & var_name) = 0;

  virtual void initialSetup() {};
  virtual void timestepSetup() {};

  virtual void subdomainSetup(SubdomainID subdomain, THREAD_ID tid) = 0;
  virtual void subdomainSetupSide(SubdomainID subdomain, THREAD_ID tid) = 0;

  virtual void prepare(const Elem * elem, THREAD_ID tid) = 0;
  virtual void prepare(const Elem * elem, unsigned int ivar, unsigned int jvar, const std::vector<unsigned int> & dof_indices, THREAD_ID tid) = 0;
  virtual void prepareAssembly(THREAD_ID tid) = 0;

  virtual void reinitElem(const Elem * elem, THREAD_ID tid) = 0;
  virtual void reinitElemFace(const Elem * elem, unsigned int side, BoundaryID bnd_id, THREAD_ID tid) = 0;
  virtual void reinitNode(const Node * node, THREAD_ID tid) = 0;
  virtual void reinitNodeFace(const Node * node, BoundaryID bnd_id, THREAD_ID tid) = 0;
  virtual void reinitNodes(const std::vector<unsigned int> & nodes, THREAD_ID tid) = 0;
  virtual void reinitNeighbor(const Elem * elem, unsigned int side, THREAD_ID tid) = 0;
  virtual void reinitNeighborPhys(const Elem * neighbor, unsigned int neighbor_side, const std::vector<Point> & physical_points, THREAD_ID tid) = 0;
  virtual void reinitNodeNeighbor(const Node * node, THREAD_ID tid) = 0;
  virtual void reinitScalars(THREAD_ID tid) = 0;

  // Materials /////
  virtual void reinitMaterials(SubdomainID blk_id, THREAD_ID tid) = 0;
  virtual void reinitMaterialsFace(SubdomainID blk_id, unsigned int side, THREAD_ID tid) = 0;
  virtual void reinitMaterialsNeighbor(SubdomainID /*blk_id*/, unsigned int /*side*/, THREAD_ID /*tid*/) { mooseError("Not implemented yet."); }
  virtual const std::vector<Material*> & getMaterials(SubdomainID /*block_id*/, THREAD_ID /*tid*/) { mooseError("Not implemented yet."); }
  virtual const std::vector<Material*> & getFaceMaterials(SubdomainID /*block_id*/, THREAD_ID /*tid*/) { mooseError("Not implemented yet."); }

  /**
   * Returns true if the Problem has Dirac kernels it needs to compute on elem.
   */
  virtual bool reinitDirac(const Elem * /*elem*/, THREAD_ID /*tid*/){ mooseError("Cannot reinit this Problem with arbitrary quadrature points!"); return false; };

  /**
   * Fills "elems" with the elements that should be looped over for Dirac Kernels
   */
  virtual void getDiracElements(std::set<const Elem *> & /*elems*/){ mooseError("Cannot retrieve Dirac elements from this problem!"); };

  /**
   * Gets called before Dirac Kernels are asked to add the points they are supposed to be evaluated in
   */
  virtual void clearDiracInfo(){ mooseError("Cannot clear Dirac Info this problem!"); };

  // Solve /////
  virtual void init() = 0;

  /**
   * Returns true if the problem is in the process of computing it's initial residual.
   * @return Whether or not the problem is currently computing the initial residual.
   */
  virtual bool computingInitialResidual() = 0;

  virtual void computeResidual(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, NumericVector<Number> & residual) = 0;
  virtual void computeJacobian(NonlinearImplicitSystem & sys, const NumericVector<Number> & soln, SparseMatrix<Number> & jacobian) = 0;
  virtual void computeBounds(NonlinearImplicitSystem & sys, NumericVector<Number> & lower, NumericVector<Number> & upper) = 0;

  virtual void addResidual(NumericVector<Number> & /*residual*/, THREAD_ID /*tid*/) { }
  virtual void addResidualNeighbor(NumericVector<Number> & /*residual*/, THREAD_ID /*tid*/) { }

  virtual void cacheResidual(THREAD_ID /*tid*/) {}
  virtual void cacheResidualNeighbor(THREAD_ID /*tid*/) {}
  virtual void addCachedResidual(NumericVector<Number> & /*residual*/, THREAD_ID /*tid*/) {}

  virtual void setResidual(NumericVector<Number> & /*residual*/, THREAD_ID /*tid*/) { }
  virtual void setResidualNeighbor(NumericVector<Number> & /*residual*/, THREAD_ID /*tid*/) { }

  virtual void addJacobian(SparseMatrix<Number> & /*jacobian*/, THREAD_ID /*tid*/) { }
  virtual void addJacobianNeighbor(SparseMatrix<Number> & /*jacobian*/, THREAD_ID /*tid*/) { }
  virtual void addJacobianBlock(SparseMatrix<Number> & /*jacobian*/, unsigned int /*ivar*/, unsigned int /*jvar*/, const DofMap & /*dof_map*/, std::vector<unsigned int> & /*dof_indices*/, THREAD_ID /*tid*/) { }
  virtual void addJacobianNeighbor(SparseMatrix<Number> & /*jacobian*/, unsigned int /*ivar*/, unsigned int /*jvar*/, const DofMap & /*dof_map*/, std::vector<unsigned int> & /*dof_indices*/, std::vector<unsigned int> & /*neighbor_dof_indices*/, THREAD_ID /*tid*/) { }

  virtual void cacheJacobian(THREAD_ID /*tid*/) {}
  virtual void cacheJacobianNeighbor(THREAD_ID /*tid*/) {}
  virtual void addCachedJacobian(SparseMatrix<Number> & /*jacobian*/, THREAD_ID /*tid*/) {}

  // Postprocessors /////
  virtual void computePostprocessors(ExecFlagType type = EXEC_TIMESTEP) = 0;
  virtual void outputPostprocessors(bool force = false) = 0;
  virtual Real & getPostprocessorValue(const std::string & name, THREAD_ID tid = 0) = 0;

  // Function /////
  virtual void addFunction(std::string type, const std::string & name, InputParameters parameters);
  virtual Function & getFunction(const std::string & name, THREAD_ID tid = 0);

  // UserData /////
  /**
   * Adds an user object to this problem
   * @param type The type (C++ class name) of the user object
   * @param name The name of the user object
   * @param parameters Parameters of the user object
   */
  virtual void addUserObject(const std::string & type, const std::string & name, InputParameters parameters);
  /**
   * Get the user object by its name
   * @param name The name of the user object being retrieved
   * @param tid The thread ID
   * @return Const reference to the user object
   */
  virtual const UserObject & getUserObject(const std::string & name, THREAD_ID tid = 0);

  // Transient /////
  virtual void copySolutionsBackwards() = 0;

  // Output system /////

  virtual void output(bool force = false) = 0;
  void outputInitial(bool out_init) { _output_initial = out_init; }

  // time periods

  /**
   * Add a time period
   * @param name Name of the time period
   * @param start_time Start time of the time period
   * @param end_time End time of the time period
   */
  void addTimePeriod(const std::string & name, Real start_time, Real end_time);

  /**
   * Get time period by name
   * @param name Name of the time period to get
   * @return Pointer to the time period struct if found, otherwise NULL
   */
  virtual TimePeriod * getTimePeriodByName(const std::string & name);


public:
  /**
   * Convenience zeros
   */
  MooseArray<Real> _real_zero;
  MooseArray<MooseArray<Real> > _zero;
  MooseArray<MooseArray<RealGradient> > _grad_zero;
  MooseArray<MooseArray<RealTensor> > _second_zero;

protected:
  /// The name of the problem
  std::string _name;
  /// Generic parameters object used during construction
  InputParameters _pars;

  /// functions
  std::vector<std::map<std::string, Function *> > _functions;
  /// User objects
  std::vector<UserObjectWarehouse> _user_objects;

  /// output initial condition if true
  bool _output_initial;

  /// Time periods by name
  std::map<std::string, TimePeriod *> _time_periods;
};

#endif /* PROBLEM_H */
