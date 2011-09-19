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

#include "CreateMeshAction.h"
#include "Moose.h"
#include "Parser.h"
#include "MooseMesh.h"

// libMesh includes
#include "getpot.h"
#include "mesh_generation.h"
#include "string_to_enum.h"


template<>
InputParameters validParams<CreateMeshAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<int>("dim", "The dimension of the mesh to be generated");
  params.addParam<int>("nx", 1, "Number of elements in the X direction");
  params.addParam<int>("ny", 1, "Number of elements in the Y direction");
  params.addParam<int>("nz", 1, "Number of elements in the Z direction");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");
  params.addParam<std::string>("elem_type", "The type of element from libMesh to generate");
  return params;
}


CreateMeshAction::CreateMeshAction(const std::string & name, InputParameters params) :
    Action(name, params)
{
}

void
CreateMeshAction::act()
{
   int mesh_dim = -1;
   std::string elem_type_str;

  // dim must be passed in the Generation block - we are no longer retrieving the dimension
  // from the parent block
  mesh_dim = getParam<int>("dim");
  if (isParamValid("elem_type"))
    elem_type_str = getParam<std::string>("elem_type");
  else
    switch (mesh_dim)
    {
    case 1: elem_type_str = "EDGE2"; break;
    case 2: elem_type_str = "QUAD4"; break;
    case 3: elem_type_str = "HEX8"; break;
    default:
      mooseError("Unable to generate mesh for unknown dimension");
    }

  ElemType elem_type = Utility::string_to_enum<ElemType>(elem_type_str);

   MooseMesh *mesh = new MooseMesh(mesh_dim);

  switch (mesh_dim)
  {
  case 1:
    MeshTools::Generation::build_line(*mesh,
                                      getParam<int>("nx"),
                                      getParam<Real>("xmin"),
                                      getParam<Real>("xmax"),
                                      elem_type);
    break;
  case 2:
    MeshTools::Generation::build_square(*mesh,
                                        getParam<int>("nx"),
                                        getParam<int>("ny"),
                                        getParam<Real>("xmin"),
                                        getParam<Real>("xmax"),
                                        getParam<Real>("ymin"),
                                        getParam<Real>("ymax"),
                                        elem_type);
    break;
  case 3:
    MeshTools::Generation::build_cube(*mesh,
                                      getParam<int>("nx"),
                                      getParam<int>("ny"),
                                      getParam<int>("nz"),
                                      getParam<Real>("xmin"),
                                      getParam<Real>("xmax"),
                                      getParam<Real>("ymin"),
                                      getParam<Real>("ymax"),
                                      getParam<Real>("zmin"),
                                      getParam<Real>("zmax"),
                                      elem_type);
    break;
   }

  _parser_handle._mesh = mesh;

}
