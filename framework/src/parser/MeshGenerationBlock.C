#include <string>

// Moose Includes
#include "Moose.h"
#include "MeshGenerationBlock.h"

// libMesh includes
#include "mesh.h"
#include "getpot.h"
#include "mesh_generation.h"
#include "string_to_enum.h"

MeshGenerationBlock::MeshGenerationBlock(const std::string & reg_id, const std::string & real_id, ParserBlock * parent, Parser & parser_handle, InputParameters params)
  :ParserBlock(reg_id, real_id, parent, parser_handle, params),
   _executed(false)
{}

void
MeshGenerationBlock::execute() 
{
  if (_executed)
    return;
  
#ifdef DEBUG
  std::cerr << "Inside the MeshGenerationBlock Object\n";
#endif

  ElemType elem_type = Utility::string_to_enum<ElemType>(getParamValue<std::string>("elem_type"));
  unsigned int mesh_dim = Moose::mesh->mesh_dimension();

  switch (mesh_dim)
  {
  case 1:
    MeshTools::Generation::build_line(*Moose::mesh,
                                      getParamValue<int>("nx"),
                                      getParamValue<Real>("xmin"),
                                      getParamValue<Real>("xmax"),
                                      elem_type);
    break;
  case 2:
    MeshTools::Generation::build_square(*Moose::mesh,
                                        getParamValue<int>("nx"),
                                        getParamValue<int>("ny"),
                                        getParamValue<Real>("xmin"),
                                        getParamValue<Real>("xmax"),
                                        getParamValue<Real>("ymin"),
                                        getParamValue<Real>("ymax"),
                                        elem_type);
    break;
  case 3:
    MeshTools::Generation::build_cube(*Moose::mesh,
                                      getParamValue<int>("nx"),
                                      getParamValue<int>("ny"),
                                      getParamValue<int>("nz"),
                                      getParamValue<Real>("xmin"),
                                      getParamValue<Real>("xmax"),
                                      getParamValue<Real>("ymin"),
                                      getParamValue<Real>("ymax"),
                                      getParamValue<Real>("zmin"),
                                      getParamValue<Real>("zmax"),
                                      elem_type);
    break;
  default:
    std::cerr << "Unable to generate mesh for unknown dimension\n";
    mooseError("");
  }
  
  visitChildren();

  _executed = true;
}
