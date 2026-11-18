# Time Dependent Anisotropic Diffusion Gold

To recreate the test data, apply the below patch to MFEM revision
6a858dc6136fe2194e77909a7d89cab757d0d5d5, compile `examples/ex29p.cpp`,
and run it with no arguments.

<details><summary>MFEM ex29p patch</summary>

```patch
diff --git a/examples/ex29p.cpp b/examples/ex29p.cpp
index 2b2251babf..eb9762cb7c 100644
--- a/examples/ex29p.cpp
+++ b/examples/ex29p.cpp
@@ -32,35 +33,13 @@ Mesh * GetMesh(int type);

 void trans(const Vector &x, Vector &r);

-void sigmaFunc(const Vector &x, DenseMatrix &s);
+void sigmaFunc(const Vector &x, real_t time, DenseMatrix &s);

 real_t uExact(const Vector &x)
 {
    return (0.25 * (2.0 + x[0]) - x[2]) * (x[2] + 0.25 * (2.0 + x[0]));
 }

-void duExact(const Vector &x, Vector &du)
-{
-   du.SetSize(3);
-   du[0] = 0.125 * (2.0 + x[0]) * x[1] * x[1];
-   du[1] = -0.125 * (2.0 + x[0]) * x[0] * x[1];
-   du[2] = -2.0 * x[2];
-}
-
-void fluxExact(const Vector &x, Vector &f)
-{
-   f.SetSize(3);
-
-   DenseMatrix s(3);
-   sigmaFunc(x, s);
-
-   Vector du(3);
-   duExact(x, du);
-
-   s.Mult(du, f);
-   f *= -1.0;
-}
-
 int main(int argc, char *argv[])
 {
    // 1. Initialize MPI and HYPRE.
@@ -73,8 +52,10 @@ int main(int argc, char *argv[])
    int order = 3;
    int mesh_type = 4; // Default to Quadrilateral mesh
    int mesh_order = 3;
-   int ser_ref_levels = 2;
-   int par_ref_levels = 1;
+   int ser_ref_levels = 1;
+   int par_ref_levels = 0;
+   real_t t_final = 1.0;
+   real_t dt = 0.05;
    bool static_cond = false;
    bool visualization = true;

@@ -90,12 +71,16 @@ int main(int argc, char *argv[])
    args.AddOption(&order, "-o", "--order",
                   "Finite element order (polynomial degree) or -1 for"
                   " isoparametric space.");
+   args.AddOption(&t_final, "-tf", "--t-final", "Final simulation time.");
+   args.AddOption(&dt, "-dt", "--time-step", "Backward Euler time step.");
    args.AddOption(&static_cond, "-sc", "--static-condensation", "-no-sc",
                   "--no-static-condensation", "Enable static condensation.");
    args.AddOption(&visualization, "-vis", "--visualization", "-no-vis",
                   "--no-visualization",
                   "Enable or disable GLVis visualization.");
    args.ParseCheck();
+   MFEM_VERIFY(t_final > 0.0 && dt > 0.0,
+               "Final time and time step must be positive.");

    // 3. Construct a quadrilateral or triangular mesh with the topology of a
    //    cylindrical surface.
@@ -137,10 +122,10 @@ int main(int argc, char *argv[])
    //    In this example, the boundary conditions are defined by marking all
    //    the boundary attributes from the mesh as essential (Dirichlet) and
    //    converting them to a list of true dofs.
-   Array<int> ess_tdof_list;
+   Array<int> ess_tdof_list, ess_bdr;
    if (pmesh.bdr_attributes.Size())
    {
-      Array<int> ess_bdr(pmesh.bdr_attributes.Max());
+      ess_bdr.SetSize(pmesh.bdr_attributes.Max());
       ess_bdr = 1;
       fespace.GetEssentialTrueDofs(ess_bdr, ess_tdof_list);
    }
@@ -153,66 +138,98 @@ int main(int argc, char *argv[])
    b.AddDomainIntegrator(new DomainLFIntegrator(one));
    b.Assemble();

-   // 10. Define the solution vector x as a finite element grid function
-   //     corresponding to fespace. Initialize x with initial guess of zero,
-   //     which satisfies the boundary conditions.
+   // 10. Initialize the solution with the steady state for sigma(x,0),
+   //     enforcing homogeneous Dirichlet conditions on the curved boundary.
    ParGridFunction x(&fespace);
-   x = 0.0;
-
-   // 11. Set up the bilinear form a(.,.) on the finite element space
-   //     corresponding to the Laplacian operator -Delta, by adding the
-   //     Diffusion domain integrator.
-   ParBilinearForm a(&fespace);
-   MatrixFunctionCoefficient sigma(3, sigmaFunc);
-   BilinearFormIntegrator *integ = new DiffusionIntegrator(sigma);
-   a.AddDomainIntegrator(integ);
-
-   // 12. Assemble the bilinear form and the corresponding linear system,
-   //     applying any necessary transformations such as: eliminating boundary
-   //     conditions, applying conforming constraints for non-conforming AMR,
-   //     static condensation, etc.
-   if (static_cond) { a.EnableStaticCondensation(); }
-   a.Assemble();
-
-   OperatorPtr A;
-   Vector B, X;
-   a.FormLinearSystem(ess_tdof_list, x, b, A, X, B);
-
-   if (myid == 0)
-   {
-      cout << "Size of linear system: "
-           << A.As<HypreParMatrix>()->GetGlobalNumRows() << endl;
-   }
-
-   // 13. Define and apply a parallel PCG solver for A X = B with the BoomerAMG
-   //     preconditioner from hypre.
-   HypreBoomerAMG *amg = new HypreBoomerAMG;
-   CGSolver cg(MPI_COMM_WORLD);
-   cg.SetRelTol(1e-12);
-   cg.SetMaxIter(2000);
-   cg.SetPrintLevel(1);
-   cg.SetPreconditioner(*amg);
-   cg.SetOperator(*A);
-   cg.Mult(B, X);
-   delete amg;
-
-   // 14. Recover the solution as a finite element grid function.
-   a.RecoverFEMSolution(X, b, x);
-
-   // 15. Compute error in the solution and its flux
    FunctionCoefficient uCoef(uExact);
-   real_t error = x.ComputeL2Error(uCoef);
+   ConstantCoefficient zero(0.0);
+   x.ProjectCoefficient(uCoef);
+   x.ProjectBdrCoefficient(zero, ess_bdr);

-   if (myid == 0) { cout << "|u - u_h|_2 = " << error << endl; }
+   // 11. Assemble the mass matrix once. Each backward Euler step solves
+   //     (M/dt + K(t)) u_new = b + M u_old/dt.
+   ParBilinearForm mass(&fespace);
+   mass.AddDomainIntegrator(new MassIntegrator());
+   mass.Assemble();
+   mass.Finalize();

+   MatrixFunctionCoefficient sigma(3, sigmaFunc);
+   sigma.SetTime(0.0);
+   DiffusionIntegrator flux_integ(sigma);
    ParFiniteElementSpace flux_fespace(&pmesh, &fec, 3);
    ParGridFunction flux(&flux_fespace);
-   x.ComputeFlux(*integ, flux); flux *= -1.0;
+   x.ComputeFlux(flux_integ, flux);
+   flux *= -1.0;

-   VectorFunctionCoefficient fluxCoef(3, fluxExact);
-   real_t flux_err = flux.ComputeL2Error(fluxCoef);
+   // 12. Record the initial state and every timestep in one ParaView collection.
+   ParaViewDataCollection pd("Run0", &pmesh);
+   pd.SetPrefixPath("ParaView");
+   pd.RegisterField("u", &x);
+   pd.SetDataFormat(VTKFormat::ASCII);
+   pd.SetHighOrderOutput(true);
+   pd.SetPrecision(9);
+   pd.SetLevelsOfDetail(1);
+   pd.SetTime(0.0);
+   pd.SetCycle(0);
+   pd.Save();

-   if (myid == 0) { cout << "|f - f_h|_2 = " << flux_err << endl; }
+   // 13. Reassemble at the new time so that the matrix coefficient is evaluated
+   //     at every step. Shorten the last step to end exactly at t_final.
+   real_t time = 0.0;
+   int step = 0;
+   while (time < t_final)
+   {
+      const real_t next_time = min((step + 1) * dt, t_final);
+      const real_t dt_step = next_time - time;
+      ConstantCoefficient inv_dt(1.0 / dt_step);
+      sigma.SetTime(next_time);
+
+      ParLinearForm rhs(&fespace);
+      mass.Mult(x, rhs);
+      rhs *= 1.0 / dt_step;
+      rhs += b;
+
+      ParBilinearForm a(&fespace);
+      a.AddDomainIntegrator(new MassIntegrator(inv_dt));
+      a.AddDomainIntegrator(new DiffusionIntegrator(sigma));
+      // 14. Assemble the bilinear form and the corresponding linear system,
+      //     applying any necessary transformations such as: eliminating boundary
+      //     conditions, applying conforming constraints for non-conforming AMR,
+      //     static condensation, etc.
+      if (static_cond) { a.EnableStaticCondensation(); }
+      a.Assemble();
+      OperatorPtr A;
+      Vector B, X;
+      a.FormLinearSystem(ess_tdof_list, x, rhs, A, X, B);
+
+      // 15. Solve with parallel PCG and BoomerAMG, then recover the solution
+      //     and its flux at the current coefficient time.
+      HypreBoomerAMG amg;
+      amg.SetPrintLevel(0);
+      CGSolver cg(MPI_COMM_WORLD);
+      // Keep algebraic error well below spatial and temporal discretization error.
+      cg.SetRelTol(1e-12);
+      cg.SetMaxIter(2000);
+      cg.SetPrintLevel(0);
+      cg.SetPreconditioner(amg);
+      cg.SetOperator(*A);
+      cg.Mult(B, X);
+      MFEM_VERIFY(cg.GetConverged(), "Backward Euler solve did not converge.");
+      a.RecoverFEMSolution(X, rhs, x);
+      x.ComputeFlux(flux_integ, flux);
+      flux *= -1.0;
+
+      time = next_time;
+      step++;
+      pd.SetTime(time);
+      pd.SetCycle(step);
+      pd.Save();
+      if (myid == 0)
+      {
+         cout << "Step " << step << ", time = " << time
+              << ", diffusion scale = " << exp(-5.0 * time) << endl;
+      }
+   }

    // 16. Save the refined mesh and the solution. This output can be viewed
    //     later using GLVis: "glvis -np <np> -m mesh -g sol".
@@ -255,6 +272,8 @@ int main(int argc, char *argv[])
                 << "window_title 'Flux'\n"  << flush;
    }

+   pmesh.Save("ex29p.mesh"); // mesh used as input to test
+
    return 0;
 }

@@ -378,8 +397,9 @@ void trans(const Vector &x, Vector &r)
    r[2] = 0.25 * (2.0 * x[2] - 1.0) * (r[0] + 2.0);
 }

-// Anisotropic diffusion coefficient
-void sigmaFunc(const Vector &x, DenseMatrix &s)
+// Scale the whole tensor to preserve symmetry and positive definiteness.
+// The decay rate 5 gives exp(-5) diffusivity at the default final time t = 1.
+void sigmaFunc(const Vector &x, real_t time, DenseMatrix &s)
 {
    s.SetSize(3);
    real_t a = 17.0 - 2.0 * x[0] * (1.0 + x[0]);
@@ -392,4 +412,5 @@ void sigmaFunc(const Vector &x, DenseMatrix &s)
    s(2,0) = 0.0;
    s(2,1) = 0.0;
    s(2,2) = a / 32.0;
+   s *= exp(-5.0 * time);
 }
```

</details>
