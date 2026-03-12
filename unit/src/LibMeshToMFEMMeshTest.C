//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "mfem/config/config.hpp"
#include <stdexcept>
#ifdef MOOSE_MFEM_ENABLED

#include "mfem/fem/gridfunc.hpp"
#include "gtest/gtest.h"
#include "GeneratedMesh.h"
#include "MeshGeneratorMesh.h"
#include "ElementGenerator.h"
#include "FileMesh.h"
#include "MooseError.h"
#include "MooseMain.h"
#include "MooseTypes.h"
#include "Registry.h"
#include "MFEMMeshFactory.h"
#include "libmesh/enum_elem_type.h"
#include "mfem/fem/eltrans.hpp"
#include "mfem/fem/fespace.hpp"
#include "mfem/linalg/densemat.hpp"
#include "mfem/mesh/element.hpp"
#include "type_traits"

template <class M, bool fallback = false, bool first_order = false>
class LibMeshToMFEMMeshTest : public ::testing::Test
{
public:
  void SetUp() override
  {
    const char * argv[2] = {"foo", "\0"};
    _app = Moose::createMooseApp("MooseUnitApp", 1, (char **)argv);
    _factory = &_app->getFactory();
    _mesh_type = Registry::getClassName<M>();
  }

  void TearDown() override
  {
    // Some tests changes this setting. We want to make sure it is
    // always changed back, even if the test fails to complete.
    Moose::_throw_on_warning = true;
  }

  std::shared_ptr<MooseApp> _app;
  Factory * _factory;
  std::string _mesh_type;
  std::shared_ptr<M> _moose_mesh_ptr;
  std::shared_ptr<mfem::ParMesh> _mfem_mesh_ptr;

  constexpr static bool _fallback = fallback;
  constexpr static bool _first_order = first_order;

  InputParameters getValidParams() { return _factory->getValidParams(_mesh_type); }

  void buildMesh(InputParameters & mesh_params)
  {
    // The input parameters in the third argument won't be used, but
    // we need a dummy value.
    buildMesh(mesh_params, "", _factory->getValidParams("ElementGenerator"));
  }

  void buildMesh(InputParameters & mesh_params,
                 std::string generator_class,
                 InputParameters generator_params)
  {
    // If the parameters were created without using the factory
    // object, we need to set the app parameter ourselves.
    mesh_params.addPrivateParam(MooseBase::app_param, _app.get());
    generator_params.addPrivateParam(MooseBase::app_param, _app.get());
    _moose_mesh_ptr = _factory->create<M>(_mesh_type, "moose_mesh", mesh_params);
    _app->actionWarehouse().mesh() = _moose_mesh_ptr;
    if (generator_class != "")
    {
      std::unique_ptr<MeshBase> libmesh =
          _factory->create<MeshGenerator>(generator_class, "mesh_generator", generator_params)
              ->generate();
      libmesh->prepare_for_use();
      _moose_mesh_ptr->setMeshBase(std::move(libmesh));
    }
    else
    {
      _moose_mesh_ptr->setMeshBase(_moose_mesh_ptr->buildMeshBaseObject());
    }
    _moose_mesh_ptr->buildMesh();
    _mfem_mesh_ptr = buildMFEMMesh(*_moose_mesh_ptr, fallback, first_order);
  }

  std::shared_ptr<M> buildAdditionalMesh(InputParameters & mesh_params,
                                         std::string generator_class,
                                         InputParameters generator_params)
  {
    // If the parameters were created without using the factory
    // object, we need to set the app parameter ourselves.
    mesh_params.addPrivateParam(MooseBase::app_param, _app.get());
    generator_params.addPrivateParam(MooseBase::app_param, _app.get());
    auto result = _factory->create<M>(_mesh_type, "moose_mesh2", mesh_params);
    // _app->actionWarehouse().mesh() = result;
    if (generator_class != "")
    {
      std::unique_ptr<MeshBase> libmesh =
          _factory->create<MeshGenerator>(generator_class, "mesh_generator2", generator_params)
              ->generate();
      libmesh->prepare_for_use();
      result->setMeshBase(std::move(libmesh));
    }
    else
    {
      result->setMeshBase(result->buildMeshBaseObject());
    }
    result->buildMesh();
    return result;
  }
};

template <int N>
using CoordND = std::array<mfem::real_t, N>;
using Coord1D = CoordND<1>;
using Coord2D = CoordND<2>;
using Coord3D = CoordND<3>;

