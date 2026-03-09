//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED
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

template <class M>
class LibMeshToMFEMMeshTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    const char * argv[2] = {"foo", "\0"};
    _app = Moose::createMooseApp("MooseUnitApp", 1, (char **)argv);
    _factory = &_app->getFactory();
    _mesh_type = Registry::getClassName<M>();
  }

  std::shared_ptr<MooseApp> _app;
  Factory * _factory;
  std::string _mesh_type;
  std::shared_ptr<M> _moose_mesh_ptr;
  std::shared_ptr<mfem::ParMesh> _mfem_mesh_ptr;

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
    _moose_mesh_ptr = _factory->create<M>(_mesh_type, "moose_mesh", mesh_params);
    _app->actionWarehouse().mesh() = _moose_mesh_ptr;
    if (generator_class != "")
    {
      // Rank is not being assigned to elements
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
    _mfem_mesh_ptr = buildMFEMMesh(*_moose_mesh_ptr);
  }
};

template <int N>
using CoordND = std::array<mfem::real_t, N>;
using Coord1D = CoordND<1>;
using Coord2D = CoordND<2>;
using Coord3D = CoordND<3>;

template <int N>
std::set<std::set<std::array<mfem::real_t, N>>>
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

using ElementGeneratorMFEMTest = LibMeshToMFEMMeshTest<MeshGeneratorMesh>;

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

