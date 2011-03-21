#ifndef SUBPROBLEMINTERFACE_H
#define SUBPROBLEMINTERFACE_H

// Moose
#include "MooseVariable.h"
#include "ParallelUniqueId.h"
#include "VariableWarehouse.h"
#include "DiracKernelInfo.h"

// libMesh
#include "elem.h"
#include "node.h"
#include "MooseMesh.h"
#include "equation_systems.h"

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

  virtual AssemblyData & assembly(THREAD_ID tid) = 0;
  virtual QBase * & qRule(THREAD_ID tid) = 0;
  virtual const std::vector<Point> & points(THREAD_ID tid) = 0;
  virtual const std::vector<Real> & JxW(THREAD_ID tid) = 0;
  virtual QBase * & qRuleFace(THREAD_ID tid) = 0;
  virtual const std::vector<Point> & pointsFace(THREAD_ID tid) = 0;
  virtual const std::vector<Real> & JxWFace(THREAD_ID tid) = 0;
  virtual const Elem * & elem(THREAD_ID tid) = 0;
  virtual unsigned int & side(THREAD_ID tid) = 0;
  virtual const Elem * & sideElem(THREAD_ID tid) = 0;
  virtual const Node * & node(THREAD_ID tid) = 0;
  virtual DiracKernelInfo & diracKernelInfo() { return _dirac_kernel_info; }

  // Geom Search
  virtual GeometricSearchData & geomSearchData() = 0;

protected:
  DiracKernelInfo _dirac_kernel_info;
};

#endif /* SUBPROBLEMINTERFACE_H */
