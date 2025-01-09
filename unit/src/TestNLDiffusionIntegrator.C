#include "gtest/gtest.h"
#include "mfem.hpp"

using namespace mfem;

double f(double u) { return u; }
double df(double u) { return 1.0; }

// Define a coefficient that, given a grid function u, function func, returns func(u)
class NonlinearGridFunctionCoefficient : public Coefficient
{
   GridFunction &gf;
   std::function<double(double)> func;
public:
   NonlinearGridFunctionCoefficient(GridFunction &gf_, std::function<double(double)> func_) : gf(gf_), func(func_) { }
   double Eval(ElementTransformation &T, const IntegrationPoint &ip)
   {
      return func(gf.GetValue(T, ip));
   }
};


// Define a nonlinear integrator that computes (f(u), v) and its linearized
// operator, (u df(u), v).
//
// Note that the action (f(u), v) can be computed using DomainLFIntegrator
// and the Jacobian matrix linearized operator can be computed using
// MassIntegrator with the appropriate coefficients.
class NonlinearMassIntegrator : public NonlinearFormIntegrator
{
   FiniteElementSpace &fes;
   GridFunction gf;
   Array<int> dofs;

public:
   NonlinearMassIntegrator(FiniteElementSpace &fes_) : fes(fes_), gf(&fes) { }

   virtual void AssembleElementVector(const FiniteElement &el,
                                      ElementTransformation &Tr,
                                      const Vector &elfun, Vector &elvect)
   {
      fes.GetElementDofs(Tr.ElementNo, dofs);
      gf.SetSubVector(dofs, elfun);
      NonlinearGridFunctionCoefficient coeff(gf, f);
      DomainLFIntegrator integ(coeff);
      integ.AssembleRHSElementVect(el, Tr, elvect);
   }

   virtual void AssembleElementGrad(const FiniteElement &el,
                                    ElementTransformation &Tr,
                                    const Vector &elfun, DenseMatrix &elmat)
   {
      fes.GetElementDofs(Tr.ElementNo, dofs);
      gf.SetSubVector(dofs, elfun);
      NonlinearGridFunctionCoefficient coeff(gf, df);
      MassIntegrator integ(coeff);
      integ.AssembleElementMatrix(el, Tr, elmat);
   }
};


TEST(CheckData, NLDiffusionTest)
{

   // 1. Parse command line options
   const char *mesh_file = "./data/star.mesh";
 
   int order = 1;
   bool nonzero_rhs = true;

   // 2. Read the mesh from the given mesh file, and refine once uniformly.
   Mesh mesh(mesh_file);
   mesh.UniformRefinement();
   ParMesh pmesh(MPI_COMM_WORLD, mesh);

   // 3. Define a finite element space on the mesh. Here we use H1 continuous
   //    high-order Lagrange finite elements of the given order.
   H1_FECollection fec(order, mesh.Dimension());
   ParFiniteElementSpace fespace(&pmesh, &fec);

   // 4. Extract the list of all the boundaries. These will be marked as
   //    Dirichlet in order to enforce zero boundary conditions.
   Array<int> ess_bdr(pmesh.bdr_attributes.Max());
   ess_bdr = 1;
   Array<int> ess_tdof_list;
   fespace.GetEssentialTrueDofs(ess_bdr, ess_tdof_list);

   // 5. Define the solution x as a finite element grid function in fespace. Set
   //    the initial guess to zero, which also sets the boundary conditions.
   ParGridFunction u1(&fespace), u2(&fespace);
   u1 = 0.0;
   u2 = 0.0;

   // Solve as a non-linear problem.

   // 6. Set up the nonlinear form n(u,v) = (grad u, grad v) + (f(u), v)
   ParNonlinearForm n(&fespace);
   n.AddDomainIntegrator(new NonlinearMassIntegrator(fespace));
   n.AddDomainIntegrator(new DiffusionIntegrator);

   // 7. Set up the the right-hand side. For simplicitly, we just use a zero
   //    vector. Because of the form of the nonlinear function f, it is still
   //    nontrivial to solve n(u,v) = 0.
   ParLinearForm b(&fespace);
   b = 0.0;
   if (nonzero_rhs) {
      ConstantCoefficient five(5.0);
      b.AddDomainIntegrator(new DomainLFIntegrator(five));
      b.Assemble();
   }

   // 8. Get true dof vectors and set essential BCs on rhs.
   Vector X(fespace.GetTrueVSize()), B(fespace.GetTrueVSize());
   u1.GetTrueDofs(X);
   b.ParallelAssemble(B);
   n.SetEssentialBC(ess_bdr, &B);

   // 9. Set up the Newton solver. Each Newton iteration requires a linear
   //    solve. Here we use UMFPack as a direct solver for these systems.
   CGSolver solver(MPI_COMM_WORLD);
   NewtonSolver newton(MPI_COMM_WORLD);
   newton.SetOperator(n);
   newton.SetSolver(solver);
   newton.SetPrintLevel(1);
   newton.SetRelTol(1e-10);
   newton.SetMaxIter(20);

   // 10. Solve the nonlinear system.
   newton.Mult(B, X);
   u1.Distribute(X);


   // Solve as a linear problem.
   {
      BilinearForm a(&fespace);
      a.AddDomainIntegrator(new DiffusionIntegrator);
      a.AddDomainIntegrator(new MassIntegrator);
      a.Assemble();

      OperatorPtr A;
      Vector C, Y;
      a.FormLinearSystem(ess_tdof_list, u2, b, A, Y, C);
      GSSmoother M((SparseMatrix&)(*A));
      PCG(*A, M, C, Y, 1, 500, 1e-12, 0.0);
      a.RecoverFEMSolution(Y, b, u2);
   }

   u1 -= u2;

   EXPECT_NEAR(u1.Norml2(), 0, 1e-5);
}