TEST_F(ElementGeneratorMFEMTest, CheckEDGE2)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
  mg_params.set<MooseEnum>("elem_type") = "EDGE2";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{1., 0., 0.}, {2., 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord1D>> actual_elements =
      getElementSet<1>(_mfem_mesh_ptr, mfem::Element::Type::SEGMENT);
  std::set<std::set<Coord1D>> expected_elements = {{{1.}, {2.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(1, 1);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<1, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0.}, {0.5}, {1.}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckEDGE3)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
  mg_params.set<MooseEnum>("elem_type") = "EDGE3";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{0., 0., 0.}, {4., 0., 0.}, {1., 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord1D>> actual_elements =
      getElementSet<1>(_mfem_mesh_ptr, mfem::Element::Type::SEGMENT);
  std::set<std::set<Coord1D>> expected_elements = {{{0.}, {4.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(1, 1);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<1, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0.25}, {0.75}, {1.}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckEDGE4)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
  mg_params.set<MooseEnum>("elem_type") = "EDGE4";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., 0., 0.}, {26., 0., 0.}, {0., 0., 0.}, {7., 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord1D>> actual_elements =
      getElementSet<1>(_mfem_mesh_ptr, mfem::Element::Type::SEGMENT);
  std::set<std::set<Coord1D>> expected_elements = {{{-1.}, {26.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(1, 1);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<1, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0.25}, {0.75}, {1.}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckTRI3)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
  mg_params.set<MooseEnum>("elem_type") = "TRI3";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{0., 0., 0.}, {2., 0., 0.}, {0., 2., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord2D>> actual_elements =
      getElementSet<2>(_mfem_mesh_ptr, mfem::Element::Type::TRIANGLE);
  std::set<std::set<Coord2D>> expected_elements = {{{0., 0.}, {2., 0.}, {0., 2.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(2, 2);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<2, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.}, {0.5, 0.25}, {1., 0.}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckTRI6)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
  mg_params.set<MooseEnum>("elem_type") = "TRI6";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3, 4, 5};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {0., 0., 0.}, {1., 0., 0.}, {0., 1., 0.}, {.5, 0.1, 0.0}, {0.5, 0.5, 0.0}, {0., .5, 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord2D>> actual_elements =
      getElementSet<2>(_mfem_mesh_ptr, mfem::Element::Type::TRIANGLE);
  std::set<std::set<Coord2D>> expected_elements = {{{0., 0.}, {1., 0.}, {0., 1.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(2, 2);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<2, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5}, {0.25, 0.0}, {0.4, 0.2}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckQUAD4)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
  mg_params.set<MooseEnum>("elem_type") = "QUAD4";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {0., 0., 0.}, {1., 0., 0.}, {2., 2., 0.}, {0.5, 1.0, 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord2D>> actual_elements =
      getElementSet<2>(_mfem_mesh_ptr, mfem::Element::Type::QUADRILATERAL);
  std::set<std::set<Coord2D>> expected_elements = {{{0., 0.}, {1., 0.}, {0.5, 1.}, {2., 2.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(2, 2);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<2, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5}, {0.75, 1.}, {0.5, 0.5}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckQUAD8)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
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
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord2D>> actual_elements =
      getElementSet<2>(_mfem_mesh_ptr, mfem::Element::Type::QUADRILATERAL);
  std::set<std::set<Coord2D>> expected_elements = {{{0., 0.}, {1., 0.}, {1., 1.}, {0., 1.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(2, 2);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<2, 4>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5}, {0.4, 0.}, {0.75, 0.5}, {0.5, 0.25}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckQUAD9)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
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
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord2D>> actual_elements =
      getElementSet<2>(_mfem_mesh_ptr, mfem::Element::Type::QUADRILATERAL);
  std::set<std::set<Coord2D>> expected_elements = {{{0., 0.}, {1., 0.}, {1., 1.}, {0., 1.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(2, 2);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<2, 4>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5}, {0.4, 0.}, {0.75, 0.5}, {0.5, 0.25}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckTET4)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
  mg_params.set<MooseEnum>("elem_type") = "TET4";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., -1., 0.}, {1., -1., 0.}, {0., 1., 0.}, {0., 0.0, -1.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements =
      getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::TETRAHEDRON);
  std::set<std::set<Coord3D>> expected_elements = {
      {{-1., -1., 0.}, {1., -1., 0.}, {0., 1., 0.}, {0., 0., -1.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<3, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.125, 0.25, 0.5}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckTET10)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
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
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements =
      getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::TETRAHEDRON);
  std::set<std::set<Coord3D>> expected_elements = {
      {{-1., -1., -1.}, {1., -1., -1.}, {-1., 1., -1.}, {-1., -1., 1.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<3, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.125, 0.25, 0.5}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckPYRAMID5)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
  mg_params.set<MooseEnum>("elem_type") = "PYRAMID5";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3, 4};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., -1., 0.}, {1., -1., 0.}, {1., 1., 0.}, {-1., 1.0, 0.}, {1.5, 0., 2.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements =
      getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::PYRAMID);
  std::set<std::set<Coord3D>> expected_elements = {
      {{-1., -1., 0.}, {1., -1., 0.}, {1., 1., 0.}, {-1., 1., 0.}, {1.5, 0., 2}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<3, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.25, 0.25, 0.75}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckPYRAMID13)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
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
  // A bug in MFEM currently means we can't support higher-order
  // libmesh pyramids. Once that is fixed buildMesh() should no longe
  // throw an error and the commented out tests below should pass.
  EXPECT_THROW(buildMesh(mesh_params, "ElementGenerator", mg_params), MooseRuntimeError);
  // ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  // std::set<std::set<Coord3D>> actual_elements =
  //     getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::PYRAMID);
  // std::set<std::set<Coord3D>> expected_elements = {
  //     {{-1., -1., -1.}, {1., -1., -1.}, {1., 1., -1.}, {-1., 1., -1.}, {0., 0., 1.}}};
  // EXPECT_EQ(expected_elements, actual_elements);
  // EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  // mfem::DenseMatrix jac(3, 3);
  // _mfem_mesh_ptr->GetElementJacobian(0, jac);
  // EXPECT_GT(jac.Det(), 0.);
  // checkTransform<3, 4>(_moose_mesh_ptr->elemPtr(0), *_mfem_mesh_ptr->GetElementTransformation(0),
  //                      {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.5, 0., 0.3}, {0.25, 0.25, 0.5}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckPYRAMID14)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
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
  // A bug in MFEM currently means we can't support higher-order
  // libmesh pyramids. Once that is fixed buildMesh() should no longe
  // throw an error and the commented out tests below should pass.
  EXPECT_THROW(buildMesh(mesh_params, "ElementGenerator", mg_params), MooseRuntimeError);
  // ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  // std::set<std::set<Coord3D>> actual_elements =
  //     getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::PYRAMID);
  // std::set<std::set<Coord3D>> expected_elements = {
  //     {{-1., -1., -1.}, {1., -1., -1.}, {1., 1., -1.}, {-1., 1., -1.}, {0., 0., 1.}}};
  // EXPECT_EQ(expected_elements, actual_elements);
  // EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  // mfem::DenseMatrix jac(3, 3);
  // _mfem_mesh_ptr->GetElementJacobian(0, jac);
  // EXPECT_GT(jac.Det(), 0.);
  // checkTransform<3, 4>(_moose_mesh_ptr->elemPtr(0), *_mfem_mesh_ptr->GetElementTransformation(0),
  //                      {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.5, 0., 0.3}, {0.25, 0.25, 0.6}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckPRISM6)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
  mg_params.set<MooseEnum>("elem_type") = "PRISM6";
  mg_params.set<std::vector<unsigned long>>("element_connectivity") = {0, 1, 2, 3, 4, 5};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., -1., -1.}, {1., -1., -1.}, {0.5, 1., -1.}, {0, -1.0, 1.}, {2, -1., 1.}, {2., 1., 1.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements =
      getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::WEDGE);
  std::set<std::set<Coord3D>> expected_elements = {
      {{-1., -1., -1.}, {1., -1., -1.}, {0.5, 1., -1.}, {0, -1.0, 1.}, {2, -1., 1.}, {2., 1., 1.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<3, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.5, 0.5, 0.75}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckPRISM15)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
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
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements =
      getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::WEDGE);
  std::set<std::set<Coord3D>> expected_elements = {{{-1., -1., -1.},
                                                    {1., -1., -1.},
                                                    {0.0, 1., -1.},
                                                    {-1., -1.0, 1.},
                                                    {1., -1., 1.},
                                                    {0., 1., 1.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<3, 4>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.5, 0., 0.8}, {0.5, 0.5, 0.75}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckPRISM18)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
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
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements =
      getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::WEDGE);
  std::set<std::set<Coord3D>> expected_elements = {{{-1., -1., -1.},
                                                    {1., -1., -1.},
                                                    {0.0, 1., -1.},
                                                    {-1., -1.0, 1.},
                                                    {1., -1., 1.},
                                                    {0., 1., 1.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<3, 4>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.5, 0., 0.8}, {0.5, 0.5, 0.75}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckHEX8)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
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
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements =
      getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::HEXAHEDRON);
  std::set<std::set<Coord3D>> expected_elements = {{{0., 0., 0.},
                                                    {1., 0., 0.},
                                                    {2., 1., 1.},
                                                    {1., 1., 1.},
                                                    {0., 0., 1.},
                                                    {1., 0., 1.},
                                                    {2., 1., 2.},
                                                    {1., 1., 2.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<3, 3>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.5, 0.5, 1.}, {0.5, 0.5, 0.5}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckHEX20)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
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
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements =
      getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::HEXAHEDRON);
  std::set<std::set<Coord3D>> expected_elements = {{{0., 0., 0.},
                                                    {1., 0., 0.},
                                                    {1., 1., 0.},
                                                    {0., 1., 0.},
                                                    {0., 0., 1.},
                                                    {1., 0., 1.},
                                                    {1., 1., 1.},
                                                    {0., 1., 1.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<3, 4>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0.0, 0., 0.25}, {1., 0.4, 0.5}, {0.75, 0., 0.5}, {0.1, 0.25, 0.25}}});
}

TEST_F(ElementGeneratorMFEMTest, CheckHEX27)
{
  InputParameters mesh_params = getValidParams();
  InputParameters mg_params = _factory->getValidParams("ElementGenerator");
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
  buildMesh(mesh_params, "ElementGenerator", mg_params);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord3D>> actual_elements =
      getElementSet<3>(_mfem_mesh_ptr, mfem::Element::Type::HEXAHEDRON);
  std::set<std::set<Coord3D>> expected_elements = {{{0., 0., 0.},
                                                    {1., 0., 0.},
                                                    {1., 1., 0.},
                                                    {0., 1., 0.},
                                                    {0., 0., 1.},
                                                    {1., 0., 1.},
                                                    {1., 1., 1.},
                                                    {0., 1., 1.}}};
  EXPECT_EQ(expected_elements, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  mfem::DenseMatrix jac(3, 3);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform<3, 4>(_moose_mesh_ptr->elemPtr(0),
                       *_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0.0, 0., 0.25}, {1., 0.4, 0.5}, {0.75, 0., 0.5}, {0.1, 0.25, 0.25}}});
}

#endif
