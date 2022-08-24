# -----------------------------------------------------------------------------
# Flow around a cylinder (2D) benchmark validation case
# This example showcases a flow around a cylinder which results in vortex
# shedding. The problem specification has been taken from the following paper:
#
# @incollection{schafer1996benchmark,
#   title={Benchmark computations of laminar flow around a cylinder},
#   author={Sch{\"a}fer, Michael and Turek, Stefan and Durst, Franz and Krause, Egon and Rannacher, Rolf},
#   booktitle={Flow simulation with high-performance computers II},
#   pages={547--566},
#   year={1996},
#   publisher={Springer}
# }
# The Reyndols number is Re=100.
# The expected Strouhal number (St) is in the [0.2950, 0.3050] range, with
# refinement=8, we expect to get St=0.2941 with the model below.
# Run it using the following command:
# ./navier_stokes-opt -i header.i mesh.i flow_over_circle.i executioner_postprocessor.i
# -----------------------------------------------------------------------------

# Geometry parameters
circle_radius = 0.05
pitch = 0.2
x_min = -0.2
x_max = 2.0
y_min = -0.2
y_max = 0.21
rundoff = 1e-4
refinement = 8

# Material properties
mu = 1e-3
rho = 1

# Boundary conditions
inlet_velocity = 1.5

# Numerical schemes
advected_interp_method = 'average'
