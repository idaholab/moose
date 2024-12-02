#include "gtest/gtest.h"
#include "mfem.hpp"

using namespace mfem;


TEST(CheckData, NLDiffusionTest)
{
   Mesh mesh("./data/holes.mesh", 1, 1);

   H1_FECollection fec(1, mesh.Dimension());

   FiniteElementSpace fespace(&mesh, &fec);

   Array<int> nbc_bdr(mesh.bdr_attributes.Max());
   Array<int> rbc_bdr(mesh.bdr_attributes.Max());
   Array<int> dbc_bdr(mesh.bdr_attributes.Max());
   nbc_bdr = 0; nbc_bdr[0] = 1;
   rbc_bdr = 0; rbc_bdr[1] = 1;
   dbc_bdr = 0; dbc_bdr[2] = 1;

   Array<int> ess_tdof_list(0);
   fespace.GetEssentialTrueDofs(dbc_bdr, ess_tdof_list);

   // See defaults in ex27.
   ConstantCoefficient matCoef(1.0);
   ConstantCoefficient dbcCoef(0.0);
   ConstantCoefficient nbcCoef(1.0);
   ConstantCoefficient rbcACoef(1.0);
   ConstantCoefficient rbcBCoef(1.0);
   ProductCoefficient m_nbcCoef(matCoef, nbcCoef);
   ProductCoefficient m_rbcACoef(matCoef, rbcACoef);
   ProductCoefficient m_rbcBCoef(matCoef, rbcBCoef);

   GridFunction u1(&fespace), u2(&fespace);
   u1 = 0.0;
   u2 = 0.0;
   u1.ProjectBdrCoefficient(dbcCoef, dbc_bdr);
   u2.ProjectBdrCoefficient(dbcCoef, dbc_bdr);

   LinearForm b(&fespace);
   b.AddBoundaryIntegrator(new BoundaryLFIntegrator(m_nbcCoef), nbc_bdr);
   b.AddBoundaryIntegrator(new BoundaryLFIntegrator(m_rbcBCoef), rbc_bdr);
   b.Assemble();

   // Solve as a linear problem.
   {
      BilinearForm a(&fespace);
      a.AddDomainIntegrator(new DiffusionIntegrator(matCoef));
      a.AddBoundaryIntegrator(new MassIntegrator(m_rbcACoef), rbc_bdr);
      a.Assemble();

      OperatorPtr A;
      Vector B, X;
      a.FormLinearSystem(ess_tdof_list, u1, b, A, X, B);
      GSSmoother M((SparseMatrix&)(*A));
      PCG(*A, M, B, X, 1, 500, 1e-12, 0.0);
      a.RecoverFEMSolution(X, b, u1);
   }

   // Solve as a nonlinear problem.
   {
      NonlinearForm a_nf(&fespace);
      a_nf.AddDomainIntegrator(new DiffusionIntegrator(matCoef));
      a_nf.AddBoundaryIntegrator(new MassIntegrator(m_rbcACoef), rbc_bdr);
      a_nf.SetEssentialTrueDofs(ess_tdof_list);

      IterativeSolver::PrintLevel print;
      print.Iterations();
      CGSolver cg;
      cg.SetPrintLevel(print);
      cg.SetMaxIter(100);
      cg.SetRelTol(1e-12); cg.SetAbsTol(0.0);

      NewtonSolver newton;
      newton.iterative_mode = false;
      newton.SetSolver(cg);
      newton.SetOperator(a_nf);
      newton.SetPrintLevel(print);
      newton.SetRelTol(1e-14); newton.SetAbsTol(0.0);
      newton.SetMaxIter(1);

      newton.Mult(b, u2);
   }

   u2 -= u1;
   //REQUIRE(u2.Norml2() == MFEM_Approx(0.0, 1e-5));
   EXPECT_NEAR(u2.Norml2(), 0, 1e-5);

  //EXPECT_NO_THROW({ integ_scale.SetIntRule(&ir); });
}
