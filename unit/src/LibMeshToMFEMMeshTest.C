//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include <stdexcept>
#include <string>
#include <tuple>
#include "gtest/gtest.h"
#include "GeneratedMeshGenerator.h"
#include "MeshGeneratorMesh.h"
#include "ElementGenerator.h"
#include "FileMesh.h"
#include "MooseError.h"
#include "MooseMain.h"
#include "MooseTypes.h"
#include "MooseUnitUtils.h"
#include "Registry.h"
#include "MFEMMeshFactory.h"
#include "libmesh/enum_elem_type.h"
#include "libmesh/fe_map.h"
#include "libmesh/utility.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/elem.h"
#include "libmesh/boundary_info.h"
#include "type_traits"

template <class M>
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

  InputParameters getValidParams() { return _factory->getValidParams(_mesh_type); }

  std::shared_ptr<M> buildMooseMeshOnly(InputParameters & mesh_params,
                                        std::string generator_class,
                                        InputParameters generator_params,
                                        const std::string & mesh_name = "moose_mesh",
                                        const std::string & generator_name = "mesh_generator")
  {
    // If the parameters were created without using the factory
    // object, we need to set the app parameter ourselves.
    mesh_params.addPrivateParam(MooseBase::app_param, _app.get());
    generator_params.addPrivateParam(MooseBase::app_param, _app.get());
    auto moose_mesh = _factory->create<M>(_mesh_type, mesh_name, mesh_params);
    _app->actionWarehouse().mesh() = moose_mesh;
    if (generator_class != "")
    {
      std::unique_ptr<MeshBase> libmesh =
          _factory->create<MeshGenerator>(generator_class, generator_name, generator_params)
              ->generate();
      libmesh->prepare_for_use();
      moose_mesh->setMeshBase(std::move(libmesh));
    }
    else
    {
      moose_mesh->setMeshBase(moose_mesh->buildMeshBaseObject());
    }
    moose_mesh->buildMesh();
    return moose_mesh;
  }

  void buildMesh(InputParameters & mesh_params, bool fallback = false, bool first_order = false)
  {
    // The input parameters in the third argument won't be used, but
    // we need a dummy value.
    buildMesh(mesh_params, "", _factory->getValidParams("ElementGenerator"), fallback, first_order);
  }

  void buildMesh(InputParameters & mesh_params,
                 std::string generator_class,
                 InputParameters generator_params,
                 bool fallback = false,
                 bool first_order = false)
  {
    _moose_mesh_ptr = buildMooseMeshOnly(mesh_params, generator_class, generator_params);
    _mfem_mesh_ptr = Moose::MFEM::buildMFEMMesh(*_moose_mesh_ptr, fallback, first_order);
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

using Coord = std::vector<mfem::real_t>;
using Connectivity = std::vector<dof_id_type>;

std::set<std::set<Coord>>
getElementSet(std::shared_ptr<mfem::ParMesh> mesh, mfem::Element::Type elem_type)
{
  std::set<std::set<Coord>> actual_elements;
  const int N = mesh->SpaceDimension();
  for (const auto i : make_range(mesh->GetNE()))
  {
    mfem::Element * elem = mesh->GetElement(i);
    EXPECT_EQ(elem->GetType(), elem_type);
    int * vertices = elem->GetVertices();
    std::set<Coord> vert_coords;
    for (const auto j : make_range(elem->GetNVertices()))
    {
      mfem::real_t * vc = mesh->GetVertex(vertices[j]);
      Coord c(N);
      for (const auto k : make_range(N))
      {
        c[k] = vc[k];
      }
      vert_coords.emplace(std::move(c));
    }
    actual_elements.emplace(std::move(vert_coords));
  }
  return actual_elements;
}

using GeneratedMeshMFEMTest = LibMeshToMFEMMeshTest<MeshGeneratorMesh>;

/**
 * Return parameters for a GeneratedMeshGenerator of dimension \p dim. Its block and boundary IDs
 * are offset from the default of zero, because MFEM requires positive IDs.
 */
InputParameters
generatedMeshParams(const unsigned int dim)
{
  InputParameters params = GeneratedMeshGenerator::validParams();
  params.set<MooseEnum>("dim") = dim;
  params.set<std::vector<SubdomainID>>("subdomain_ids") = {1};
  params.set<boundary_id_type>("boundary_id_offset") = 1;
  return params;
}

TEST_F(GeneratedMeshMFEMTest, Check1D)
{
  InputParameters mesh_params = getValidParams();
  InputParameters generator_params = generatedMeshParams(1);
  generator_params.set<unsigned int>("nx") = 3;
  generator_params.set<Real>("xmax") = 3.;
  buildMesh(mesh_params, "GeneratedMeshGenerator", generator_params);
  EXPECT_EQ(_mfem_mesh_ptr->Dimension(), 1);
  EXPECT_EQ(_mfem_mesh_ptr->SpaceDimension(), 1);
  EXPECT_EQ(_mfem_mesh_ptr->GetNV(), 4);
  EXPECT_EQ(_mfem_mesh_ptr->GetNE(), 3);

  std::set<std::set<Coord>> actual_elements =
      getElementSet(_mfem_mesh_ptr, mfem::Element::Type::SEGMENT);
  std::set<std::set<Coord>> expected_elements{{{0.}, {1.}}, {{1.}, {2.}}, {{2.}, {3.}}};
  EXPECT_EQ(expected_elements, actual_elements);
}

TEST_F(GeneratedMeshMFEMTest, Check2D)
{
  InputParameters mesh_params = getValidParams();
  InputParameters generator_params = generatedMeshParams(2);
  generator_params.set<unsigned int>("nx") = 2;
  generator_params.set<unsigned int>("ny") = 2;
  buildMesh(mesh_params, "GeneratedMeshGenerator", generator_params);
  EXPECT_EQ(_mfem_mesh_ptr->Dimension(), 2);
  EXPECT_EQ(_mfem_mesh_ptr->SpaceDimension(), 2);
  EXPECT_EQ(_mfem_mesh_ptr->GetNV(), 9);
  EXPECT_EQ(_mfem_mesh_ptr->GetNE(), 4);
  EXPECT_EQ(_mfem_mesh_ptr->GetNEdges(), 12);

  std::set<std::set<Coord>> actual_elements =
      getElementSet(_mfem_mesh_ptr, mfem::Element::Type::QUADRILATERAL);
  std::set<std::set<Coord>> expected_elements{{{0., 0.}, {0., .5}, {.5, 0.}, {.5, .5}},
                                              {{0.5, 0.}, {0.5, 0.5}, {1., 0.}, {1., 0.5}},
                                              {{0., 0.5}, {0., 1.}, {0.5, 0.5}, {0.5, 1.}},
                                              {{0.5, 0.5}, {0.5, 1.0}, {1.0, 0.5}, {1.0, 1.0}}};
  EXPECT_EQ(expected_elements, actual_elements);
}

TEST_F(GeneratedMeshMFEMTest, Check3D)
{
  InputParameters mesh_params = getValidParams();
  InputParameters generator_params = generatedMeshParams(3);
  generator_params.set<unsigned int>("nx") = 2;
  generator_params.set<unsigned int>("ny") = 1;
  generator_params.set<unsigned int>("nz") = 2;
  buildMesh(mesh_params, "GeneratedMeshGenerator", generator_params);
  EXPECT_EQ(_mfem_mesh_ptr->Dimension(), 3);
  EXPECT_EQ(_mfem_mesh_ptr->SpaceDimension(), 3);
  EXPECT_EQ(_mfem_mesh_ptr->GetNV(), 18);
  EXPECT_EQ(_mfem_mesh_ptr->GetNE(), 4);
  EXPECT_EQ(_mfem_mesh_ptr->GetNEdges(), 33);
  EXPECT_EQ(_mfem_mesh_ptr->GetNFaces(), 20);

  std::set<std::set<Coord>> actual_elements =
      getElementSet(_mfem_mesh_ptr, mfem::Element::Type::HEXAHEDRON);
  std::set<std::set<Coord>> expected_elements{{{0., 0., 0.},
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

TEST_F(GeneratedMeshMFEMTest, BlockIDZero)
{
  InputParameters mesh_params = getValidParams();
  InputParameters generator_params = generatedMeshParams(2);
  generator_params.set<std::vector<SubdomainID>>("subdomain_ids") = {0};
  EXPECT_MOOSEERROR_MSG_CONTAINS(buildMesh(mesh_params, "GeneratedMeshGenerator", generator_params),
                                 "Block ID 0 can not be represented in an MFEM mesh");
}

TEST_F(GeneratedMeshMFEMTest, BoundaryIDZero)
{
  InputParameters mesh_params = getValidParams();
  InputParameters generator_params = generatedMeshParams(2);
  generator_params.set<boundary_id_type>("boundary_id_offset") = 0;
  EXPECT_MOOSEERROR_MSG_CONTAINS(buildMesh(mesh_params, "GeneratedMeshGenerator", generator_params),
                                 "Boundary ID 0 can not be represented in an MFEM mesh");
}

TEST_F(GeneratedMeshMFEMTest, Quad8CentreNodes)
{
  // MFEM's second-order quadrilateral has a centre node, which a QUAD8 lacks, so it is
  // interpolated from the other nodes of each element.
  InputParameters mesh_params = getValidParams();
  InputParameters generator_params = generatedMeshParams(2);
  generator_params.set<unsigned int>("nx") = 2;
  generator_params.set<unsigned int>("ny") = 2;
  generator_params.set<MooseEnum>("elem_type") = "QUAD8";
  buildMesh(mesh_params, "GeneratedMeshGenerator", generator_params);

  const auto & fespace = *_mfem_mesh_ptr->GetNodalFESpace();
  mfem::Array<int> vertices, dofs;
  for (const auto e : make_range(_mfem_mesh_ptr->GetNE()))
  {
    // The elements are straight-sided, so the centre node lies at the mean of the vertices.
    _mfem_mesh_ptr->GetElementVertices(e, vertices);
    Coord centre(2, 0.);
    for (const auto v : vertices)
      for (const auto d : make_range(2))
        centre[d] += _mfem_mesh_ptr->GetVertex(v)[d] / vertices.Size();

    // The centre node is the last degree of freedom of a second-order quadrilateral.
    fespace.GetElementDofs(e, dofs);
    Coord node(2);
    _mfem_mesh_ptr->GetNode(dofs.Last(), node.data());
    EXPECT_DOUBLE_EQ(node[0], centre[0]);
    EXPECT_DOUBLE_EQ(node[1], centre[1]);
  }
}

/**
 * Check that the higher-order mesh \p mesh has the same geometry as \p first_order_mesh, which
 * holds the same elements in the same order. This holds for straight-sided meshes, and fails if
 * any higher-order node is misplaced.
 */
void
expectSameGeometry(mfem::ParMesh & mesh, mfem::ParMesh & first_order_mesh)
{
  ASSERT_NE(mesh.GetNodes(), nullptr);
  ASSERT_EQ(mesh.GetNE(), first_order_mesh.GetNE());
  mfem::Vector x, x_first_order;
  for (const auto e : make_range(mesh.GetNE()))
  {
    auto & transformation = *mesh.GetElementTransformation(e);
    auto & first_order_transformation = *first_order_mesh.GetElementTransformation(e);
    // Compare at the nodes of the higher-order element, which include any interpolated nodes.
    const auto & points = mesh.GetNodalFESpace()->GetFE(e)->GetNodes();
    for (const auto i : make_range(points.GetNPoints()))
    {
      transformation.Transform(points.IntPoint(i), x);
      first_order_transformation.Transform(points.IntPoint(i), x_first_order);
      // The two mappings are evaluated with different bases, so they agree only to round-off,
      // which is far below 1e-12 for coordinates of order one. A misplaced node would move
      // points by a fraction of the element size.
      for (const auto d : make_range(x.Size()))
        EXPECT_NEAR(x(d), x_first_order(d), 1e-12);
    }
  }
}

/// Element type, mesh dimension, and whether fallback element types are allowed
using HigherOrderGeometryParams = std::tuple<std::string, unsigned int, bool>;

/**
 * Converts straight-sided meshes of several higher-order elements, and compares their geometry
 * with that of the same mesh converted to first-order elements. Any misplaced higher-order node
 * makes the two differ.
 */
class HigherOrderGeometryMFEMTest : public LibMeshToMFEMMeshTest<MeshGeneratorMesh>,
                                    public testing::WithParamInterface<HigherOrderGeometryParams>
{
};

TEST_P(HigherOrderGeometryMFEMTest, MatchesFirstOrder)
{
  const auto & [elem_type, dim, fallback] = GetParam();
  // Fallback element types produce a warning.
  Moose::_throw_on_warning = false;

  InputParameters mesh_params = getValidParams();
  InputParameters generator_params = generatedMeshParams(dim);
  generator_params.set<MooseEnum>("elem_type") = elem_type;
  generator_params.set<unsigned int>("nx") = 2;
  if (dim > 1)
    generator_params.set<unsigned int>("ny") = 2;
  if (dim > 2)
    generator_params.set<unsigned int>("nz") = 2;
  buildMesh(mesh_params, "GeneratedMeshGenerator", generator_params, fallback);
  const auto first_order_moose_mesh =
      buildAdditionalMesh(mesh_params, "GeneratedMeshGenerator", generator_params);
  const auto first_order_mesh =
      Moose::MFEM::buildMFEMMesh(*first_order_moose_mesh, fallback, /*first_order=*/true);

  expectSameGeometry(*_mfem_mesh_ptr, *first_order_mesh);
}

INSTANTIATE_TEST_SUITE_P(MultiElement,
                         HigherOrderGeometryMFEMTest,
                         testing::Values(HigherOrderGeometryParams{"EDGE3", 1, false},
                                         HigherOrderGeometryParams{"EDGE4", 1, false},
                                         HigherOrderGeometryParams{"TRI6", 2, false},
                                         HigherOrderGeometryParams{"TRI7", 2, true},
                                         HigherOrderGeometryParams{"QUAD8", 2, false},
                                         HigherOrderGeometryParams{"QUAD9", 2, false},
                                         HigherOrderGeometryParams{"TET10", 3, false},
                                         HigherOrderGeometryParams{"TET14", 3, true},
                                         HigherOrderGeometryParams{"HEX20", 3, false},
                                         HigherOrderGeometryParams{"HEX27", 3, false},
                                         HigherOrderGeometryParams{"PRISM15", 3, false},
                                         HigherOrderGeometryParams{"PRISM18", 3, false}),
                         [](const auto & info) { return std::get<0>(info.param); });

/**
 * Tests for PRISM20 and PRISM21 elements, which MFEM can only represent as PRISM18s. No mesh
 * generator produces them directly, so a mesh of PRISM6s is converted to PRISM21s, whose first
 * 20 nodes are those of a PRISM20.
 */
class CompletePrismMFEMTest : public LibMeshToMFEMMeshTest<MeshGeneratorMesh>,
                              public testing::WithParamInterface<libMesh::ElemType>
{
protected:
  std::shared_ptr<MeshGeneratorMesh> buildPrismMesh(const std::string & name);
};

std::shared_ptr<MeshGeneratorMesh>
CompletePrismMFEMTest::buildPrismMesh(const std::string & name)
{
  InputParameters mesh_params = getValidParams();
  InputParameters generator_params = generatedMeshParams(3);
  generator_params.set<MooseEnum>("elem_type") = "PRISM6";
  generator_params.set<unsigned int>("nx") = 2;
  generator_params.set<unsigned int>("ny") = 2;
  generator_params.set<unsigned int>("nz") = 2;
  auto moose_mesh = buildMooseMeshOnly(
      mesh_params, "GeneratedMeshGenerator", generator_params, name, name + "_generator");
  auto & mesh = moose_mesh->getMesh();
  mesh.all_complete_order();

  if (GetParam() == libMesh::ElemType::PRISM20)
  {
    // The boundary information refers to the elements being replaced, and is not needed here.
    mesh.get_boundary_info().clear();
    std::vector<dof_id_type> element_ids;
    for (const auto * const elem : mesh.element_ptr_range())
      element_ids.push_back(elem->id());
    for (const auto id : element_ids)
    {
      auto & prism21 = mesh.elem_ref(id);
      auto prism20 = libMesh::Elem::build(libMesh::ElemType::PRISM20);
      for (const auto n : make_range(prism20->n_nodes()))
        prism20->set_node(n, prism21.node_ptr(n));
      prism20->subdomain_id() = prism21.subdomain_id();
      prism20->set_id(id);
      mesh.insert_elem(std::move(prism20));
    }
    mesh.prepare_for_use();
  }

  for (const auto * const elem : mesh.element_ptr_range())
    EXPECT_EQ(elem->type(), GetParam());
  return moose_mesh;
}

TEST_P(CompletePrismMFEMTest, FallbackMatchesFirstOrder)
{
  // Falling back to PRISM18 elements produces a warning.
  Moose::_throw_on_warning = false;
  const auto mesh = buildPrismMesh("prism_mesh");
  const auto mfem_mesh = Moose::MFEM::buildMFEMMesh(*mesh, /*fallback=*/true, false);
  EXPECT_EQ(mfem_mesh->GetElementType(0), mfem::Element::WEDGE);
  const auto first_order_mfem_mesh = Moose::MFEM::buildMFEMMesh(*mesh, true, /*first_order=*/true);
  expectSameGeometry(*mfem_mesh, *first_order_mfem_mesh);
}

TEST_P(CompletePrismMFEMTest, ErrorWithoutFallback)
{
  const auto mesh = buildPrismMesh("prism_mesh");
  EXPECT_MOOSEERROR_MSG_CONTAINS(Moose::MFEM::buildMFEMMesh(*mesh, false, false),
                                 "Can not represent libMesh element type ");
}

INSTANTIATE_TEST_SUITE_P(FallbackElementSupport,
                         CompletePrismMFEMTest,
                         testing::Values(libMesh::ElemType::PRISM20, libMesh::ElemType::PRISM21),
                         [](const auto & info)
                         { return libMesh::Utility::enum_to_string(info.param); });

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
    _elem_type = libmesh_map_find(types, _filename.substr(0, split_at));
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
  for (const auto i : make_range(n_elem))
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
  ASSERT_EQ(_mfem_mesh_ptr->attribute_sets.GetAttributeSetNames(), std::set<std::string>());
  ASSERT_EQ(_mfem_mesh_ptr->bdr_attribute_sets.GetAttributeSetNames(),
            std::set<std::string>({"top", "bottom"}));
  EXPECT_EQ(_mfem_mesh_ptr->bdr_attribute_sets.GetAttributeSet("top").Size(), 1);
  EXPECT_EQ(_mfem_mesh_ptr->bdr_attribute_sets.GetAttributeSet("bottom").Size(), 1);
}

INSTANTIATE_TEST_SUITE_P(
    SimpleCubes,
    FileMeshMFEMTest,
    testing::Values(
        "tet-4.e", "tet-10.e", "hex-8.e", "hex-27.e", "wedge-6.e", "wedge-18.e", "pyramid-5.e"));

Point
toPoint(const Coord & x, libMesh::ElemType etype)
{
  switch (etype)
  {
    case libMesh::ElemType::EDGE2:
    case libMesh::ElemType::EDGE3:
    case libMesh::ElemType::EDGE4:
      return Point(2 * x[0] - 1);
    case libMesh::ElemType::TRI3:
    case libMesh::ElemType::TRI6:
    case libMesh::ElemType::TRI7:
      return Point(x[0], x[1]);
    case libMesh::ElemType::QUAD4:
    case libMesh::ElemType::QUAD8:
    case libMesh::ElemType::QUAD9:
      return Point(2 * x[0] - 1, 2 * x[1] - 1);
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
 * the case for MFEM.
 */
void
checkTransform(const Elem * libmesh_elem,
               mfem::ElementTransformation & mfem_transform,
               const std::vector<Coord> & ref_coords)
{
  // Assemble matrices to use for the MFEM transform
  const int M = ref_coords[0].size();
  const int N = ref_coords.size();
  mfem::DenseMatrix ref_coords_mat(M, N), actual_phys_coords_mat(M, N);
  for (const auto i : make_range(M))
  {
    for (const auto j : make_range(N))
    {
      ref_coords_mat(i, j) = ref_coords[j][i];
    }
  }
  mfem_transform.Transform(ref_coords_mat, actual_phys_coords_mat);
  for (const auto j : make_range(N))
  {
    std::string ref_coord;
    for (const auto i : make_range(M))
    {
      if (i > 0)
      {
        ref_coord += ", ";
      }
      ref_coord += std::to_string(ref_coords[j][i]);
    }
    Point expected =
        libMesh::FEMap::map(M, libmesh_elem, toPoint(ref_coords[j], libmesh_elem->type()));
    // The two mappings are evaluated with different bases, so they agree only to round-off,
    // which is far below 1e-12 for coordinates of order one.
    for (const auto i : make_range(M))
    {
      EXPECT_NEAR(actual_phys_coords_mat(i, j), expected(i), 1e-12)
          << "Unexpected element " << i << " of physical coordinate at (" << ref_coord
          << ") in reference space";
    }
  }
}

struct ElementTestData
{
  InputParameters mesh_params;
  InputParameters mg_params;
  std::set<std::set<Coord>> expected_element;
  std::vector<Coord> test_coords;
  bool higher_order;
  mfem::Element::Type type;
};

ElementTestData edge2Data();
ElementTestData edge3Data();
ElementTestData edge4Data();
ElementTestData tri3Data();
ElementTestData tri6Data();
ElementTestData tri7Data();
ElementTestData quad4Data();
ElementTestData quad8Data();
ElementTestData quad9Data();
ElementTestData tet4Data();
ElementTestData tet10Data();
ElementTestData tet14Data();
ElementTestData pyr5Data();
ElementTestData pyr13Data();
ElementTestData pyr14Data();
ElementTestData prism6Data();
ElementTestData prism15Data();
ElementTestData prism18Data();
ElementTestData hex8Data();
ElementTestData hex20Data();
ElementTestData hex27Data();

enum class ElementCase
{
  Edge2,
  Edge3,
  Edge4,
  Tri3,
  Tri6,
  Tri7,
  Quad4,
  Quad8,
  Quad9,
  Tet4,
  Tet10,
  Tet14,
  Pyr5,
  Pyr13,
  Pyr14,
  Prism6,
  Prism15,
  Prism18,
  Hex8,
  Hex20,
  Hex27
};

ElementTestData
elementData(ElementCase elem_case)
{
  switch (elem_case)
  {
    case ElementCase::Edge2:
      return edge2Data();
    case ElementCase::Edge3:
      return edge3Data();
    case ElementCase::Edge4:
      return edge4Data();
    case ElementCase::Tri3:
      return tri3Data();
    case ElementCase::Tri6:
      return tri6Data();
    case ElementCase::Tri7:
      return tri7Data();
    case ElementCase::Quad4:
      return quad4Data();
    case ElementCase::Quad8:
      return quad8Data();
    case ElementCase::Quad9:
      return quad9Data();
    case ElementCase::Tet4:
      return tet4Data();
    case ElementCase::Tet10:
      return tet10Data();
    case ElementCase::Tet14:
      return tet14Data();
    case ElementCase::Pyr5:
      return pyr5Data();
    case ElementCase::Pyr13:
      return pyr13Data();
    case ElementCase::Pyr14:
      return pyr14Data();
    case ElementCase::Prism6:
      return prism6Data();
    case ElementCase::Prism15:
      return prism15Data();
    case ElementCase::Prism18:
      return prism18Data();
    case ElementCase::Hex8:
      return hex8Data();
    case ElementCase::Hex20:
      return hex20Data();
    case ElementCase::Hex27:
      return hex27Data();
  }

  mooseError("Unhandled element test case.");
}

std::string
elementCaseName(ElementCase elem_case)
{
  switch (elem_case)
  {
    case ElementCase::Edge2:
      return "EDGE2";
    case ElementCase::Edge3:
      return "EDGE3";
    case ElementCase::Edge4:
      return "EDGE4";
    case ElementCase::Tri3:
      return "TRI3";
    case ElementCase::Tri6:
      return "TRI6";
    case ElementCase::Tri7:
      return "TRI7";
    case ElementCase::Quad4:
      return "QUAD4";
    case ElementCase::Quad8:
      return "QUAD8";
    case ElementCase::Quad9:
      return "QUAD9";
    case ElementCase::Tet4:
      return "TET4";
    case ElementCase::Tet10:
      return "TET10";
    case ElementCase::Tet14:
      return "TET14";
    case ElementCase::Pyr5:
      return "PYRAMID5";
    case ElementCase::Pyr13:
      return "PYRAMID13";
    case ElementCase::Pyr14:
      return "PYRAMID14";
    case ElementCase::Prism6:
      return "PRISM6";
    case ElementCase::Prism15:
      return "PRISM15";
    case ElementCase::Prism18:
      return "PRISM18";
    case ElementCase::Hex8:
      return "HEX8";
    case ElementCase::Hex20:
      return "HEX20";
    case ElementCase::Hex27:
      return "HEX27";
  }

  mooseError("Unhandled element test case.");
}

std::pair<std::string, int>
firstOrderElementInfo(ElementCase elem_case)
{
  switch (elem_case)
  {
    case ElementCase::Edge2:
    case ElementCase::Edge3:
    case ElementCase::Edge4:
      return {"EDGE2", 2};
    case ElementCase::Tri3:
    case ElementCase::Tri6:
    case ElementCase::Tri7:
      return {"TRI3", 3};
    case ElementCase::Quad4:
    case ElementCase::Quad8:
    case ElementCase::Quad9:
      return {"QUAD4", 4};
    case ElementCase::Tet4:
    case ElementCase::Tet10:
    case ElementCase::Tet14:
      return {"TET4", 4};
    case ElementCase::Pyr5:
    case ElementCase::Pyr13:
    case ElementCase::Pyr14:
      return {"PYRAMID5", 5};
    case ElementCase::Prism6:
    case ElementCase::Prism15:
    case ElementCase::Prism18:
      return {"PRISM6", 6};
    case ElementCase::Hex8:
    case ElementCase::Hex20:
    case ElementCase::Hex27:
      return {"HEX8", 8};
  }

  mooseError("Unhandled element test case.");
}

ElementTestData
edge2Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "EDGE2";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{1., 0., 0.}, {2., 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{1.}, {2.}}},
          {{0.}, {0.5}, {1.}},
          false,
          mfem::Element::Type::SEGMENT};
}

ElementTestData
edge3Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "EDGE3";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{0., 0., 0.}, {4., 0., 0.}, {1., 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0.}, {4.}}},
          {{0.}, {0.5}, {1.}},
          true,
          mfem::Element::Type::SEGMENT};
}