template <int N>
std::set<std::set<CoordND<N>>>
getElementSet(std::shared_ptr<mfem::ParMesh> mesh, mfem::Element::Type elem_type)
{
  std::set<std::set<CoordND<N>>> actual_elements;
  for (int i = 0; i < mesh->GetNE(); ++i)
  {
    mfem::Element * elem = mesh->GetElement(i);
    EXPECT_EQ(elem->GetType(), elem_type);
    int * vertices = elem->GetVertices();
    std::set<CoordND<N>> vert_coords;
    for (int j = 0; j < elem->GetNVertices(); ++j)
    {
      mfem::real_t * vc = mesh->GetVertex(vertices[j]);
      CoordND<N> c;
      for (int k = 0; k < N; ++k)
      {
        c[k] = vc[k];
      }
      vert_coords.emplace(std::move(c));
    }
    actual_elements.emplace(std::move(vert_coords));
  }
  return actual_elements;
}

using GeneratedMeshMFEMTest = LibMeshToMFEMMeshTest<GeneratedMesh>;

TEST_F(GeneratedMeshMFEMTest, Check1D)
{
  InputParameters mesh_params = getValidParams();
  mesh_params.set<MooseEnum>("dim") = 1;
  mesh_params.set<unsigned int>("nx") = 3;
  mesh_params.set<double>("xmax") = 3.;
  buildMesh(mesh_params);
  EXPECT_EQ(_mfem_mesh_ptr->Dimension(), 1);
  EXPECT_EQ(_mfem_mesh_ptr->SpaceDimension(), 1);
  EXPECT_EQ(_mfem_mesh_ptr->GetNV(), 4);
  EXPECT_EQ(_mfem_mesh_ptr->GetNE(), 3);

  std::set<std::set<Coord1D>> actual_elements =
      getElementSet<1>(_mfem_mesh_ptr, mfem::Element::Type::SEGMENT);
  std::set<std::set<Coord1D>> expected_elements{{{0.}, {1.}}, {{1.}, {2.}}, {{2.}, {3.}}};
  EXPECT_EQ(expected_elements, actual_elements);
}

TEST_F(GeneratedMeshMFEMTest, Check2D)
{
  InputParameters mesh_params = getValidParams();
  mesh_params.set<MooseEnum>("dim") = 2;
  mesh_params.set<unsigned int>("nx") = 2;
  mesh_params.set<unsigned int>("ny") = 2;
  buildMesh(mesh_params);
  EXPECT_EQ(_mfem_mesh_ptr->Dimension(), 2);
  EXPECT_EQ(_mfem_mesh_ptr->SpaceDimension(), 2);
  EXPECT_EQ(_mfem_mesh_ptr->GetNV(), 9);
  EXPECT_EQ(_mfem_mesh_ptr->GetNE(), 4);
  EXPECT_EQ(_mfem_mesh_ptr->GetNEdges(), 12);

  std::set<std::set<Coord2D>> actual_elements =
      getElementSet<2>(_mfem_mesh_ptr, mfem::Element::Type::QUADRILATERAL);
  std::set<std::set<Coord2D>> expected_elements{{{0., 0.}, {0., .5}, {.5, 0.}, {.5, .5}},
                                                {{0.5, 0.}, {0.5, 0.5}, {1., 0.}, {1., 0.5}},
                                                {{0., 0.5}, {0., 1.}, {0.5, 0.5}, {0.5, 1.}},
                                                {{0.5, 0.5}, {0.5, 1.0}, {1.0, 0.5}, {1.0, 1.0}}};
  EXPECT_EQ(expected_elements, actual_elements);
}

TEST_F(GeneratedMeshMFEMTest, Check3D)
{
  InputParameters mesh_params = getValidParams();
  mesh_params.set<MooseEnum>("dim") = 3;
  mesh_params.set<unsigned int>("nx") = 2;
  mesh_params.set<unsigned int>("ny") = 1;
  mesh_params.set<unsigned int>("nz") = 2;
  buildMesh(mesh_params);
  EXPECT_EQ(_mfem_mesh_ptr->Dimension(), 3);
  EXPECT_EQ(_mfem_mesh_ptr->SpaceDimension(), 3);
  EXPECT_EQ(_mfem_mesh_ptr->GetNV(), 18);
  EXPECT_EQ(_mfem_mesh_ptr->GetNE(), 4);
  EXPECT_EQ(_mfem_mesh_ptr->GetNEdges(), 33);
  EXPECT_EQ(_mfem_mesh_ptr->GetNFaces(), 20);

  std::set<std::set<Coord3D>> actual_elements =
      getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::HEXAHEDRON);
  std::set<std::set<Coord3D>> expected_elements{{{0., 0., 0.},
                                                 {0., 0., .5},
                                                 {0., 1., 0.},
                                                 {0., 1., .5},
                                                 {.5, 0., 0.},
                                                 {.5, 0., .5},
                                                 {.5, 1., 0.},
                                                 {.5, 1., .5}},
                                                {{0.5, 0., 0.},
                                                 {0.5, 0., 0.5},
                                                 {0.5, 1., 0.},
                                                 {0.5, 1., 0.5},
                                                 {1., 0., 0.},
                                                 {1., 0., 0.5},
                                                 {1., 1., 0.},
                                                 {1., 1., 0.5}},
                                                {{0., 0., 0.5},
                                                 {0., 0., 1.},
                                                 {0., 1., 0.5},
                                                 {0., 1., 1.},
                                                 {0.5, 0., 0.5},
                                                 {0.5, 0., 1.},
                                                 {0.5, 1., 0.5},
                                                 {0.5, 1., 1.}},
                                                {{0.5, 0., 0.5},
                                                 {0.5, 0., 1.0},
                                                 {0.5, 1., 0.5},
                                                 {0.5, 1., 1.0},
                                                 {1.0, 0., 0.5},
                                                 {1.0, 0., 1.0},
                                                 {1.0, 1., 0.5},
                                                 {1.0, 1., 1.0}}};
  EXPECT_EQ(expected_elements, actual_elements);
}

