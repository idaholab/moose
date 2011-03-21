#ifndef ELEMENTDELETER_H
#define ELEMENTDELETER_H

#include "MeshModifier.h"
#include "Function.h"
#include "FunctionInterface.h"

class ElementDeleter : public MeshModifier
// TODO: Make this work
//                     , FunctionInterface
{
public:
  ElementDeleter(const std::string & name, InputParameters parameters);

  virtual void modifyMesh(Mesh & mesh);

private:
  void removeAllElemBCs(Mesh & mesh, Elem * elem);
};

#endif /* ELEMENTDELETER_H */