ElementTestData
edge4Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "EDGE4";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., 0., 0.}, {26., 0., 0.}, {0., 0., 0.}, {7., 0., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1.}, {26.}}},
          {{0.25}, {0.75}, {1.}},
          true,
          mfem::Element::Type::SEGMENT};
}

ElementTestData
tri3Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TRI3";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2};
  mg_params.set<std::vector<Point>>("nodal_positions") = {{0., 0., 0.}, {2., 0., 0.}, {0., 2., 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0.}, {2., 0.}, {0., 2.}}},
          {{0., 0.}, {0.5, 0.25}, {1., 0.}},
          false,
          mfem::Element::TRIANGLE};
}

ElementTestData
tri6Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TRI6";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3, 4, 5};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {0., 0., 0.}, {1., 0., 0.}, {0., 1., 0.}, {.5, 0.1, 0.0}, {0.5, 0.5, 0.0}, {0., .5, 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0.}, {1., 0.}, {0., 1.}}},
          {{0., 0.5}, {0.25, 0.0}, {0.4, 0.2}},
          true,
          mfem::Element::TRIANGLE};
}

ElementTestData
tri7Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TRI7";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3, 4, 5, 6};
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
          {{0., 0.5}, {0.25, 0.0}, {0.4, 0.2}},
          true,
          mfem::Element::TRIANGLE};
}

