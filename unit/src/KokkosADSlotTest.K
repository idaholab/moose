//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "KokkosADKernel.h"

#include "MooseMain.h"
#include "AppFactory.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "NonlinearSystemBase.h"
#include "AuxiliarySystem.h"

#include "libmesh/coupling_matrix.h"

/**
 * AD kernel reading the value and gradient of its own variable and of a coupled variable, which
 * exposes its slot assignment for inspection
 */
class KokkosADSlotTestKernel : public Moose::Kokkos::ADKernel
{
public:
  /// u, grad u, v, and grad v
  static constexpr unsigned int max_ad_slots = 8;

  static InputParameters validParams()
  {
    InputParameters params = ADKernel::validParams();
    params.addRequiredCoupledVar("v", "The coupled variable");
    return params;
  }

  KokkosADSlotTestKernel(const InputParameters & parameters)
    : ADKernel(parameters),
      _u(adValue<max_ad_slots>()),
      _grad_u(adGradient<max_ad_slots>()),
      _v(adCoupledValue<max_ad_slots>("v")),
      _grad_v(adCoupledGradient<max_ad_slots>("v"))
  {
  }

  template <typename Derived>
  KOKKOS_FUNCTION Moose::Kokkos::DenseADReal<max_ad_slots>
  precomputeQpResidualValue(const unsigned int qp, AssemblyDatum & datum) const
  {
    return _u(datum, qp) * _v(datum, qp) + _grad_u(datum, qp) * _grad_v(datum, qp);
  }

  /// Slot table and slot assignments, for inspection
  ///@{
  const Moose::Kokkos::ADSlotTable & table() const { return adSlotTable(); }
  unsigned int uSlot() { return _u.slot().first; }
  unsigned int gradUSlot() { return _grad_u.slot().first; }
  unsigned int vSlot() { return _v.slot().first; }
  unsigned int gradVSlot() { return _grad_v.slot().first; }
  bool finalized() { return _u.slot().finalized && _v.slot().finalized; }
  ///@}

private:
  Moose::Kokkos::ADVariableValue<max_ad_slots> _u;
  Moose::Kokkos::ADVariableGradient<max_ad_slots> _grad_u;
  Moose::Kokkos::ADVariableValue<max_ad_slots> _v;
  Moose::Kokkos::ADVariableGradient<max_ad_slots> _grad_v;
};

registerKokkosADResidualObject("MooseUnitApp", KokkosADSlotTestKernel);

class KokkosADSlotTest : public ::testing::Test
{
public:
  KokkosADSlotTest()
    : _app(Moose::createMooseApp("MooseUnitApp", 0, nullptr)), _factory(_app->getFactory())
  {
    InputParameters mesh_params = _factory.getValidParams("GeneratedMesh");
    mesh_params.set<MooseEnum>("dim") = "2";
    mesh_params.set<unsigned int>("nx") = 1;
    mesh_params.set<unsigned int>("ny") = 1;
    _mesh = _factory.createUnique<MooseMesh>("GeneratedMesh", "moose_mesh", mesh_params);
    _mesh->setMeshBase(_mesh->buildMeshBaseObject());
    _mesh->buildMesh();
    _mesh->prepare();

    InputParameters problem_params = _factory.getValidParams("FEProblem");
    problem_params.set<MooseMesh *>("mesh") = _mesh.get();
    problem_params.set<std::string>(MooseBase::name_param) = "problem";
    _problem = _factory.create<FEProblem>("FEProblem", "problem", problem_params);
    _problem->createQRules(libMesh::QGAUSS, libMesh::FIRST, libMesh::FIRST, libMesh::FIRST);
    _app->actionWarehouse().problemBase() = _problem;

    for (const auto & name : {"u", "v"})
    {
      InputParameters params = _factory.getValidParams("MooseVariable");
      _problem->addVariable("MooseVariable", name, params);
    }
  }

protected:
  /**
   * Set the coupling matrix of the nonlinear system, build the AD kernel, and run its initial
   * setup
   * @param full Whether the coupling matrix couples u to v
   */
  std::shared_ptr<KokkosADSlotTestKernel> setup(const bool full)
  {
    auto cm = std::make_unique<libMesh::CouplingMatrix>(2);
    (*cm)(0, 0) = 1;
    (*cm)(1, 1) = 1;
    if (full)
    {
      (*cm)(0, 1) = 1;
      (*cm)(1, 0) = 1;
    }
    _problem->setCouplingMatrix(std::move(cm), 0);

    InputParameters params = _factory.getValidParams("KokkosADSlotTestKernel");
    params.set<NonlinearVariableName>("variable") = "u";
    params.set<std::vector<VariableName>>("v") = {"v"};
    _problem->addKokkosKernel("KokkosADSlotTestKernel", "kernel", params);

    for (const auto system : make_range(_problem->numNonlinearSystems()))
      _problem->getNonlinearSystemBase(system).system().init();
    _problem->getAuxiliarySystem().system().init();
    _problem->initKokkos();

    auto & warehouse = _problem->getNonlinearSystemBase(0).getKokkosKernelWarehouse();
    auto kernel = std::dynamic_pointer_cast<KokkosADSlotTestKernel>(warehouse.getObjects().front());
    kernel->initialSetup();
    return kernel;
  }

  std::unique_ptr<MooseMesh> _mesh;
  std::shared_ptr<MooseApp> _app;
  Factory & _factory;
  std::shared_ptr<FEProblem> _problem;
};

TEST_F(KokkosADSlotTest, FullCouplingSeedsCoupledVariable)
{
  if (!_problem->mesh().getKokkosMesh())
    GTEST_SKIP() << "Kokkos mesh not available";

  auto kernel = setup(true);
  const auto & blocks = kernel->table().hostBlocks();

  EXPECT_TRUE(kernel->finalized());
  EXPECT_EQ(kernel->table().numSlots(), 8u);
  ASSERT_EQ(blocks.size(), 4u);

  // Blocks follow registration order: u, grad u, v, grad v
  EXPECT_EQ(kernel->uSlot(), 0u);
  EXPECT_EQ(kernel->gradUSlot(), 1u);
  EXPECT_EQ(kernel->vSlot(), 4u);
  EXPECT_EQ(kernel->gradVSlot(), 5u);
  EXPECT_EQ(blocks[2].var, _problem->getVariable(0, "v").number());
  EXPECT_FALSE(blocks[2].gradient);
  EXPECT_TRUE(blocks[3].gradient);
}

TEST_F(KokkosADSlotTest, DiagonalCouplingLeavesCoupledVariableUnseeded)
{
  if (!_problem->mesh().getKokkosMesh())
    GTEST_SKIP() << "Kokkos mesh not available";

  auto kernel = setup(false);
  const auto & blocks = kernel->table().hostBlocks();

  // The coupled variable is evaluated without derivatives, so no Jacobian block of v is assembled
  EXPECT_TRUE(kernel->finalized());
  EXPECT_EQ(kernel->table().numSlots(), 4u);
  ASSERT_EQ(blocks.size(), 2u);
  for (const auto & block : blocks)
    EXPECT_EQ(block.var, _problem->getVariable(0, "u").number());

  EXPECT_EQ(kernel->uSlot(), 0u);
  EXPECT_EQ(kernel->gradUSlot(), 1u);
  EXPECT_EQ(kernel->vSlot(), libMesh::invalid_uint);
  EXPECT_EQ(kernel->gradVSlot(), libMesh::invalid_uint);
}
