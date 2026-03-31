//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMObjectUnitTest.h"
#include "EquationSystem.h"
#include "MFEMMassKernel.h"
#include "MFEMDomainLFKernel.h"
#include "MFEMMixedBilinearFormKernel.h"
#include "MFEMPicardNonlinearSolver.h"

namespace
{
class ExternalSquaredCouplingIntegrator : public mfem::NonlinearFormIntegrator
{
public:
  ExternalSquaredCouplingIntegrator(mfem::ParGridFunction & trial_gf, mfem::real_t scale)
    : _trial_gf(trial_gf), _trial_coef(&_trial_gf), _scale(scale)
  {
  }

  void AssembleElementVector(const mfem::FiniteElement & el,
                             mfem::ElementTransformation & Tr,
                             const mfem::Vector &,
                             mfem::Vector & elvect) override
  {
    const auto * ir = IntRule;
    if (!ir)
      ir = &mfem::IntRules.Get(el.GetGeomType(), 2 * el.GetOrder() + 2);

    mfem::Vector shape(el.GetDof());
    elvect.SetSize(el.GetDof());
    elvect = 0.0;

    for (int i = 0; i < ir->GetNPoints(); ++i)
    {
      const auto & ip = ir->IntPoint(i);
      Tr.SetIntPoint(&ip);
      el.CalcShape(ip, shape);
      const auto trial_value = _trial_coef.Eval(Tr, ip);
      elvect.Add(ip.weight * Tr.Weight() * _scale * trial_value * trial_value, shape);
    }
  }

private:
  mfem::ParGridFunction & _trial_gf;
  mfem::GridFunctionCoefficient _trial_coef;
  mfem::real_t _scale;
};

class TestOffDiagonalSquaredCouplingKernel : public MFEMMixedBilinearFormKernel
{
public:
  static InputParameters validParams()
  {
    auto params = MFEMMixedBilinearFormKernel::validParams();
    params.addParam<Real>("scale", -1.0, "Scaling applied to the squared trial-variable term.");
    params.addClassDescription(
        "Test-only MFEM kernel adding an off-diagonal nonlinear residual contribution.");
    return params;
  }

  TestOffDiagonalSquaredCouplingKernel(const InputParameters & parameters)
    : MFEMMixedBilinearFormKernel(parameters), _scale(getParam<Real>("scale"))
  {
  }

  mfem::NonlinearFormIntegrator * createNLIntegrator() override
  {
    auto * trial_gf = getMFEMProblem().getProblemData().gridfunctions.Get(_trial_var_name);
    return new ExternalSquaredCouplingIntegrator(*trial_gf, _scale);
  }

private:
  const mfem::real_t _scale;
};
}

registerMooseObject("MooseUnitApp", TestOffDiagonalSquaredCouplingKernel);

class TestEquationSystem : public Moose::MFEM::EquationSystem
{
public:
  void initAndBuild(Moose::MFEM::GridFunctions & gridfunctions,
                    Moose::MFEM::ComplexGridFunctions & cmplx_gridfunctions,
                    mfem::AssemblyLevel assembly_level)
  {
    Init(gridfunctions, cmplx_gridfunctions, assembly_level);
    BuildEquationSystem();
  }
};

class MFEMPicardSolverTest : public MFEMObjectUnitTest
{
public:
  MFEMPicardSolverTest() : MFEMObjectUnitTest("MooseUnitApp")
  {
    auto pm = _mfem_mesh_ptr->getMFEMParMeshPtr().get();
    mfem::common::H1_FESpace fe(pm, 1);
    mfem::GridFunction gf(&fe);
    _mfem_problem->getProblemData().gridfunctions.Register(
        "u", std::make_shared<mfem::ParGridFunction>(pm, &gf));
    _mfem_problem->getProblemData().gridfunctions.Register(
        "v", std::make_shared<mfem::ParGridFunction>(pm, &gf));
  }

protected:
  template <typename T>
  std::shared_ptr<T> addSharedObject(const std::string & type,
                                     const std::string & name,
                                     InputParameters & params)
  {
    auto objects = _mfem_problem->addObject<T>(type, name, params);
    mooseAssert(objects.size() == 1, "Doesn't work with threading");
    return objects[0];
  }
};