ElementTestData
quad4Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "QUAD4";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {0., 0., 0.}, {1., 0., 0.}, {2., 2., 0.}, {0.5, 1.0, 0.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{0., 0.}, {1., 0.}, {0.5, 1.}, {2., 2.}}},
          {{0., 0.5}, {0.75, 1.}, {0.5, 0.5}},
          false,
          mfem::Element::QUADRILATERAL};
}

ElementTestData
quad8Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "QUAD8";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3, 4, 5, 6, 7};
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
          {{0., 0.5}, {0.75, 0.5}, {0.5, 0.25}},
          true,
          mfem::Element::QUADRILATERAL};
}

ElementTestData
quad9Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "QUAD9";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3, 4, 5, 6, 7, 8};
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
          {{0., 0.5}, {0.75, 0.5}, {0.5, 0.25}},
          true,
          mfem::Element::QUADRILATERAL};
}

ElementTestData
tet4Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TET4";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., -1., 0.}, {1., -1., 0.}, {0., 1., 0.}, {0., 0.0, -1.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., 0.}, {1., -1., 0.}, {0., 1., 0.}, {0., 0., -1.}}},
          {{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.125, 0.25, 0.5}},
          false,
          mfem::Element::TETRAHEDRON};
}

ElementTestData
tet10Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TET10";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
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
          {{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.125, 0.25, 0.5}},
          true,
          mfem::Element::TETRAHEDRON};
}