class FileMeshMFEMTest : public LibMeshToMFEMMeshTest<FileMesh>,
                         public testing::WithParamInterface<std::string>
{
protected:
  static const std::map<std::string, mfem::Element::Type> types;
  std::string _filename;
  mfem::Element::Type _elem_type;
  int _nodes_per_element;

  void SetUp() override
  {
    LibMeshToMFEMMeshTest<FileMesh>::SetUp();
    _filename = GetParam();
    size_t split_at = _filename.find("-");
    _elem_type = types.at(_filename.substr(0, split_at));
    _nodes_per_element = std::stoi(_filename.substr(split_at + 1, _filename.find(".") - split_at));
  }

  void loadMesh()
  {
    InputParameters mesh_params = getValidParams();
    mesh_params.set<MeshFileName>("file") = "files/LibMeshToMFEMMeshTest/gold/" + _filename;
    buildMesh(mesh_params);
  };
};

const std::map<std::string, mfem::Element::Type> FileMeshMFEMTest::types{
    {"point", mfem::Element::POINT},
    {"seg", mfem::Element::SEGMENT},
    {"tri", mfem::Element::TRIANGLE},
    {"quad", mfem::Element::QUADRILATERAL},
    {"tet", mfem::Element::TETRAHEDRON},
    {"hex", mfem::Element::HEXAHEDRON},
    {"wedge", mfem::Element::WEDGE},
    {"pyramid", mfem::Element::PYRAMID}};

TEST_P(FileMeshMFEMTest, CheckLoad)
{
  loadMesh();
  int n_elem = _mfem_mesh_ptr->GetNE();
  EXPECT_EQ(n_elem, _moose_mesh_ptr->nElem());
  const mfem::FiniteElementSpace * nodal_fespace = _mfem_mesh_ptr->GetNodalFESpace();
  for (int i = 0; i < n_elem; ++i)
  {
    mfem::Element * elem = _mfem_mesh_ptr->GetElement(i);
    EXPECT_EQ(elem->GetType(), _elem_type);
    if (nodal_fespace == nullptr)
    {
      EXPECT_EQ(elem->GetNVertices(), _nodes_per_element);
    }
    else
    {
      EXPECT_EQ(nodal_fespace->GetFE(i)->GetNodes().GetNPoints(), _nodes_per_element);
    }
  }
}

INSTANTIATE_TEST_SUITE_P(
    SimpleCubes,
    FileMeshMFEMTest,
    testing::Values(
        "tet-4.e", "tet-10.e", "hex-8.e", "hex-27.e", "wedge-6.e", "wedge-18.e", "pyramid-5.e"));

Point
toPoint(const CoordND<1> & x, libMesh::ElemType)
{
  return Point(2 * x[0] - 1);
}
Point
toPoint(const CoordND<2> & x, libMesh::ElemType etype)
{
  switch (etype)
  {
    case libMesh::ElemType::TRI3:
    case libMesh::ElemType::TRI6:
    case libMesh::ElemType::TRI7:
      return Point(x[0], x[1]);
    default:
      return Point(2 * x[0] - 1, 2 * x[1] - 1);
  }
}
Point
toPoint(const CoordND<3> & x, libMesh::ElemType etype)
{
  switch (etype)
  {
    case libMesh::ElemType::TET4:
    case libMesh::ElemType::TET10:
    case libMesh::ElemType::TET14:
      return Point(x[0], x[1], x[2]);
    case libMesh::ElemType::PRISM6:
    case libMesh::ElemType::PRISM15:
    case libMesh::ElemType::PRISM18:
    case libMesh::ElemType::PRISM20:
    case libMesh::ElemType::PRISM21:
      return Point(x[0], x[1], 2 * x[2] - 1);
    case libMesh::ElemType::PYRAMID5:
    case libMesh::ElemType::PYRAMID13:
    case libMesh::ElemType::PYRAMID14:
    case libMesh::ElemType::PYRAMID18:
    {
      const double zp = 1 - x[2];
      return Point(2 * x[0] - zp, 2 * x[1] - zp, x[2]);
    }
    default:
      return Point(2 * x[0] - 1, 2 * x[1] - 1, 2 * x[2] - 1);
  }
}

