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
class Function;
class TimePeriod;
class Problem;

template<>
InputParameters validParams<Problem>();

/**
 * Class that hold the whole problem being solved.
 */
class Problem
{
public:
  Problem(const std::string & name, InputParameters parameters);
  virtual ~Problem();

  /**
   * Get the name of this problem
   * @return The name of this problem
   */
  virtual const std::string & name() { return _name; }
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
  virtual void reinitMaterialsBoundary(BoundaryID boundary_id, THREAD_ID tid) = 0;
  virtual void reinitMaterialsNeighbor(SubdomainID /*blk_id*/, unsigned int /*side*/, THREAD_ID /*tid*/) { mooseError("Not implemented yet."); }

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
  virtual void outputPostprocessors(bool force = false) = 0;
  virtual Real & getPostprocessorValue(const std::string & name, THREAD_ID tid = 0) = 0;


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
  TimePeriod & addTimePeriod(const std::string & name, Real start_time);

  /**
   * Get time period by name
   * @param name Name of the time period to get
   * @return Pointer to the time period struct if found, otherwise NULL
   */
  virtual TimePeriod * getTimePeriodByName(const std::string & name);

  const std::vector<TimePeriod *> & getTimePeriods() const;

protected:
  /// The name of the problem
  std::string _name;
  /// Generic parameters object used during construction
  InputParameters _pars;

  /// output initial condition if true
  bool _output_initial;

  /// Time periods
  std::vector<TimePeriod *> _time_periods;
};

#endif /* PROBLEM_H */