ElementTestData
tet14Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "TET14";
  mg_params.set<Connectivity>("element_connectivity") = {
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
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
          {{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.125, 0.25, 0.5}},
          true,
          mfem::Element::TETRAHEDRON};
}

ElementTestData
pyr5Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PYRAMID5";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3, 4};
  mg_params.set<std::vector<Point>>("nodal_positions") = {
      {-1., -1., 0.}, {1., -1., 0.}, {1., 1., 0.}, {-1., 1.0, 0.}, {1.5, 0., 2.}};
  mg_params.set<unsigned short>("subdomain_id") = 1;
  return {mesh_params,
          mg_params,
          {{{-1., -1., 0.}, {1., -1., 0.}, {1., 1., 0.}, {-1., 1., 0.}, {1.5, 0., 2}}},
          {{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.25, 0.25, 0.75}},
          false,
          mfem::Element::PYRAMID};
}

ElementTestData
pyr13Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PYRAMID13";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
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
          {{0.25, 0.5, 0.}, {0.5, 0., 0.3}, {0.25, 0.25, 0.5}},
          true,
          mfem::Element::PYRAMID};
}

ElementTestData
pyr14Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PYRAMID14";
  mg_params.set<Connectivity>("element_connectivity") = {
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
          {{0., 0.5, 0.5}, {0.5, 0., 0.3}, {0.25, 0.25, 0.6}},
          true,
          mfem::Element::PYRAMID};
}

