#ifndef MESHBLOCK_H_
#define MESHBLOCK_H_

#include "ParserBlock.h"

namespace libMesh
{
  class MeshRefinement;
  class Mesh;
}


//Forward Declarations
class GenericVariableBlock;

class MeshBlock : public ParserBlock
{
public:
  MeshBlock(const std::string & name, InputParameters params);

  virtual void execute();

private:
  bool autoResizeProblem(Mesh *mesh, MeshRefinement &mesh_refinement);
  bool checkVariableProperties(bool (GenericVariableBlock::*property)() const);
};

template<>
InputParameters validParams<MeshBlock>();


#endif //MESHBLOCK_H
