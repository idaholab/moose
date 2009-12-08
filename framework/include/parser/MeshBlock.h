#ifndef MESHBLOCK_H
#define MESHBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class GenericVariableBlock;
class MeshBlock;
class MeshRefinement;
class Mesh;

template<>
InputParameters validParams<MeshBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<int>("dim", -1, "The dimension of the mesh file to read or generate", true);
  params.addParam<std::string>("file", "", "The name of the mesh file to read (required unless using dynamic generation)", false);
  params.addParam<bool>("second_order", false, "Turns on second order elements for the input mesh", false);
  params.addParam<bool>("generated", false, "Tell MOOSE that a mesh will be generated", false);
  params.addParam<std::string>("partitioner", "", "Specifies a mesh partitioner to use when spliting the mesh for a parallel computation", false);
  params.addParam<int>("uniform_refine", 0, "Specify the level of uniform refinement applied to the initial mesh", false);
  return params;
}

class MeshBlock: public ParserBlock
{
public:
  MeshBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();

private:
  bool autoResizeProblem(Mesh *mesh, MeshRefinement &mesh_refinement);
  bool checkVariableProperties(bool (GenericVariableBlock::*property)() const);
};

  

#endif //MESHBLOCK_H
