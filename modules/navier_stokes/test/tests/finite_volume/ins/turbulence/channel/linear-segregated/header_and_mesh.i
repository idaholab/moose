H = 1 #halfwidth of the channel
L = 100

Re = 13700

rho = 1
bulk_u = 1
mu = '${fparse rho * bulk_u * 2 * H / Re}'

advected_interp_method = 'upwind'

### k-epsilon Closure Parameters ###
sigma_k = 1.0
sigma_eps = 1.3
C1_eps = 1.44
C2_eps = 1.92
C_mu = 0.09

### Initial and Boundary Conditions ###
intensity = ${fparse 0.16*Re^(-1./8.)}
k_init = '${fparse 1.5*(intensity * bulk_u)^2}'
eps_init = '${fparse C_mu^0.75 * k_init^1.5 / (2*H)}'

### Modeling parameters ###
bulk_wall_treatment = false
walls = 'top bottom'
wall_treatment = 'eq_newton'  # Options: eq_newton, eq_incremental, eq_linearized, neq

[Mesh]
  [block_1]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = 0
    ymax = ${H}
    nx = 4
    ny = 4
    bias_y = 0.7
  []
  [block_2]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = ${fparse -H}
    ymax = 0
    nx = 4
    ny = 4
    bias_y = ${fparse 1/0.7}
  []
  [smg]
    type = StitchMeshGenerator
    inputs = 'block_1 block_2'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'bottom top'
    merge_boundaries_with_same_name = true
  []
  # Prevent test diffing on distributed parallel element numbering
  allow_renumbering = false
[]
