# Sublattice KKS model

The sublattice Kim-Kim-Suzuki (SLKKS) [!cite](Schwen2021) model is an extension
of the original KKS model incorporating an additional equal chemical potential
constraint among the sublattice concentration in each phase.

As such each component in the system will have corresponding phase
concentrations, which are split up into per-sublattice concentrations.

\begin{equation}
c_i = \sum_j h(\eta_j)\sum_k a_{jk} c_{ijk},
\end{equation}

## Nomenclature

$i$ indexes a component, $j$ a phase, and $k$ a sublattice in the given
phase. $a_{jk}$ is the fraction of $k$ sublattice sites in phase $j$.

## Sublattice equilibrium

All sublattice pairs $k$ and $k'$ in a given phase are assumed to be always in
equilibrium, thus

\begin{equation}
\frac 1{a_{jk}} \frac{\partial F_j}{\partial c_{ijk}} = \frac 1{a_{jk'}} \frac{\partial F_j}{\partial c_{ijk'}}
\end{equation}

This condition is enforced by the [`SLKKSChemicalPotential`](SLKKSChemicalPotential.md) kernel.

```style=background-color:gray
[chempot1a1b]
  type = SLKKSChemicalPotential
  variable = Cijk
  k        = ajk
  cs       = Cijk'
  ks       = ajk'
  F  = Fj
[]
```

With $N$ sublattices in a phase $N-1$ such kernels are required. That leaves one
sublattice concentration variable in a given phase to be covered by a kernel.

# Phase equilibrium

At the same time the original KKS model chemical potential equilibria still hold

\begin{equation}
\frac{\partial F}{\partial c_i} = \frac{\partial F_j}{\partial c_{ijk}} \quad ,\quad \forall \{j,k\}.
\end{equation}

meaning that sublattice chemical potentials (corrected for the sublattice site
fractions) must be in equilibrium across phases.

\begin{equation}
\frac 1{a_{jk}} \frac{\partial F_j}{\partial c_{ijk}} = \frac 1{a_{j'k}} \frac{\partial F_{j'}}{\partial c_{ij'k}}
\end{equation}

That remaining sublattice concentration will couple to the next phase using a
[`KKSPhaseChemicalPotential`](/KKSPhaseChemicalPotential.md) kernel.

```style=background-color:gray
[chempot1c2a]
  type = KKSPhaseChemicalPotential
  variable = Cijk
  ka = ajk
  fa_name = Fj
  cb = Cij'k
  kb = aj'k
  fb_name = Fj'
[]
```

Note that in this kernel the `ajk` and `aj'k` must be true site fractions ranging from 0 to 1.

## Mass transport

The global concentration variables $c_i$ govern the mass transport along
chemical potential gradients.

\begin{equation}
   \frac{\partial c_i}{\partial t} = \nabla\cdot D_i\sum_j h_j\sum_k a_{jk}\nabla c_{ijk}. \label{eq:chdiff}
\end{equation}

This equation can be implemented using multiple `MatDiffusion` kernels in MOOSE.
To illustrate this we can re-order the summation to yield

\begin{equation}
   \frac{\partial c_i}{\partial t} = \sum_j\sum_k \nabla\cdot \underbrace{D_i h_j a_{jk}}_{D'_i}\nabla c_{ijk}. \label{eq:chdiff2}
\end{equation}

This means we need to add a `MatDiffusion` kernel for sublattice in all phases,
each operating on the variable $c_i$. We use the
[!param](/Kernels/MatDiffusion/v) parameter to  specify $c_{ijk}$ as the
variable to take the gradient of (rather than $c_i$). The effective diffusivity
$D'_i$ passed into the kernel is $D\cdot h_j \cdot a_{jk}$, which can be provided
by a [`DerivativeParsedMaterial`](DerivativeParsedMaterial.md)

```style=background-color:gray
[D'i]
  type = DerivativeParsedMaterial
  f_name = D'i
  function = Di*hi*ajk
  material_property_names = 'Di hi'
  constant_names = ajk
  constant_expressions = 1/3
[]
```

Here we assume the site fraction of the current sublattice to be $1/3$.

!alert note title=Extension to phase/sublattice dependent diffusivities
The derivation should be check to see if a phase dependent concentration $D_{ij}$ or even sublattice dependent concentration $D_{ijk}$ is feasible.


## Example

An open TDB file for the Fe-Cr system was graciously provided by Aurélie Jacob
at TU Wien, based on the work in [!cite](jacob2018revised).

The TDB file can be converted into MOOSE [`DerivativeParsedMaterial`](DerivativeParsedMaterial.md)
syntax using the [`free_energy.py` script](/CALPHAD.md).

```
mamba activate pycalphad
cd $MOOSE_DIR/modules/phase_field/examples/slkks
../../../../python/calphad/free_energy.py CrFe_Jacob.tdb BCC_A2 SIGMA
```

The example directory also contains an Jupyter notebook to visualize the phase
diagram for the supplied Fe-Cr thermodynamic database file and the simulation
results,

!listing modules/phase_field/examples/slkks/CrFe.i
