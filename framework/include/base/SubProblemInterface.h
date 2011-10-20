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

#ifndef SUBPROBLEMINTERFACE_H
#define SUBPROBLEMINTERFACE_H

// Moose
#include "MooseVariable.h"
#include "ParallelUniqueId.h"
#include "VariableWarehouse.h"
#include "DiracKernelInfo.h"
#include "MooseMesh.h"
#include "AsmBlock.h"
// libMesh
#include "elem.h"
#include "node.h"
#include "equation_systems.h"
#include "dense_subvector.h"
#include "dense_submatrix.h"
// system
#include <string>

class MooseMesh;
class GeometricSearchData;
class Problem;

class SubProblemInterface
{
public:
  virtual EquationSystems & es() = 0;
  virtual MooseMesh & mesh() = 0;
  virtual Problem * parent() = 0;

  virtual AsmBlock & asmBlock(THREAD_ID tid) = 0;
  virtual void addResidual(NumericVector<Number> & residual, THREAD_ID tid) = 0;
  virtual void addResidualNeighbor(NumericVector<Number> & residual, THREAD_ID tid) = 0;
  virtual void addJacobian(SparseMatrix<Number> & jacobian, THREAD_ID tid) = 0;
  virtual void addJacobianNeighbor(SparseMatrix<Number> & jacobian, THREAD_ID tid) = 0;
  virtual void prepareShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual void prepareFaceShapes(unsigned int var, THREAD_ID tid) = 0;
  virtual void prepareNeighborShapes(unsigned int var, THREAD_ID tid) = 0;

  virtual AssemblyData & assembly(THREAD_ID tid) = 0;
  virtual QBase * & qRule(THREAD_ID tid) = 0;
  virtual const std::vector<Point> & points(THREAD_ID tid) = 0;
  virtual const std::vector<Point> & physicalPoints(THREAD_ID tid) = 0;
  virtual const std::vector<Real> & JxW(THREAD_ID tid) = 0;
  virtual QBase * & qRuleFace(THREAD_ID tid) = 0;
  virtual const std::vector<Point> & pointsFace(THREAD_ID tid) = 0;
  virtual const std::vector<Real> & JxWFace(THREAD_ID tid) = 0;
  virtual const Elem * & elem(THREAD_ID tid) = 0;
  virtual unsigned int & side(THREAD_ID tid) = 0;
  virtual const Elem * & sideElem(THREAD_ID tid) = 0;
  virtual const Node * & node(THREAD_ID tid) = 0;
  virtual DiracKernelInfo & diracKernelInfo() { return _dirac_kernel_info; }
  virtual Real finalNonlinearResidual() { return 0; }
  virtual unsigned int nNonlinearIterations() { return 0; }
  virtual unsigned int nLinearIterations() { return 0; }

  // Geom Search
  virtual void updateGeomSearch() = 0;
  virtual GeometricSearchData & geomSearchData() = 0;

  virtual void meshChanged() { mooseError("This system does not support changing the mesh"); }

  virtual void storeMatPropName(unsigned int block_id, const std::string & name);
  virtual void checkMatProp(unsigned int block_id, const std::string & name);

  // Transient
  virtual Real & time() = 0;
  virtual int & timeStep() = 0;
  virtual Real & dt() = 0;
  virtual Real & dtOld() = 0;

  virtual void transient(bool trans) = 0;
  virtual bool isTransient() = 0;

  virtual std::vector<Real> & timeWeights() = 0;

protected:
  DiracKernelInfo _dirac_kernel_info;

  /// the map of material properties (block_id -> list of properties)
  std::map<unsigned int, std::set<std::string> > _map_material_props;
};

#endif /* SUBPROBLEMINTERFACE_H */
