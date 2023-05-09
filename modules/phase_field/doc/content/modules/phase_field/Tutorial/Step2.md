# Step 2: Make a Faster Test Model

The input file for this step can be found here: [s2_fasttest.i](https://github.com/idaholab/moose/blob/devel/modules/phase_field/tutorials/spinodal_decomposition/s2_fasttest.i)

## Changes to input file

Before we work on improving the accuracy of the model, it is a good idea to add some features that will speed up the solution. We will add adaptive time stepping, prevent MOOSE from taking unnecessary derivatives, and tell MOOSE to output some information that can help us speed up the simulation even more.

### Derivative Order

Parsed materials allow MOOSE to take the derivatives of the input functions automatically, rather than the user having to input the derivatives by hand. MOOSE will default to taking 3rd order derivatives, but in our problem we only need second order derivatives calculated. We can specify that MOOSE should only take second order derivatives and prevent it from taking unnecessary third derivatives.

```yaml
  [local_energy]
    # Defines the function for the local free energy density as given in the
    # problem, then converts units and adds scaling factor.
    type = DerivativeParsedMaterial
    block = 0
    f_name = f_loc
    args = c
    constant_names = 'A   B   C   D   E   F   G  eV_J  d'
    constant_expressions = '-2.446831e+04 -2.827533e+04 4.167994e+03 7.052907e+03
                            1.208993e+04 2.568625e+03 -2.354293e+03
                            6.24150934e+18 1e-27'
    function = 'eV_J*d*(A*c+B*(1-c)+C*c*log(c)+D*(1-c)*log(1-c)+
                E*c*(1-c)+F*c*(1-c)*(2*c-1)+G*c*(1-c)*(2*c-1)^2)'
    derivative_order = 2
  []
```

### Adaptive Time Stepping

Adaptive time stepping lets the simulation change the time step size depending on how quickly the surface is actually changing. There are several ways to tell MOOSE how to decide how to change the time step. For this simulation we will link it to how many iterations it takes the previous time step to converge. This is done in a sub-block of the executioner block.

```yaml
[Executioner]
  type = Transient
  solve_type = NEWTON
  l_max_its = 30
  l_tol = 1e-6
  nl_max_its = 50
  nl_abs_tol = 1e-9
  end_time = 86400   # 1 day. We only need to run this long enough to verify
                     # the model is working properly.
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  preonly      ilu          1'

  [TimeStepper]
    # Turn on time stepping
    type = IterationAdaptiveDT
    dt = 10
    cutback_factor = 0.8
    growth_factor = 1.5
    optimal_iterations = 7
  []
[]
```

### More Information from MOOSE

If we are going to maximize how the solution converges, we need to know some more information. First we are going to have MOOSE tell us how many times it calculated the residual, then we will have it tell us how long it is spending on the simulation. If we know these pieces of information, we can make small changes to the input files and see how those changes affect our convergence.

To do this, we add [Postprocessors](/syntax/Postprocessors/). Eventually we will add a lot to tell us about the simulation, but here we will start with two.

```yaml
[Postprocessors]
  [evaluations]           # Cumulative residual calculations for simulation
    type = NumResidualEvaluations
  []
  [active_time]           # Time computer spent on simulation
    type = PerformanceData
    event =  ACTIVE
  []
[]
```

Now that we have Postprocessors, we need to output them. We modify the outputs block to output a csv file and a terminal console that will tell us the PostProcessor values.

```yaml
[Outputs]
  exodus = true
  console = true
  csv = true
  print_perf_log = true
  output_initial = true

  [console]
    type = Console
    max_rows = 10
  []
[]
```

There is another option that can improve the speed of convergence. The system converges best if the residuals of all variables are at the same magnitude. MOOSE has an option called scaling, which allows us to multiply the residual of one of the variables by some factor in order to get the residuals to the same magnitude. But before we can do this, we need to know what the residuals are. We add a Debug block to tell us the residuals after every iteration.

```yaml
[Debug]
  show_var_residual_norms = true
[]
```

## Simulation Results

After running this simulation, you should see that the solution is identical to the first simulation, but it took only a small fraction of the time and only a small fraction of the time steps. We can open the csv file that was generated and see information about how long the simulation took. Additionally, we can look at the terminal screen and see that as each time step converges, the two variable residuals have the same magnitude. No scaling is necessary right now.

Now that the simulation is going fast, we can change it to the real initial conditions and see if it will decompose like we expect it to.

## Continue

[step 3: Add Phase Decomposition to the Model](Step3.md)