/**
 * Check the libMesh and MFEM transforms return the same
 * results. Reference coordinates should be in the range [0, 1], as is
 * the case for MFEM. The `x_is_unit`, `y_is_unit`, and `z_is_unit`
 * determine whether this is the same domain used by the libMesh
 * element for that reference coordinate. If `true` then that
 * coordinate is in rnage [0, 1], otherwise it is in range [-1, 1].
 */
template <int M, int N>
void
checkTransform(const Elem * libmesh_elem,
               mfem::ElementTransformation & mfem_transform,
               std::array<CoordND<M>, N> ref_coords)
{
  // Assemble matrices to use for the MFEM transform
  mfem::DenseMatrix ref_coords_mat(M, N), actual_phys_coords_mat(M, N);
  for (int i = 0; i < M; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      ref_coords_mat(i, j) = ref_coords[j][i];
    }
  }
  mfem_transform.Transform(ref_coords_mat, actual_phys_coords_mat);
  for (int j = 0; j < N; ++j)
  {
    std::string ref_coord;
    for (int i = 0; i < M; ++i)
    {
      if (i > 0)
      {
        ref_coord += ", ";
      }
      ref_coord += std::to_string(ref_coords[j][i]);
    }
    Point expected =
        libMesh::FEMap::map(M, libmesh_elem, toPoint(ref_coords[j], libmesh_elem->type()));
    for (int i = 0; i < M; ++i)
    {
      EXPECT_NEAR(actual_phys_coords_mat(i, j), expected(i), 1e-12)
          << "Unexpected element " << i << " of physical coordinate at (" << ref_coord
          << ") in reference space";
    }
  }
}

template <int N>
struct ElementTestData
{
  InputParameters mesh_params;
  InputParameters mg_params;
  std::set<std::set<CoordND<N>>> expected_element;
  std::array<CoordND<N>, 3> test_coords;
  bool higher_order;
  mfem::Element::Type type;
};

ElementTestData<1>
edge2Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "EDGE2";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{1., 0., 0.}, {2., 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{1.}, {2.}}},
          {{{0.}, {0.5}, {1.}}},
          false,
          mfem::Element::Type::SEGMENT};
}

ElementTestData<1>
edge3Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "EDGE3";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{0., 0., 0.}, {4., 0., 0.}, {1., 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0.}, {4.}}},
          {{{0.}, {0.5}, {1.}}},
          true,
          mfem::Element::Type::SEGMENT};
}

ElementTestData<1>
edge4Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "EDGE4";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., 0., 0.}, {26., 0., 0.}, {0., 0., 0.}, {7., 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1.}, {26.}}},
          {{{0.25}, {0.75}, {1.}}},
          true,
          mfem::Element::Type::SEGMENT};
}

ElementTestData<2>
tri3Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TRI3";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{0., 0., 0.}, {2., 0., 0.}, {0., 2., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0.}, {2., 0.}, {0., 2.}}},
          {{{0., 0.}, {0.5, 0.25}, {1., 0.}}},
          false,
          mfem::Element::TRIANGLE};
}

ElementTestData<2>
tri6Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TRI6";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3, 4, 5};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {0., 0., 0.}, {1., 0., 0.}, {0., 1., 0.}, {.5, 0.1, 0.0}, {0.5, 0.5, 0.0}, {0., .5, 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0.}, {1., 0.}, {0., 1.}}},
          {{{0., 0.5}, {0.25, 0.0}, {0.4, 0.2}}},
          true,
          mfem::Element::TRIANGLE};
}

ElementTestData<2>
tri7Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TRI7";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3, 4, 5, 6};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{0., 0., 0.},
                                                          {1., 0., 0.},
                                                          {0., 1., 0.},
                                                          {.5, 0.1, 0.0},
                                                          {0.5, 0.5, 0.0},
                                                          {0., .5, 0.},
                                                          {0.3, 0.25, 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0.}, {1., 0.}, {0., 1.}}},
          {{{0., 0.5}, {0.25, 0.0}, {0.4, 0.2}}},
          true,
          mfem::Element::TRIANGLE};
}

ElementTestData<2>
quad4Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "QUAD4";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {0., 0., 0.}, {1., 0., 0.}, {2., 2., 0.}, {0.5, 1.0, 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0.}, {1., 0.}, {0.5, 1.}, {2., 2.}}},
          {{{0., 0.5}, {0.75, 1.}, {0.5, 0.5}}},
          false,
          mfem::Element::QUADRILATERAL};
}