ElementTestData
prism6Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PRISM6";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3, 4, 5};
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
          {{0., 0.5, 0.5}, {0.25, 0.5, 0.}, {0.5, 0.5, 0.75}},
          false,
          mfem::Element::WEDGE};
}

ElementTestData
prism15Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PRISM15";
  mg_params.set<Connectivity>("element_connectivity") = {
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
          {{0.25, 0.5, 0.}, {0.5, 0., 0.8}, {0.5, 0.5, 0.75}},
          true,
          mfem::Element::WEDGE};
}

ElementTestData
prism18Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "PRISM18";
  mg_params.set<Connectivity>("element_connectivity") = {
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
          {{0.25, 0.5, 0.}, {0.5, 0., 0.8}, {0.5, 0.5, 0.75}},
          true,
          mfem::Element::WEDGE};
}

ElementTestData
hex8Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "HEX8";
  mg_params.set<Connectivity>("element_connectivity") = {0, 1, 2, 3, 4, 5, 6, 7};
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
          {{0., 0.5, 0.5}, {0.5, 0.5, 1.}, {0.5, 0.5, 0.5}},
          false,
          mfem::Element::HEXAHEDRON};
}

ElementTestData
hex20Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "HEX20";
  mg_params.set<Connectivity>("element_connectivity") = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
                                                         10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
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
          {{1., 0.4, 0.5}, {0.75, 0., 0.5}, {0.1, 0.25, 0.25}},
          true,
          mfem::Element::HEXAHEDRON};
}

