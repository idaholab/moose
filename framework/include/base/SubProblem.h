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

#ifndef SUBPROBLEM_H
#define SUBPROBLEM_H

#include "Moose.h"
#include "ParallelUniqueId.h"
#include "Problem.h"
#include "DiracKernelInfo.h"
#include "Assembly.h"
#include "GeometricSearchData.h"

// libMesh include
#include "equation_systems.h"
#include "transient_system.h"
#include "nonlinear_implicit_system.h"
#include "numeric_vector.h"
#include "sparse_matrix.h"

class MooseMesh;

class SubProblem;

template<>
InputParameters validParams<SubProblem>();

/**
 * Generic class for solving transient nonlinear problems
 *
 */
class SubProblem : public Problem
{
public:
  SubProblem(const std::string & name, InputParameters parameters);
  virtual ~SubProblem();

  virtual Problem * parent() { return _parent; }
  virtual EquationSystems & es() { return _eq; }
  virtual MooseMesh & mesh() { return _mesh; }

  Moose::CoordinateSystemType & coordSystem() { return _coord_sys; }
  virtual void setCoordSystem(Moose::CoordinateSystemType type) { _coord_sys = type; }

  /**
   * Whether or not this problem should utilize FE shape function caching.
   *
   * @param fe_cache True for using the cache false for not.
   */
  virtual void useFECache(bool fe_cache) = 0;

  virtual void init();
  virtual void solve() = 0;
  virtual bool converged() = 0;

  virtual void onTimestepBegin() = 0;
  virtual void onTimestepEnd() = 0;

  virtual Real & time() { return _time; }
  virtual int & timeStep() { return _t_step; }
  virtual Real & dt() { return _dt; }
  virtual Real & dtOld() { return _dt_old; }

  virtual void transient(bool trans) { _transient = trans; }
  virtual bool isTransient() { return _transient; }

  virtual std::vector<Real> & timeWeights() { return _time_weights; }

  virtual Order getQuadratureOrder() = 0;

  virtual Assembly & assembly(THREAD_ID tid) = 0;
  virtual void prepareShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid) = 0;

  virtual QBase * & qRule(THREAD_ID tid) = 0;
  virtual const MooseArray<Point> & points(THREAD_ID tid) = 0;
  virtual const MooseArray<Point> & physicalPoints(THREAD_ID tid) = 0;
  virtual const MooseArray<Real> & JxW(THREAD_ID tid) = 0;
  virtual const Real & elemVolume(THREAD_ID tid) = 0;
  virtual const MooseArray<Real> & coords(THREAD_ID tid) = 0;
  virtual QBase * & qRuleFace(THREAD_ID tid) = 0;
  virtual const MooseArray<Point> & pointsFace(THREAD_ID tid) = 0;
  virtual const MooseArray<Real> & JxWFace(THREAD_ID tid) = 0;
  virtual const Real & sideElemVolume(THREAD_ID tid) = 0;
  virtual const Elem * & elem(THREAD_ID tid) = 0;
  virtual unsigned int & side(THREAD_ID tid) = 0;
  virtual const Elem * & sideElem(THREAD_ID tid) = 0;
  virtual const Node * & node(THREAD_ID tid) = 0;
  virtual const Node * & nodeNeighbor(THREAD_ID tid) = 0;
  virtual DiracKernelInfo & diracKernelInfo() { return _dirac_kernel_info; }
  virtual Real finalNonlinearResidual() { return 0; }
  virtual unsigned int nNonlinearIterations() { return 0; }
  virtual unsigned int nLinearIterations() { return 0; }

  // Geom Search
  virtual void updateGeomSearch() = 0;
  virtual GeometricSearchData & geomSearchData() = 0;

  virtual void meshChanged() { mooseError("This system does not support changing the mesh"); }

  virtual void storeMatPropName(SubdomainID block_id, const std::string & name);
  virtual void checkMatProp(SubdomainID block_id, const std::string & name);

  /**
   * Will make sure that all dofs connected to elem_id are ghosted to this processor
   */
  virtual void addGhostedElem(unsigned int elem_id) = 0;

  /**
   * Will make sure that all necessary elements from boundary_id are ghosted to this processor
   */
  virtual void addGhostedBoundary(BoundaryID boundary_id) = 0;

  /**
   * Get a vector containing the block ids the material property is defined on.
   */
  virtual std::vector<SubdomainID> getMaterialPropertyBlocks(const std::string prop_name);

  /**
   * Get a vector of block id equivalences that the material property is defined on.
   */
  virtual std::vector<SubdomainName> getMaterialPropertyBlockNames(const std::string prop_name);

protected:
  Problem * _parent;
  MooseMesh & _mesh;
  EquationSystems & _eq;

  /// Type of coordinate system
  Moose::CoordinateSystemType _coord_sys;

  bool _transient;
  Real & _time;
  int & _t_step;
  Real & _dt;
  Real _dt_old;

  std::vector<Real> _time_weights;

  DiracKernelInfo _dirac_kernel_info;

  /// the map of material properties (block_id -> list of properties)
  std::map<unsigned int, std::set<std::string> > _map_material_props;
};


namespace Moose
{

void initial_condition(EquationSystems & es, const std::string & system_name);

} // namespace Moose


#endif /* SUBPROBLEM_H */
