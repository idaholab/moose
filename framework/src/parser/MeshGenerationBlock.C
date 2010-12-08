/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include <string>

// Moose Includes
#include "Moose.h"
#include "MeshGenerationBlock.h"

// libMesh includes
#include "mesh.h"
#include "getpot.h"
#include "mesh_generation.h"
#include "string_to_enum.h"

template<>
InputParameters validParams<MeshGenerationBlock>()
{
  InputParameters params = validParams<ParserBlock>();
  params.addParam<int>("dim", "The dimension of the mesh to be generated - Required to appear in this block or parent");
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

MeshGenerationBlock::MeshGenerationBlock(const std::string & name, InputParameters params)
  :ParserBlock(name, params),
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
  int mesh_dim = -1;

  if (isParamValid("dim"))
    mesh_dim = getParamValue<int>("dim");
  else if (_parent->isParamValid("dim"))
    mesh_dim = _parent->getParamValue<int>("dim");
  else
    mooseError("You must provide ""dim"" in the Mesh or Mesh/Generation block");
  
  Mesh *mesh = _moose_system.getMesh(true);
  
  switch (mesh_dim)
  {
  case 1:
    MeshTools::Generation::build_line(*mesh,
                                      getParamValue<int>("nx"),
                                      getParamValue<Real>("xmin"),
                                      getParamValue<Real>("xmax"),
                                      elem_type);
    break;
  case 2:
    MeshTools::Generation::build_square(*mesh,
                                        getParamValue<int>("nx"),
                                        getParamValue<int>("ny"),
                                        getParamValue<Real>("xmin"),
                                        getParamValue<Real>("xmax"),
                                        getParamValue<Real>("ymin"),
                                        getParamValue<Real>("ymax"),
                                        elem_type);
    break;
  case 3:
    MeshTools::Generation::build_cube(*mesh,
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