ElementTestData
hex27Data()
{
  InputParameters mesh_params = MeshGeneratorMesh::validParams();
  InputParameters mg_params = ElementGenerator::validParams();
  mg_params.set<MooseEnum>("elem_type") = "HEX27";
  mg_params.set<Connectivity>("element_connectivity") = {0,  1,  2,  3,  4,  5,  6,  7,  8,
                                                         9,  10, 11, 12, 13, 14, 15, 16, 17,
                                                         18, 19, 20, 21, 22, 23, 24, 25, 26};
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
          {{1., 0.4, 0.5}, {0.75, 0., 0.5}, {0.1, 0.25, 0.25}},
          true,
          mfem::Element::HEXAHEDRON};
}

class ElementGeneratorMFEMTest : public LibMeshToMFEMMeshTest<MeshGeneratorMesh>,
                                 public testing::WithParamInterface<ElementCase>
{
public:
  static std::string testParamName(const testing::TestParamInfo<ElementCase> & info)
  {
    return elementCaseName(info.param);
  }
};

TEST_P(ElementGeneratorMFEMTest, CheckElem)
{
  auto data = elementData(GetParam());
  buildMesh(data.mesh_params, "ElementGenerator", data.mg_params, true, false);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord>> actual_elements = getElementSet(_mfem_mesh_ptr, data.type);
  EXPECT_EQ(data.expected_element, actual_elements);
  if (data.higher_order)
  {
    EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  }
  else
  {
    EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  }
  const int N = _mfem_mesh_ptr->SpaceDimension();
  mfem::DenseMatrix jac(N, N);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);
  checkTransform(
      _moose_mesh_ptr->elemPtr(0), *_mfem_mesh_ptr->GetElementTransformation(0), data.test_coords);
}

