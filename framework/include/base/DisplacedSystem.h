#ifndef DISPLACEDSYSTEM_H_
#define DISPLACEDSYSTEM_H_

#include "System.h"

// libMesh include
#include "explicit_system.h"
#include "transient_system.h"

namespace Moose
{

class DisplacedSystem : public SystemTempl<TransientExplicitSystem>
{
public:
  DisplacedSystem(ProblemInterface & problem, const std::string & name);
  virtual ~DisplacedSystem();

  virtual void prepare(THREAD_ID tid);
  virtual void reinitElem(const Elem * elem, THREAD_ID tid);
  virtual void reinitElemFace(const Elem * elem, unsigned int side, unsigned int bnd_id, THREAD_ID tid);
  virtual void reinitNode(const Node * node, THREAD_ID tid);
  virtual void reinitNodeFace(const Node * node, unsigned int bnd_id, THREAD_ID tid);
};

}

#endif /* DISPLACEDSYSTEM_H_ */
