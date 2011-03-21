#ifndef PROBLEMINTERFACE_H_
#define PROBLEMINTERFACE_H_

#include <string>
#include "Variable.h"
#include "ParallelUniqueId.h"
#include "Output.h"
// libMesh
#include "elem.h"
#include "node.h"
#include "mesh.h"
#include "equation_systems.h"

namespace Moose
{

class Mesh;
class Variable;

class ProblemInterface
{
public:
  virtual EquationSystems & es() = 0;
  virtual Mesh & mesh() = 0;
  virtual Problem * parent() = 0;

  // Variables /////
  virtual bool hasVariable(const std::string & var_name) = 0;
  virtual Variable & getVariable(THREAD_ID tid, const std::string & var_name) = 0;

  virtual AssemblyData & assembly(THREAD_ID tid) = 0;
  virtual void subdomainSetup(unsigned int subdomain, THREAD_ID tid) = 0;

  virtual void prepare(const Elem * elem, THREAD_ID tid) = 0;
  virtual void reinitElem(const Elem * elem, THREAD_ID tid) = 0;
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid) = 0;
  virtual void reinitNode(const Node * node, THREAD_ID tid) = 0;
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid) = 0;

  // Transient /////
  virtual bool transient() = 0;
};

}

#endif /* PROBLEMINTERFACE_H_ */