ElementTestData<2>
quad8Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "QUAD8";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3, 4, 5, 6, 7};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{0., 0., 0.},
                                                          {1., 0., 0.},
                                                          {1., 1., 0.},
                                                          {0., 1.0, 0.},
                                                          {0.5, 0.1, 0.},
                                                          {1., 0.5, 0.},
                                                          {0.5, 1., 0.},
                                                          {0., 0.5, 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0.}, {1., 0.}, {1., 1.}, {0., 1.}}},
          {{{0., 0.5}, {0.75, 0.5}, {0.5, 0.25}}},
          true,
          mfem::Element::QUADRILATERAL};
}

ElementTestData<2>
quad9Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "QUAD9";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3, 4, 5, 6, 7, 8};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{0., 0., 0.},
                                                          {1., 0., 0.},
                                                          {1., 1., 0.},
                                                          {0., 1.0, 0.},
                                                          {0.5, 0.1, 0.},
                                                          {1., 0.5, 0.},
                                                          {0.5, 1., 0.},
                                                          {0., 0.5, 0.},
                                                          {0.5, 0.6, 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0.}, {1., 0.}, {1., 1.}, {0., 1.}}},
          {{{0., 0.5}, {0.75, 0.5}, {0.5, 0.25}}},
          true,
          mfem::Element::QUADRILATERAL};
}

ElementTestData<3>
tet4Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TET4";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., -1., 0.}, {1., -1., 0.}, {0., 1., 0.}, {0., 0.0, -1.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., 0.}, {1., -1., 0.}, {0., 1., 0.}, {0., 0., -1.}}},
          {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.125, 0.25, 0.5}}},
          false,
          mfem::Element::TETRAHEDRON};
}

ElementTestData<3>
tet10Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TET10";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{-1., -1., -1.},
                                                          {1., -1., -1.},
                                                          {-1., 1., -1.},
                                                          {-1., -1.0, 1.},
                                                          {0., -.8, -.8},
                                                          {0., 0., -1.},
                                                          {-1., 0., -1},
                                                          {-1., -1., 0.},
                                                          {0., -1., 0.},
                                                          {-1., 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., -1.}, {1., -1., -1.}, {-1., 1., -1.}, {-1., -1., 1.}}},
          {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.125, 0.25, 0.5}}},
          true,
          mfem::Element::TETRAHEDRON};
}

ElementTestData<3>
tet14Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TET14";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{-1., -1., -1.},
                                                          {1., -1., -1.},
                                                          {-1., 1., -1.},
                                                          {-1., -1.0, 1.},
                                                          {0., -.8, -.8},
                                                          {0., 0., -1.},
                                                          {-1., 0., -1},
                                                          {-1., -1., 0.},
                                                          {0., -1., 0.},
                                                          {-1., 0., 0.},
                                                          {0., -0.5, -0.9},
                                                          {0., -1.1, -0.5},
                                                          {0.01, -0.01, 0.01},
                                                          {-0.95, 0., -0.5}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., -1.}, {1., -1., -1.}, {-1., 1., -1.}, {-1., -1., 1.}}},
          {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.125, 0.25, 0.5}}},
          true,
          mfem::Element::TETRAHEDRON};
}

ElementTestData<3>
pyr5Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PYRAMID5";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3, 4};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., -1., 0.}, {1., -1., 0.}, {1., 1., 0.}, {-1., 1.0, 0.}, {1.5, 0., 2.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., 0.}, {1., -1., 0.}, {1., 1., 0.}, {-1., 1., 0.}, {1.5, 0., 2}}},
          {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.25, 0.25, 0.75}}},
          false,
          mfem::Element::PYRAMID};
}

ElementTestData<3>
pyr13Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PYRAMID13";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{-1., -1., -1.},
                                                          {1., -1., -1.},
                                                          {1., 1., -1.},
                                                          {-1., 1.0, -1.},
                                                          {0., 0., 1.},
                                                          {0., -1., -1.},
                                                          {1., 0., -1.},
                                                          {0., 1.1, -1.},
                                                          {-1., 0., -1.},
                                                          {-.5, -.5, 0.},
                                                          {0.5, -.5, 0.},
                                                          {0.5, 0.5, 0.},
                                                          {-.5, .5, 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., -1.}, {1., -1., -1.}, {1., 1., -1.}, {-1., 1., -1.}, {0., 0., 1.}}},
          {{{0.25, 0.5, 0.}, {0.5, 0., 0.3}, {0.25, 0.25, 0.5}}},
          true,
          mfem::Element::PYRAMID};
}

