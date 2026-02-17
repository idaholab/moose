#include "gtest/gtest.h"
#include "GeneratedMesh.h"
#include "MeshGeneratorMesh.h"
#include "ElementGenerator.h"
#include "FileMesh.h"
#include "MooseMain.h"
#include "MooseTypes.h"
#include "Registry.h"
#include "MFEMMeshFactory.h"
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

template <int M, int N>
void
checkTransform(mfem::ElementTransformation & transform,
               std::array<CoordND<M>, N> ref_coords,
               std::array<CoordND<M>, N> expected_phys_coords)
{
  mfem::DenseMatrix ref_coords_mat(M, N), actual_phys_coords_mat(M, N);
  for (int i = 0; i < M; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      ref_coords_mat(i, j) = ref_coords[j][i];
    }
  }
  transform.Transform(ref_coords_mat, actual_phys_coords_mat);
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
    for (int i = 0; i < M; ++i)
    {
      EXPECT_DOUBLE_EQ(actual_phys_coords_mat(i, j), expected_phys_coords[j][i])
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
  checkTransform<1, 3>(
      *_mfem_mesh_ptr->GetElementTransformation(0), {{{0.}, {0.5}, {1.}}}, {{{1.}, {1.5}, {2.}}});
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
  checkTransform<1, 3>(*_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0.25}, {0.75}, {1.}}},
                       {{{0.25}, {2.25}, {4.}}});
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
  checkTransform<1, 3>(*_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0.25}, {0.75}, {1.}}},
                       {{{-0.578125}, {10.390625}, {26.}}});
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
  checkTransform<2, 3>(*_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.}, {0.5, 0.25}, {1., 0.}}},
                       {{{0., 0.}, {1., 0.5}, {2., 0.}}});
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
  checkTransform<2, 3>(*_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5}, {0.75, 1.}, {0.5, 0.5}}},
                       {{{0.25, 0.5}, {1.625, 1.75}, {.875, 0.75}}});
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
  checkTransform<3, 3>(*_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.125, 0.25, 0.5}}},
                       {{{0., 0.5, -.5}, {0., 0.0, 0.}, {0., 0.0, -.5}}});
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
  checkTransform<3, 3>(*_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.25, 0.25, 0.75}}},
                       {{{0.25, 0.5, 1.}, {-0.5, 0., 0.}, {1.375, 0.25, 1.5}}});
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
  checkTransform<3, 3>(*_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.5, 0.5, 0.75}}},
                       {{{0.375, 0.0, 0.}, {0.25, 0.0, -1.}, {1.6875, 0., 0.5}}});
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
  checkTransform<3, 3>(*_mfem_mesh_ptr->GetElementTransformation(0),
                       {{{0., 0.5, 0.5}, {0.5, 0.5, 1.}, {0.5, 0.5, 0.5}}},
                       {{{0.5, 0.5, 1.0}, {1., 0.5, 1.5}, {1., 0.5, 1.}}});
}
