//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FillBetweenPointVectorsGenerator.h"
#include "FillBetweenPointVectorsTools.h"

registerMooseObject("MooseApp", FillBetweenPointVectorsGenerator);

InputParameters
FillBetweenPointVectorsGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<std::vector<Point>>("positions_vector_1",
                                              "Array of the node positions of the first boundary.");
  params.addRequiredParam<std::vector<Point>>(
      "positions_vector_2", "Array of the node positions of the second boundary.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_layers", "num_layers>0", "Layers of elements for transition.");
  params.addParam<subdomain_id_type>("block_id", 1, "ID to be assigned to the block.");
  params.addParam<boundary_id_type>(
      "input_boundary_1_id",
      10000,
      "Boundary ID to be assigned to the boundary defined by positions_vector_1.");
  params.addParam<boundary_id_type>(
      "input_boundary_2_id",
      10000,
      "Boundary ID to be assigned to the boundary defined by positions_vector_2.");
  params.addParam<boundary_id_type>("begin_side_boundary_id",
                                    10000,
                                    "Boundary ID to be assigned to the boundary connecting "
                                    "starting points of the positions_vectors.");
  params.addParam<boundary_id_type>("end_side_boundary_id",
                                    10000,
                                    "Boundary ID to be assigned to the boundary connecting ending "
                                    "points of the positions_vectors.");
  params.addParam<bool>(
      "use_quad_elements",
      false,
      "Whether QUAD4 instead of TRI3 elements are used to construct the transition layer.");
  params.addRangeCheckedParam<Real>(
      "bias_parameter",
      1.0,
      "bias_parameter>=0",
      "Parameter used to set up biasing of the layers: bias_parameter > 0.0 is used as the biasing "
      "factor; bias_parameter = 0.0 activates automatic biasing based on local node density on "
      "both input boundaries.");
  params.addRangeCheckedParam<Real>(
      "gaussian_sigma",
      3.0,
      "gaussian_sigma>0.0",
      "Gaussian parameter used to smoothen local node density for automatic biasing; this "
      "parameter is not used if other biasing option is selected.");
  params.addClassDescription(
      "This FillBetweenPointVectorsGenerator object is designed to generate a "
      "transition layer with two sides containing different numbers of nodes.");
  return params;
}

FillBetweenPointVectorsGenerator::FillBetweenPointVectorsGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _positions_vector_1(getParam<std::vector<Point>>("positions_vector_1")),
    _positions_vector_2(getParam<std::vector<Point>>("positions_vector_2")),
    _num_layers(getParam<unsigned int>("num_layers")),
    _block_id(getParam<subdomain_id_type>("block_id")),
    _input_boundary_1_id(getParam<boundary_id_type>("input_boundary_1_id")),
    _input_boundary_2_id(getParam<boundary_id_type>("input_boundary_2_id")),
    _begin_side_boundary_id(getParam<boundary_id_type>("begin_side_boundary_id")),
    _end_side_boundary_id(getParam<boundary_id_type>("end_side_boundary_id")),
    _use_quad_elements(getParam<bool>("use_quad_elements")),
    _bias_parameter(getParam<Real>("bias_parameter")),
    _sigma(getParam<Real>("gaussian_sigma"))
{
}

std::unique_ptr<MeshBase>
FillBetweenPointVectorsGenerator::generate()
{
  auto mesh = buildReplicatedMesh(2);
  FillBetweenPointVectorsTools::fillBetweenPointVectorsGenerator(*mesh,
                                                                 _positions_vector_2,
                                                                 _positions_vector_1,
                                                                 _num_layers,
                                                                 _block_id,
                                                                 _input_boundary_2_id,
                                                                 _input_boundary_1_id,
                                                                 _begin_side_boundary_id,
                                                                 _end_side_boundary_id,
                                                                 _type,
                                                                 _name,
                                                                 _use_quad_elements,
                                                                 _bias_parameter,
                                                                 _sigma);
  return mesh;
}