ElementTestData<3>
pyr14Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PYRAMID14";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{-1., -1., -1.},
                                                          {1., -1., -1.},
                                                          {1., 1., -1.},
                                                          {-1., 1.0, -1.},
                                                          {0., 0., 1.},
                                                          {0., -1., -1.},
                                                          {1., 0., -1.},
                                                          {0., 1.1, -1.},
                                                          {-1., 0., -1.},
                                                          {-.5, -.5, 0.},
                                                          {0.5, -.5, 0.1},
                                                          {0.5, 0.5, 0.},
                                                          {-.5, .5, 0.},
                                                          {0., 0.05, -1.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., -1.}, {1., -1., -1.}, {1., 1., -1.}, {-1., 1., -1.}, {0., 0., 1.}}},
          {{{0., 0.5, 0.5}, {0.5, 0., 0.3}, {0.25, 0.25, 0.6}}},
          true,
          mfem::Element::PYRAMID};
}
// // A bug in MFEM currently means we can't support higher-order
// // libmesh pyramids. Once that is fixed buildMesh() should no longe
// // throw an error and the commented out tests below should pass.
// EXPECT_THROW(buildMesh(mesh_params, "ElementGenerator", mg_params), MooseRuntimeError);

ElementTestData<3>
prism6Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PRISM6";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3, 4, 5};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., -1., -1.}, {1., -1., -1.}, {0.5, 1., -1.}, {0, -1.0, 1.}, {2, -1., 1.}, {2., 1., 1.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., -1.},
            {1., -1., -1.},
            {0.5, 1., -1.},
            {0, -1.0, 1.},
            {2, -1., 1.},
            {2., 1., 1.}}},
          {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.5, 0.5, 0.75}}},
          false,
          mfem::Element::WEDGE};
}

ElementTestData<3>
prism15Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PRISM15";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{-1., -1., -1.},
                                                          {1., -1., -1.},
                                                          {0., 1., -1.},
                                                          {-1, -1.0, 1.},
                                                          {1, -1., 1.},
                                                          {0., 1., 1.},
                                                          {0., -1., -1.},
                                                          {0.5, 0.0, -1.},
                                                          {-0.5, 0., -1.},
                                                          {-1., -1., 0.},
                                                          {1., -1., 0.},
                                                          {0.2, 1., -0.1},
                                                          {0., -1., 1.},
                                                          {0.5, 0., 1.},
                                                          {-0.4, 0.1, 1.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., -1.},
            {1., -1., -1.},
            {0.0, 1., -1.},
            {-1., -1.0, 1.},
            {1., -1., 1.},
            {0., 1., 1.}}},
          {{{0.25, 0.5, 0.}, {0.5, 0., 0.8}, {0.5, 0.5, 0.75}}},
          true,
          mfem::Element::WEDGE};
}

ElementTestData<3>
prism18Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PRISM18";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{-1., -1., -1.},
                                                          {1., -1., -1.},
                                                          {0., 1., -1.},
                                                          {-1, -1.0, 1.},
                                                          {1, -1., 1.},
                                                          {0., 1., 1.},
                                                          {0., -1., -1.},
                                                          {0.5, 0.0, -1.},
                                                          {-0.5, 0., -1.},
                                                          {-1., -1., 0.},
                                                          {1., -1., 0.},
                                                          {0.2, 1., -0.1},
                                                          {0., -1., 1.},
                                                          {0.5, 0., 1.},
                                                          {-0.4, 0.1, 1.},
                                                          {0., -1., 0.},
                                                          {0.5, 0., 0.},
                                                          {-0.5, 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., -1.},
            {1., -1., -1.},
            {0.0, 1., -1.},
            {-1., -1.0, 1.},
            {1., -1., 1.},
            {0., 1., 1.}}},
          {{{0.25, 0.5, 0.}, {0.5, 0., 0.8}, {0.5, 0.5, 0.75}}},
          true,
          mfem::Element::WEDGE};
}

ElementTestData<3>
hex8Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "HEX8";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3, 4, 5, 6, 7};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{0., 0., 0.},
                                                          {1., 0., 0.},
                                                          {2., 1., 1.},
                                                          {1.0, 1.0, 1.},
                                                          {0., 0., 1.},
                                                          {1., 0., 1.},
                                                          {2., 1., 2.},
                                                          {1., 1., 2.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0., 0.},
            {1., 0., 0.},
            {2., 1., 1.},
            {1., 1., 1.},
            {0., 0., 1.},
            {1., 0., 1.},
            {2., 1., 2.},
            {1., 1., 2.}}},
          {{{0., 0.5, 0.5}, {0.5, 0.5, 1.}, {0.5, 0.5, 0.5}}},
          false,
          mfem::Element::HEXAHEDRON};
}

