#ifndef MESHGENERATIONBLOCK_H
#define MESHGENERATIONBLOCK_H

#include "ParserBlock.h"

//Forward Declarations
class MeshGenerationBlock;

template<>
InputParameters validParams<MeshGenerationBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<int>("nx", 1, "Number of elements in the X direction");
  params.addParam<int>("ny", 1, "Number of elements in the Y direction");
  params.addParam<int>("nz", 1, "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<std::string>("elem_type", "QUAD4", "The type of element from libMesh to generate");
  return params;
}

class MeshGenerationBlock: public ParserBlock
{
public:
  MeshGenerationBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params);

  virtual void execute();
  
private:
  bool _executed;
};

#endif //MESHGENERATIONBLOCK_H
