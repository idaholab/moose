#include <string>

// Moose Includes
#include "Moose.h"
#include "MeshGenerationBlock.h"

// libMesh includes
#include "mesh.h"
#include "getpot.h"
#include "mesh_generation.h"
#include "string_to_enum.h"

MeshGenerationBlock::MeshGenerationBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, const GetPot & input_file)
  :ParserBlock(reg_id, real_id, parent, input_file),
   _executed(false)
{
  _block_params.set<int>("nx") = 1;
  _block_params.set<int>("ny") = 1;
  _block_params.set<int>("nz") = 1;
  _block_params.set<Real>("xmin") = 0.0;
  _block_params.set<Real>("ymin") = 0.0;
  _block_params.set<Real>("zmin") = 0.0;
  _block_params.set<Real>("xmax") = 1.0;
  _block_params.set<Real>("ymax") = 1.0;
  _block_params.set<Real>("zmax") = 2.0;
  _block_params.set<std::string>("elem_type") = "QUAD4";
}

void
MeshGenerationBlock::execute() 
{
  if (_executed)
    return;
  
#ifdef DEBUG
  std::cerr << "Inside the MeshGenerationBlock Object\n";
#endif

  ElemType elem_type = Utility::string_to_enum<ElemType>(_block_params.get<std::string>("elem_type"));
  unsigned int mesh_dim = Moose::mesh->mesh_dimension();

  switch (mesh_dim)
  {
  case 1:
    MeshTools::Generation::build_line(*Moose::mesh,
                                      _block_params.get<int>("nx"),
                                      _block_params.get<Real>("xmin"),
                                      _block_params.get<Real>("xmax"),
                                      elem_type);
    break;
  case 2:
    MeshTools::Generation::build_square(*Moose::mesh,
                                        _block_params.get<int>("nx"),
                                        _block_params.get<int>("ny"),
                                        _block_params.get<Real>("xmin"),
                                        _block_params.get<Real>("xmax"),
                                        _block_params.get<Real>("ymin"),
                                        _block_params.get<Real>("ymax"),
                                        elem_type);
    break;
  case 3:
    MeshTools::Generation::build_cube(*Moose::mesh,
                                      _block_params.get<int>("nx"),
                                      _block_params.get<int>("ny"),
                                      _block_params.get<int>("nz"),
                                      _block_params.get<Real>("xmin"),
                                      _block_params.get<Real>("xmax"),
                                      _block_params.get<Real>("ymin"),
                                      _block_params.get<Real>("ymax"),
                                      _block_params.get<Real>("zmin"),
                                      _block_params.get<Real>("zmax"),
                                      elem_type);
    break;
  default:
    std::cerr << "Unable to generate mesh for unknown dimension\n";
    mooseError();
  }
  
  visitChildren();

  _executed = true;
}