ElementTestData<3>
hex20Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "HEX20";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {0., 0., 0.},  {1., 0., 0.},  {1., 1., 0.},   {0., 1., 0.},   {0., 0., 1.},
      {1., 0., 1.},  {1, 1., 1.},   {0., 1., 1.},   {0.5, 0., 0.},  {1., 0.5, 0.},
      {0.5, 1., 0.}, {0., 0.5, 0.}, {0., 0.1, 0.5}, {1., 0.1, 0.5}, {1., 1., 0.5},
      {0., 1., 0.5}, {0.5, 0., 1.}, {1., 0.5, 1.},  {0.5, 1., 1.},  {0., 0.5, 1.},
  };
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0., 0.},
            {1., 0., 0.},
            {1., 1., 0.},
            {0., 1., 0.},
            {0., 0., 1.},
            {1., 0., 1.},
            {1., 1., 1.},
            {0., 1., 1.}}},
          {{{1., 0.4, 0.5}, {0.75, 0., 0.5}, {0.1, 0.25, 0.25}}},
          true,
          mfem::Element::HEXAHEDRON};
}

ElementTestData<3>
hex27Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "HEX27";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
      14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {0., 0., 0.},   {1., 0., 0.},    {1., 1., 0.},   {0., 1., 0.},   {0., 0., 1.},
      {1., 0., 1.},   {1, 1., 1.},     {0., 1., 1.},   {0.5, 0., 0.},  {1., 0.5, 0.},
      {0.5, 1., 0.},  {0., 0.5, 0.},   {0., 0.1, 0.5}, {1., 0.1, 0.5}, {1., 1., 0.5},
      {0., 1., 0.5},  {0.5, 0., 1.},   {1., 0.5, 1.},  {0.5, 1., 1.},  {0., 0.5, 1.},
      {0.5, 0.5, 0.}, {0.5, 0.1, 0.5}, {1., 0.6, 0.5}, {0.5, 1., 0.5}, {0., 0.6, 0.5},
      {0.5, 0.5, 1.}, {0.5, 0.6, 0.5},

  };
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0., 0.},
            {1., 0., 0.},
            {1., 1., 0.},
            {0., 1., 0.},
            {0., 0., 1.},
            {1., 0., 1.},
            {1., 1., 1.},
            {0., 1., 1.}}},
          {{{1., 0.4, 0.5}, {0.75, 0., 0.5}, {0.1, 0.25, 0.25}}},
          true,
          mfem::Element::HEXAHEDRON};
}

template <int N>
class ElementGeneratorMFEMTest : public LibMeshToMFEMMeshTest<MeshGeneratorMesh, true, false>,
                                 public testing::WithParamInterface<ElementTestData<N>>
{
public:
  static std::string testParamName(const testing::TestParamInfo<ElementTestData<N>> & info)
  {
    auto & params = info.param.mg_params;
    return params.template get<MooseEnum>("elem_type");
  }
};

using Element1DGeneratorMFEMTest = ElementGeneratorMFEMTest<1>;

