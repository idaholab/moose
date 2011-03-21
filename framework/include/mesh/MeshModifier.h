#ifndef MESHMODIFIER_H
#define MESHMODIFIER_H

#include "Object.h"
#include "mesh.h"

class MeshModifier : public Object
{
public:
  MeshModifier(const std::string & name, InputParameters parameters);

  virtual void modifyMesh(Mesh & mesh) = 0;
  
};

#endif /* MESHMODIFIER_H */
