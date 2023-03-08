//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestMeshMetaData.h"

registerMooseObject("MooseTestApp", TestMeshMetaData);

InputParameters
TestMeshMetaData::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  return params;
}

TestMeshMetaData::TestMeshMetaData(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getParam<MeshGeneratorName>("input")),
    _input_mesh(getMeshByName(_input))
{
}

std::unique_ptr<MeshBase>
TestMeshMetaData::generate()
{
  checkMeshMetaData<Real>("rs_1", 1.234);
  checkMeshMetaData<Real>("rs_2", 12.34);
  checkMeshMetaData<unsigned int>("uis_1", 1234);
  checkMeshMetaData<int>("is_1", -1234);
  checkMeshMetaData<int>("is_2", 4321);
  checkMeshMetaData<int>("is_3", -5678);
  checkMeshMetaData<dof_id_type>("ds_1", 12);
  checkMeshMetaData<dof_id_type>("ds_2", 123);
  checkMeshMetaData<subdomain_id_type>("ss_1", 21);
  checkMeshMetaData<subdomain_id_type>("ss_2", 321);
  checkMeshMetaData<bool>("bs_1", false);
  checkMeshMetaData<bool>("bs_2", true);
  checkMeshMetaData<bool>("bs_3", false);
  checkMeshMetaData<Point>("ps_1", Point(1.0, -2.0, 3.5));
  checkMeshMetaData<Point>("ps_2", Point(-2.0, 3.0, -1.5));

  checkMeshMetaData<std::vector<Real>>("rv_1", {1.234, 12.34, 123.4});
  checkMeshMetaData<std::vector<Real>>("rv_2", {4.321, 43.21});
  checkMeshMetaData<std::vector<unsigned int>>("uiv_1", {1234, 567, 89});
  checkMeshMetaData<std::vector<int>>("iv_1", {-1234, 4321});
  checkMeshMetaData<std::vector<int>>("iv_2", {-567, 89});
  checkMeshMetaData<std::vector<int>>("iv_3", {98, 76, 54});
  checkMeshMetaData<std::vector<dof_id_type>>("dv_1", {12, 123});
  checkMeshMetaData<std::vector<dof_id_type>>("dv_2", {45, 678, 9});
  checkMeshMetaData<std::vector<subdomain_id_type>>("sv_1", {21, 321});
  checkMeshMetaData<std::vector<subdomain_id_type>>("sv_2", {9, 876, 54});
  checkMeshMetaData<std::vector<Point>>("pv_1", {Point(2.0, -1.0, 3.0), Point(2.0, 2.0, -1.0)});
  checkMeshMetaData<std::vector<Point>>(
      "pv_2", {Point(5.0, -2.0, 3.0), Point(1.0, -3.0, -2.0), Point(4.0, -1.0, -2.5)});

  return std::move(_input_mesh);
}

template <class T>
void
TestMeshMetaData::checkMeshMetaData(const std::string mesh_data_name, const T ref_data)
{
  if (hasMeshProperty<float>(mesh_data_name, _input))
    mooseError("Mesh metadata ", mesh_data_name, " of incorrect type was found");
  if (!hasMeshProperty(mesh_data_name, _input))
    mooseError("Mesh metadata ", mesh_data_name, " is missing");
  if (!hasMeshProperty<T>(mesh_data_name, _input))
    mooseError("Mesh metadata ",
               mesh_data_name,
               " of type ",
               MooseUtils::prettyCppType<T>(),
               " is missing");
  if (getMeshProperty<T>(mesh_data_name, _input) != ref_data)
    mooseError("Mesh metadata " + mesh_data_name + " has an incorrect value.");
}
