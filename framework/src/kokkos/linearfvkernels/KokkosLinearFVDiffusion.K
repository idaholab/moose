//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosLinearFVDiffusion.h"
#include "KokkosLinearFVAdvectionDiffusionFunctorDirichletBC.h"
#include "Attributes.h"
#include "FEProblemBase.h"

#include <set>
#include <vector>

registerKokkosLinearFVKernel("MooseApp", KokkosLinearFVDiffusion);

InputParameters
KokkosLinearFVDiffusion::validParams()
{
  InputParameters params = Moose::Kokkos::LinearFVFluxKernel::validParams();
  params.addRequiredParam<FunctionName>("diffusion_coeff", "The diffusion coefficient.");
  params.addClassDescription("Kokkos LinearFV diffusion kernel.");
  return params;
}

KokkosLinearFVDiffusion::KokkosLinearFVDiffusion(const InputParameters & parameters)
  : Moose::Kokkos::LinearFVFluxKernel(parameters),
    _diffusion_coeff(getKokkosFunction("diffusion_coeff"))
{
}

void
KokkosLinearFVDiffusion::initialSetup()
{
  std::vector<Moose::Kokkos::LinearFVBoundaryCondition *> bcs;
  auto query = _fe_problem.theWarehouse()
                   .query()
                   .template condition<AttribSystem>("LinearFVBoundaryCondition")
                   .template condition<AttribKokkos>(true)
                   .template condition<AttribThread>(_tid)
                   .template condition<AttribSysNum>(_sys.number());
  query.queryInto(bcs);

  std::vector<BoundaryID> boundary_ids;
  std::vector<Moose::Kokkos::Function> values;
  std::set<BoundaryID> seen;

  for (auto * bc : bcs)
  {
    if (bc->variable().number() != _var_num)
      continue;

    auto * dirichlet = dynamic_cast<KokkosLinearFVAdvectionDiffusionFunctorDirichletBC *>(bc);
    if (!dirichlet)
      mooseError("KokkosLinearFVDiffusion currently only supports ",
                 "KokkosLinearFVAdvectionDiffusionFunctorDirichletBC boundary conditions.");

    for (const auto boundary_id : dirichlet->boundaryIDs())
    {
      if (!seen.insert(boundary_id).second)
        mooseError("Duplicate Kokkos LinearFV boundary condition on boundary ", boundary_id, ".");

      boundary_ids.push_back(boundary_id);
      values.push_back(dirichlet->functor());
    }
  }

  _dirichlet_boundary_ids.create(boundary_ids.size());
  // Kokkos functions do not have default constructors, so we allocate here and do placement new
  // later
  _dirichlet_values.create(values.size());

  for (const auto i : index_range(boundary_ids))
  {
    _dirichlet_boundary_ids[i] = boundary_ids[i];
    new (&_dirichlet_values[i]) Moose::Kokkos::Function(values[i]);
  }

  _dirichlet_boundary_ids.moveToDevice();
  _dirichlet_values.moveToDevice();
}