TEST_F(MFEMPicardSolverTest, SolvesOffDiagonalNonlinearCoupling)
{
  InputParameters mass_u_params = _factory.getValidParams("MFEMMassKernel");
  mass_u_params.set<VariableName>("variable") = "u";
  mass_u_params.set<MFEMScalarCoefficientName>("coefficient") = "1.0";
  auto mass_u = addSharedObject<MFEMMassKernel>("MFEMMassKernel", "mass_u", mass_u_params);

  InputParameters mass_v_params = _factory.getValidParams("MFEMMassKernel");
  mass_v_params.set<VariableName>("variable") = "v";
  mass_v_params.set<MFEMScalarCoefficientName>("coefficient") = "1.0";
  auto mass_v = addSharedObject<MFEMMassKernel>("MFEMMassKernel", "mass_v", mass_v_params);

  InputParameters rhs_v_params = _factory.getValidParams("MFEMDomainLFKernel");
  rhs_v_params.set<VariableName>("variable") = "v";
  rhs_v_params.set<MFEMScalarCoefficientName>("coefficient") = "1.0";
  auto rhs_v = addSharedObject<MFEMDomainLFKernel>("MFEMDomainLFKernel", "rhs_v", rhs_v_params);

  InputParameters coupling_params = _factory.getValidParams("TestOffDiagonalSquaredCouplingKernel");
  coupling_params.set<VariableName>("variable") = "u";
  coupling_params.set<VariableName>("trial_variable") = "v";
  coupling_params.set<Real>("scale") = -1.0;
  auto coupling = addSharedObject<TestOffDiagonalSquaredCouplingKernel>(
      "TestOffDiagonalSquaredCouplingKernel", "uv_squared", coupling_params);

  TestEquationSystem eqn_system;
  eqn_system.SetSolverRequiresGradient(false);
  eqn_system.AddKernel(mass_u);
  eqn_system.AddKernel(mass_v);
  eqn_system.AddKernel(rhs_v);
  eqn_system.AddKernel(coupling);
  eqn_system.initAndBuild(_mfem_problem->getProblemData().gridfunctions,
                          _mfem_problem->getProblemData().cmplx_gridfunctions,
                          mfem::AssemblyLevel::LEGACY);

  mfem::Array<int> offsets(3);
  offsets[0] = 0;
  offsets[1] = _mfem_problem->getProblemData().gridfunctions.Get("u")->ParFESpace()->TrueVSize();
  offsets[2] = _mfem_problem->getProblemData().gridfunctions.Get("v")->ParFESpace()->TrueVSize();
  offsets.PartialSum();

  mfem::BlockVector true_x(offsets), true_rhs(offsets);
  true_x = 0.0;
  eqn_system.FormLinearSystem(true_x, true_rhs);

  Moose::MFEM::PicardNonlinearSolver solver(MPI_COMM_WORLD, 5, 1.0e-12, 1.0e-12, 0, 1.0);
  solver.SetOperator(eqn_system);
  solver.Mult(true_rhs, true_x);
  eqn_system.SetTrialVariablesFromTrueVectors(true_x);

  mfem::Vector residual(true_rhs.Size());
  eqn_system.Mult(true_x, residual);
  residual -= true_rhs;
  EXPECT_LT(residual.Norml2(), 1.0e-10);

  for (const auto & var_name : {"u", "v"})
  {
    mfem::ParGridFunction expected(_mfem_problem->getProblemData().gridfunctions.Get(var_name)
                                       ->ParFESpace());
    expected = 1.0;
    mfem::Vector expected_true(expected.ParFESpace()->TrueVSize());
    expected.ParallelProject(expected_true);

    mfem::Vector error(true_x.GetBlock(var_name[0] == 'u' ? 0 : 1));
    error -= expected_true;
    EXPECT_LT(error.Norml2(), 1.0e-10);
  }
}

#endif
