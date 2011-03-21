#ifndef MESHMODIFIER_H
#define MESHMODIFIER_H

#include "MooseObject.h"
#include "MooseMesh.h"

class MeshModifier : public MooseObject
{
public:
  MeshModifier(const std::string & name, InputParameters parameters);

  virtual void modifyMesh(Mesh & mesh) = 0;
  
};

#endif /* MESHMODIFIER_H */
