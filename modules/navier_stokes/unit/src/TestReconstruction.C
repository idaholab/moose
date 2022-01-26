//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Registry.h"
#include "MooseMesh.h"
#include "NavierStokesApp.h"
#include "AppFactory.h"
#include "Factory.h"
#include "InputParameters.h"
#include "MeshGeneratorMesh.h"
#include "MooseError.h"
#include "CastUniquePointer.h"
#include "GeneratedMeshGenerator.h"
#include "MooseFunctor.h"
#include "PolynomialFit.h"
#include "FaceInfo.h"
#include "MooseTypes.h"
#include "CellCenteredMapFunctor.h"
#include "Reconstructions.h"

#include "libmesh/elem.h"
#include "libmesh/tensor_value.h"
#include "libmesh/point.h"
#include "libmesh/vector_value.h"
#include "libmesh/utility.h"

#include <memory>
#include <vector>
#include <memory>

void
testReconstruction(const Moose::CoordinateSystemType coord_type)
{
  const char * argv[2] = {"foo", "\0"};

  MultiMooseEnum coord_type_enum("XYZ RZ RSPHERICAL", "XYZ");
  coord_type_enum = (coord_type == Moose::COORD_XYZ) ? "XYZ" : "RZ";

  std::vector<unsigned int> num_elem = {64, 128, 256};
  std::vector<Real> weller_errors;
  std::vector<Real> moukalled_errors;
  std::vector<Real> tano_errors;
  std::vector<Real> tano_errors_twice;
  std::vector<Real> h(num_elem.size());
  for (const auto i : index_range(num_elem))
    h[i] = 1. / num_elem[i];

  for (const auto i : index_range(num_elem))
  {
    const auto nx = num_elem[i];
    auto app = AppFactory::createAppShared("NavierStokesApp", 1, (char **)argv);
    auto * factory = &app->getFactory();
    std::string mesh_type = "MeshGeneratorMesh";

    std::shared_ptr<MeshGeneratorMesh> mesh;
    {
      InputParameters params = factory->getValidParams(mesh_type);
      mesh = factory->create<MeshGeneratorMesh>(mesh_type, "moose_mesh", params);
    }

    app->actionWarehouse().mesh() = mesh;

    {
      std::unique_ptr<MeshBase> lm_mesh;
      InputParameters params = factory->getValidParams("GeneratedMeshGenerator");
      params.set<unsigned int>("nx") = nx;
      params.set<unsigned int>("ny") = nx;
      params.set<MooseEnum>("dim") = "2";
      auto mesh_gen =
          factory->create<GeneratedMeshGenerator>("GeneratedMeshGenerator", "mesh_gen", params);
      lm_mesh = mesh_gen->generate();
      mesh->setMeshBase(std::move(lm_mesh));
    }

    mesh->prepare();
    mesh->setCoordSystem({}, coord_type_enum);
    mooseAssert(mesh->getAxisymmetricRadialCoord() == 0,
                "This should be 0 because we haven't set anything.");
    const auto & all_fi = mesh->allFaceInfo();
    mesh->computeFaceInfoFaceCoords();
    std::vector<const FaceInfo *> faces(all_fi.size());
    for (const auto i : index_range(all_fi))
      faces[i] = &all_fi[i];

    auto & lm_mesh = mesh->getMesh();

    CellCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>> u(
        *mesh);
    for (auto * const elem : lm_mesh.active_element_ptr_range())
    {
      const auto centroid = elem->vertex_average();
      const auto value = RealVectorValue(-std::sin(centroid(0)) * std::cos(centroid(1)),
                                         std::cos(centroid(0)) * std::sin(centroid(1)));
      u[elem->id()] = value;
    }

    CellCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
        up_weller(*mesh);
    std::unordered_map<dof_id_type, RealVectorValue> up_moukalled;
    CellCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
        up_tano(*mesh);

    for (const auto & fi : all_fi)
    {
      auto moukalled_reconstruct = [&fi](auto & functor, auto & container) {
        auto face = Moose::FV::makeCDFace(fi);
        const RealVectorValue uf(functor(face));
        const Point surface_vector = fi.normal() * fi.faceArea();
        auto product = (uf * fi.dCF()) * surface_vector;

        container[fi.elem().id()] += product * fi.gC() / fi.elemVolume();
        if (fi.neighborPtr())
          container[fi.neighbor().id()] +=
              std::move(product) * (1. - fi.gC()) / fi.neighborVolume();
      };

      moukalled_reconstruct(u, up_moukalled);
    }

    struct AllBlocks
    {
      bool hasBlocks(SubdomainID) const { return true; }
    };

    AllBlocks consumer;
    Moose::FV::interpolateReconstruct(up_weller, u, 1, true, faces, consumer);
    Moose::FV::interpolateReconstruct(up_tano, u, 1, false, faces, consumer);

    Real error = 0;
    Real weller_error = 0;
    Real tano_error = 0;
    Real moukalled_error = 0;
    const auto current_h = h[i];
    for (auto * const elem : lm_mesh.active_element_ptr_range())
    {
      const auto elem_id = elem->id();
      auto elem_arg = Moose::ElemArg{elem, false, false};
      const RealVectorValue analytic(u(elem_arg));

      auto compute_elem_error = [elem_id, current_h, &analytic](auto & container, auto & error) {
        auto & current = libmesh_map_find(container, elem_id);
        const auto diff = analytic - current;
        error += diff * diff * current_h * current_h;
      };

      compute_elem_error(up_weller, weller_error);
      compute_elem_error(up_moukalled, moukalled_error);
      compute_elem_error(up_tano, tano_error);
    }
    error = std::sqrt(error);
    weller_error = std::sqrt(weller_error);
    moukalled_error = std::sqrt(moukalled_error);
    tano_error = std::sqrt(tano_error);
    weller_errors.push_back(weller_error);
    moukalled_errors.push_back(moukalled_error);
    tano_errors.push_back(tano_error);

    up_tano.clear();
    Moose::FV::interpolateReconstruct(up_tano, u, 2, false, faces, consumer);

    tano_error = 0;
    for (auto * const elem : lm_mesh.active_element_ptr_range())
    {
      const auto elem_id = elem->id();
      const auto elem_arg = Moose::ElemArg{elem, false, false};
      const RealVectorValue analytic(u(elem_arg));

      auto compute_elem_error = [elem_id, current_h, &analytic](auto & container, auto & error) {
        auto & current = libmesh_map_find(container, elem_id);
        const auto diff = analytic - current;
        error += diff * diff * current_h * current_h;
      };
      compute_elem_error(up_tano, tano_error);
    }
    tano_error = std::sqrt(tano_error);
    tano_errors_twice.push_back(tano_error);
  }

  std::for_each(h.begin(), h.end(), [](Real & h_elem) { h_elem = std::log(h_elem); });

  auto expect_errors = [&h](auto & errors_arg, Real expected_error) {
    std::for_each(
        errors_arg.begin(), errors_arg.end(), [](Real & error) { error = std::log(error); });
    PolynomialFit fit(h, errors_arg, 1);
    fit.generate();

    const auto & coeffs = fit.getCoefficients();
    EXPECT_NEAR(coeffs[1], expected_error, .05);
  };

  expect_errors(weller_errors, coord_type == Moose::COORD_RZ ? 1.5 : 2);
  expect_errors(moukalled_errors, 2);
  expect_errors(tano_errors, 2);
  expect_errors(tano_errors_twice, 2);
}

TEST(TestReconstruction, Cartesian) { testReconstruction(Moose::COORD_XYZ); }

TEST(TestReconstruction, Cylindrical) { testReconstruction(Moose::COORD_RZ); }