INSTANTIATE_TEST_SUITE_P(SingleElement,
                         ElementGeneratorMFEMTest,
                         testing::Values(ElementCase::Edge2,
                                         ElementCase::Edge3,
                                         ElementCase::Edge4,
                                         ElementCase::Tri3,
                                         ElementCase::Tri6,
                                         ElementCase::Quad4,
                                         ElementCase::Quad8,
                                         ElementCase::Quad9,
                                         ElementCase::Tet4,
                                         ElementCase::Tet10,
                                         ElementCase::Pyr5,
                                         // A bug in MFEM currently means we can't support
                                         // higher-order libmesh pyramids. Once that is fixed
                                         // these cases should be uncommented.
                                         // ElementCase::Pyr13,
                                         // ElementCase::Pyr14,
                                         ElementCase::Prism6,
                                         ElementCase::Prism15,
                                         ElementCase::Prism18,
                                         ElementCase::Hex8,
                                         ElementCase::Hex20,
                                         ElementCase::Hex27),
                         ElementGeneratorMFEMTest::testParamName);

using UnsupportedElementGeneratorMFEMTest = ElementGeneratorMFEMTest;

TEST_P(UnsupportedElementGeneratorMFEMTest, CheckElem)
{
  auto data = elementData(GetParam());
  EXPECT_MOOSEERROR_MSG(
      buildMesh(data.mesh_params, "ElementGenerator", data.mg_params),
      "Due to bug in MFEM, can not convert higher order libMesh pyramid elements.");
}

// A bug in MFEM currently means we can't support higher-order
// libmesh pyramids. Once that is fixed this test should be removed.
INSTANTIATE_TEST_SUITE_P(SingleElement,
                         UnsupportedElementGeneratorMFEMTest,
                         testing::Values(ElementCase::Pyr13, ElementCase::Pyr14),
                         UnsupportedElementGeneratorMFEMTest::testParamName);

enum class FallbackElementCase
{
  Tri7,
  Tet14
};

std::pair<ElementTestData, ElementTestData>
fallbackElementData(FallbackElementCase fallback_case)
{
  switch (fallback_case)
  {
    case FallbackElementCase::Tri7:
      return {tri7Data(), tri6Data()};
    case FallbackElementCase::Tet14:
      return {tet14Data(), tet10Data()};
  }

  mooseError("Unhandled fallback element test case.");
}

class ElementComparisonMFEMTest : public LibMeshToMFEMMeshTest<MeshGeneratorMesh>,
                                  public testing::WithParamInterface<FallbackElementCase>
{
public:
  static std::string testParamName(const testing::TestParamInfo<FallbackElementCase> & info)
  {
    switch (info.param)
    {
      case FallbackElementCase::Tri7:
        return "TRI7";
      case FallbackElementCase::Tet14:
        return "TET14";
    }

    mooseError("Unhandled fallback element test case.");
  }
};

using LibMeshToMFEMMeshFallbackTest = ElementComparisonMFEMTest;

