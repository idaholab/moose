#ifndef MESHBLOCK_H
#define MESHBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericVariableBlock;
class MeshBlock;

namespace libMesh
{
  class MeshRefinement;
  class Mesh;
}

template<>
InputParameters validParams<MeshBlock>();

class MeshBlock: public ParserBlock
{
public:
  MeshBlock(std::string name, MooseSystem & moose_system, InputParameters params);

  virtual void execute();

private:
  bool autoResizeProblem(Mesh *mesh, MeshRefinement &mesh_refinement);
  bool checkVariableProperties(bool (GenericVariableBlock::*property)() const);
};

  

#endif //MESHBLOCK_H
