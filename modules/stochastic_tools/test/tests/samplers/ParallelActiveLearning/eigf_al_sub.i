#* Sub-app: evaluates f(x) = sin(x) + 2*x at the x value controlled by the
#* main app (vigf_al_python_comparison.i) via MultiAppSamplerControl.
#*
#* FIX 1: ConstantPostprocessor value changed from 0.0 to 2.5 (matching
#*   the main app's initial_values = '2.5').  The default only matters in
#*   standalone mode -- the main app always overrides it via CLI_ARGS before
#*   the sub-app runs -- but a meaningful default prevents f(0)=0 from
#*   appearing if the sub-app is ever run outside the active-learning loop.
#*
#* FIX 2: average execute_on changed from 'initial timestep_end' to
#*   'timestep_end' only.  For a Steady solver there is one "timestep" so
#*   the initial execute is redundant.  Removing it eliminates the silent
#*   evaluation of f(x_input_default) = f(2.5) at sub-app initialisation,
#*   which could appear as a spurious y value if timing of the reporter
#*   transfer is off.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [x_input]
    type = ConstantPostprocessor
    value = 2.5   # FIX 1: was 0.0 -- changed to match initial_values in main app
  []
  [average]
    type = FunctionValuePostprocessor
    function = true_function
    execute_on = 'timestep_end'   # FIX 2: was 'initial timestep_end'
  []
[]

[Functions]
  [true_function]
    type = ParsedFunction
    expression = 'sin(x_in) + 2*x_in'
    symbol_names = 'x_in'
    symbol_values = 'x_input'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
[]
