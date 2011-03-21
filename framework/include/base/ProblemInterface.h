#ifndef PROBLEMINTERFACE_H
#define PROBLEMINTERFACE_H

#include <string>
#include "MooseVariable.h"
#include "ParallelUniqueId.h"
#include "Output.h"
// libMesh
#include "elem.h"
#include "node.h"
#include "MooseMesh.h"
#include "equation_systems.h"

class MooseMesh;
class MooseVariable;
class GeometricSearchData;

class ProblemInterface
{
public:
  virtual EquationSystems & es() = 0;
  virtual MooseMesh & mesh() = 0;
  virtual Problem * parent() = 0;

  // Variables /////
  virtual bool hasVariable(const std::string & var_name) = 0;
  virtual MooseVariable & getVariable(THREAD_ID tid, const std::string & var_name) = 0;

  virtual AssemblyData & assembly(THREAD_ID tid) = 0;
  virtual void subdomainSetup(unsigned int subdomain, THREAD_ID tid) = 0;

  virtual void prepare(const Elem * elem, THREAD_ID tid) = 0;
  virtual void reinitElem(const Elem * elem, THREAD_ID tid) = 0;
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid) = 0;
  virtual void reinitNode(const Node * node, THREAD_ID tid) = 0;
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid) = 0;

  // Transient /////
  virtual bool transient() = 0;

  // Geom Search
  virtual GeometricSearchData & geomSearchData() = 0;
};

#endif /* PROBLEMINTERFACE_H */
