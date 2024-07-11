#include "gtest/gtest.h"
#include "problem_builder.h"
#include "mfem.hpp"

TEST(CheckSetup, Variables)
{
  mfem::Mesh mesh("data/beam-tet.mesh", 1, 1);

  auto problem_builder = std::make_unique<platypus::TimeDomainProblemBuilder>();

  auto pmesh = std::make_shared<mfem::ParMesh>(MPI_COMM_WORLD, mesh);

  problem_builder->SetMesh(pmesh);
  problem_builder->AddFESpace(std::string("HCurl"), std::string("ND_3D_P2"));
  problem_builder->AddGridFunction(std::string("vector_potential"), std::string("HCurl"));

  auto problem = problem_builder->ReturnProblem();

  auto stored_gf = problem->_gridfunctions.Get("vector_potential");
  auto stored_fespace = problem->_fespaces.Get("HCurl");

  EXPECT_EQ(stored_fespace->GetVSize(), stored_gf->ParFESpace()->GetVSize());
  EXPECT_GT(stored_fespace->GetVSize(), 0);
}