TEST_P(Element1DGeneratorMFEMTest, CheckElem)
{
  auto data = GetParam();
  buildMesh(data.mesh_params, "ElementGenerator", data.mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord1D>> actual_elements = getElementSet<1>(_mfem_mesh_ptr, data.type);
  EXPECT_EQ(data.expected_element, actual_elements);
  if (data.higher_order)
  {
    EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  }
  else
  {
    EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  }
  mfem::DenseMatrix jac(1, 1);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<1, 3>(
      _moose_mesh_ptr->elemPtr(0), *_mfem_mesh_ptr->GetElementTransformation(0), data.test_coords);
}

INSTANTIATE_TEST_SUITE_P(SingleElement,
                         Element1DGeneratorMFEMTest,
                         testing::Values(edge2Data(), edge3Data(), edge4Data()),
                         Element1DGeneratorMFEMTest::testParamName);

using Element2DGeneratorMFEMTest = ElementGeneratorMFEMTest<2>;

TEST_P(Element2DGeneratorMFEMTest, CheckElem)
{
  auto data = GetParam();
  buildMesh(data.mesh_params, "ElementGenerator", data.mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord2D>> actual_elements = getElementSet<2>(_mfem_mesh_ptr, data.type);
  EXPECT_EQ(data.expected_element, actual_elements);
  if (data.higher_order)
  {
    EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  }
  else
  {
    EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  }
  mfem::DenseMatrix jac(2, 2);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<2, 3>(
      _moose_mesh_ptr->elemPtr(0), *_mfem_mesh_ptr->GetElementTransformation(0), data.test_coords);
}

INSTANTIATE_TEST_SUITE_P(
    SingleElement,
    Element2DGeneratorMFEMTest,
    testing::Values(tri3Data(), tri6Data(), quad4Data(), quad8Data(), quad9Data()),
    Element2DGeneratorMFEMTest::testParamName);

using Element3DGeneratorMFEMTest = ElementGeneratorMFEMTest<3>;

TEST_P(Element3DGeneratorMFEMTest, CheckElem)
{
  auto data = GetParam();
  buildMesh(data.mesh_params, "ElementGenerator", data.mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements = getElementSet<3>(_mfem_mesh_ptr, data.type);
  EXPECT_EQ(data.expected_element, actual_elements);
  if (data.higher_order)
  {
    EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  }
  else
  {
    EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  }
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<3, 3>(
      _moose_mesh_ptr->elemPtr(0), *_mfem_mesh_ptr->GetElementTransformation(0), data.test_coords);
}

INSTANTIATE_TEST_SUITE_P(SingleElement,
                         Element3DGeneratorMFEMTest,
                         testing::Values(tet4Data(),
                                         tet10Data(),
                                         pyr5Data(),
                                         // A bug in MFEM currently means we can't support
                                         // higher-order libmesh pyramids. Once that is fixed
                                         // these cases should be uncommented.
                                         // pyr13Data(),
                                         // pyr14Data(),
                                         prism6Data(),
                                         prism15Data(),
                                         prism18Data(),
                                         hex8Data(),
                                         hex20Data(),
                                         hex27Data()),
                         Element3DGeneratorMFEMTest::testParamName);

using UnsupportedElementGeneratorMFEMTest = ElementGeneratorMFEMTest<3>;

TEST_P(UnsupportedElementGeneratorMFEMTest, CheckElem)
{
  auto data = GetParam();
  EXPECT_THROW(buildMesh(data.mesh_params, "ElementGenerator", data.mg_params), MooseRuntimeError);
}

// A bug in MFEM currently means we can't support higher-order
// libmesh pyramids. Once that is fixed this test should be removed.
INSTANTIATE_TEST_SUITE_P(SingleElement,
                         UnsupportedElementGeneratorMFEMTest,
                         testing::Values(pyr13Data(), pyr14Data()),
                         UnsupportedElementGeneratorMFEMTest::testParamName);

using LibMeshToMFEMMeshFallbackTest = LibMeshToMFEMMeshTest<MeshGeneratorMesh, true, false>;

TEST_F(LibMeshToMFEMMeshFallbackTest, CheckTri7)
{
  auto data = tri7Data();
  // A warning should be emitted when converting from TRI7 to TRI6
  EXPECT_THROW(buildMesh(data.mesh_params, "ElementGenerator", data.mg_params), std::runtime_error);
  // Temporarily allow warnings to be emitted, so we can test the
  // conversion actually works properly.
  Moose::_throw_on_warning = false;
  _mfem_mesh_ptr = buildMFEMMesh(*_moose_mesh_ptr, _fallback, _first_order);
  Moose::_throw_on_warning = true;
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord2D>> actual_elements = getElementSet<2>(_mfem_mesh_ptr, data.type);
  EXPECT_EQ(data.expected_element, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(2, 2);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);

  auto data2 = tri6Data();
  std::shared_ptr<MeshGeneratorMesh> expected_mesh =
      buildAdditionalMesh(data2.mesh_params, "ElementGenerator", data2.mg_params);
  checkTransform<2, 3>(
      expected_mesh->elemPtr(0), *_mfem_mesh_ptr->GetElementTransformation(0), data.test_coords);
}

TEST_F(LibMeshToMFEMMeshFallbackTest, CheckTet14)
{
  auto data = tet14Data();
  // A warning should be emitted when converting from TRI7 to TRI6
  EXPECT_THROW(buildMesh(data.mesh_params, "ElementGenerator", data.mg_params), std::runtime_error);
  // Temporarily allow warnings to be emitted, so we can test the
  // conversion actually works properly.
  Moose::_throw_on_warning = false;
  _mfem_mesh_ptr = buildMFEMMesh(*_moose_mesh_ptr, _fallback, _first_order);
  Moose::_throw_on_warning = true;
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements = getElementSet<3>(_mfem_mesh_ptr, data.type);
  EXPECT_EQ(data.expected_element, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);

  auto data2 = tet10Data();
  std::shared_ptr<MeshGeneratorMesh> expected_mesh =
      buildAdditionalMesh(data2.mesh_params, "ElementGenerator", data2.mg_params);
  checkTransform<3, 3>(
      expected_mesh->elemPtr(0), *_mfem_mesh_ptr->GetElementTransformation(0), data.test_coords);
}

using LibMeshToMFEMMeshNoFallbackTest = LibMeshToMFEMMeshTest<MeshGeneratorMesh, false, false>;

TEST_F(libMeshToMFEMMeshNoFallbackTest, CheckTRI7)
{
  auto data = tri7Data();
  EXPECT_THROW(buildMesh(data.mesh_params, "ElementGenerator", data.mg_params), MooseRuntimeError);
}

TEST_F(libMeshToMFEMMeshNoFallbackTest, CheckTET14)
{
  auto data = tet14Data();
  EXPECT_THROW(buildMesh(data.mesh_params, "ElementGenerator", data.mg_params), MooseRuntimeError);
}

using ElementToFirstOrderMFEMTest = LibMeshToMFEMMeshTest<MeshGeneratorMesh, true, true>;

#endif