TEST_P(LibMeshToMFEMMeshFallbackTest, CheckWarningThrown)
{
  auto data = fallbackElementData(GetParam()).first;
  // A warning should be emitted when converting from TRI7 to TRI6
  EXPECT_THROW(buildMesh(data.mesh_params, "ElementGenerator", data.mg_params, true, false),
               std::runtime_error);
}

TEST_P(LibMeshToMFEMMeshFallbackTest, CheckFallbackCorrect)
{
  auto [data, expected_data] = fallbackElementData(GetParam());
  // Temporarily allow warnings to be emitted, so we can test the
  // conversion actually works properly.
  const auto throw_on_warning = Moose::_throw_on_warning;
  const auto warnings_are_errors = Moose::_warnings_are_errors;
  Moose::_throw_on_warning = false;
  Moose::_warnings_are_errors = false;
  try
  {
    buildMesh(data.mesh_params, "ElementGenerator", data.mg_params, true, false);
  }
  catch (...)
  {
    Moose::_throw_on_warning = throw_on_warning;
    Moose::_warnings_are_errors = warnings_are_errors;
    throw;
  }
  Moose::_throw_on_warning = throw_on_warning;
  Moose::_warnings_are_errors = warnings_are_errors;
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord>> actual_elements = getElementSet(_mfem_mesh_ptr, data.type);
  EXPECT_EQ(data.expected_element, actual_elements);
  EXPECT_NE(_mfem_mesh_ptr->GetNodes(), nullptr);
  const int N = _mfem_mesh_ptr->SpaceDimension();
  mfem::DenseMatrix jac(N, N);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);

  std::shared_ptr<MeshGeneratorMesh> expected_mesh =
      buildAdditionalMesh(expected_data.mesh_params, "ElementGenerator", expected_data.mg_params);
  checkTransform(
      expected_mesh->elemPtr(0), *_mfem_mesh_ptr->GetElementTransformation(0), data.test_coords);
}

TEST_P(LibMeshToMFEMMeshFallbackTest, CheckError)
{
  auto data = fallbackElementData(GetParam()).first;
  EXPECT_MOOSEERROR_MSG_CONTAINS(buildMesh(data.mesh_params, "ElementGenerator", data.mg_params),
                                 "Can not represent libMesh element type ");
}

INSTANTIATE_TEST_SUITE_P(FallbackElementSupport,
                         LibMeshToMFEMMeshFallbackTest,
                         testing::Values(FallbackElementCase::Tri7, FallbackElementCase::Tet14),
                         ElementComparisonMFEMTest::testParamName);

class LibMeshToMFEMMeshFirstOrderTest : public LibMeshToMFEMMeshTest<MeshGeneratorMesh>,
                                        public testing::WithParamInterface<ElementCase>
{
public:
  static std::string testParamName(const testing::TestParamInfo<ElementCase> & info)
  {
    return elementCaseName(info.param);
  }
};

TEST_P(LibMeshToMFEMMeshFirstOrderTest, CheckConversion)
{
  auto data = elementData(GetParam());
  buildMesh(data.mesh_params, "ElementGenerator", data.mg_params, false, true);
  ASSERT_EQ(_moose_mesh_ptr->nElem(), 1);
  std::set<std::set<Coord>> actual_elements = getElementSet(_mfem_mesh_ptr, data.type);
  EXPECT_EQ(data.expected_element, actual_elements);
  EXPECT_EQ(_mfem_mesh_ptr->GetNodes(), nullptr);
  const int N = _mfem_mesh_ptr->SpaceDimension();
  mfem::DenseMatrix jac(N, N);
  _mfem_mesh_ptr->GetElementJacobian(0, jac);
  EXPECT_GT(jac.Det(), 0.);

  const auto [first_order_elem_type, M] = firstOrderElementInfo(GetParam());
  data.mg_params.set<MooseEnum>("elem_type") = first_order_elem_type;
  data.mg_params.set<Connectivity>("element_connectivity").resize(M);
  data.mg_params.set<std::vector<Point>>("nodal_positions").resize(M);
  std::shared_ptr<MeshGeneratorMesh> expected_mesh =
      buildAdditionalMesh(data.mesh_params, "ElementGenerator", data.mg_params);
  checkTransform(
      expected_mesh->elemPtr(0), *_mfem_mesh_ptr->GetElementTransformation(0), data.test_coords);
}

INSTANTIATE_TEST_SUITE_P(ConvertToFirstOrder,
                         LibMeshToMFEMMeshFirstOrderTest,
                         testing::Values(ElementCase::Edge2,
                                         ElementCase::Edge3,
                                         ElementCase::Edge4,
                                         ElementCase::Tri3,
                                         ElementCase::Tri6,
                                         ElementCase::Tri7,
                                         ElementCase::Quad4,
                                         ElementCase::Quad8,
                                         ElementCase::Quad9,
                                         ElementCase::Tet4,
                                         ElementCase::Tet10,
                                         ElementCase::Tet14,
                                         ElementCase::Pyr5,
                                         ElementCase::Pyr13,
                                         ElementCase::Pyr14,
                                         ElementCase::Prism6,
                                         ElementCase::Prism15,
                                         ElementCase::Prism18,
                                         ElementCase::Hex8,
                                         ElementCase::Hex20,
                                         ElementCase::Hex27),
                         LibMeshToMFEMMeshFirstOrderTest::testParamName);

#endif
