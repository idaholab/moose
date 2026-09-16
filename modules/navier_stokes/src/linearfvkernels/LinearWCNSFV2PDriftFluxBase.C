//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearWCNSFV2PDriftFluxBase.h"

InputParameters
LinearWCNSFV2PDriftFluxBase::validParams()
{
  auto params = LinearFVFluxKernel::validParams();
  params.addRequiredParam<MooseFunctorName>("u_slip", "The slip velocity in the x direction.");
  params.addParam<MooseFunctorName>("v_slip", "The slip velocity in the y direction.");
  params.addParam<MooseFunctorName>("w_slip", "The slip velocity in the z direction.");
  params.addRequiredParam<MooseFunctorName>("rho_d", "Dispersed phase density.");
  params.addParam<MooseFunctorName>(
      "fraction_dispersed", 0.0, "Volume fraction of the dispersed phase.");
  params.addParam<std::vector<BoundaryName>>(
      "slip_boundaries",
      {},
      "Boundaries across which the dispersed phase may travel, normally the inlets and the "
      "outlets. The slip contributes on these boundaries and on every internal face, but not on "
      "impermeable boundaries such as walls, where the phase cannot cross even though the "
      "variable may carry a boundary condition.");
  return params;
}

LinearWCNSFV2PDriftFluxBase::LinearWCNSFV2PDriftFluxBase(const InputParameters & params)
  : LinearFVFluxKernel(params),
    _dim(_subproblem.mesh().dimension()),
    _f_d(getFunctor<Real>("fraction_dispersed")),
    _rho_d(getFunctor<Real>("rho_d")),
    _u_slip(getFunctor<Real>("u_slip")),
    _v_slip(isParamValid("v_slip") ? &getFunctor<Real>("v_slip") : nullptr),
    _w_slip(isParamValid("w_slip") ? &getFunctor<Real>("w_slip") : nullptr),
    _boundary_normal_factor(1.0)
{
  NS::checkSlipVelocityComponents(*this, _dim, _v_slip, _w_slip);

  const auto slip_ids =
      _mesh.getBoundaryIDs(getParam<std::vector<BoundaryName>>("slip_boundaries"));
  _slip_boundaries.insert(slip_ids.begin(), slip_ids.end());
}

void
LinearWCNSFV2PDriftFluxBase::setupFaceData(const FaceInfo * face_info)
{
  LinearFVFluxKernel::setupFaceData(face_info);
  _boundary_normal_factor = (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) ? 1.0 : -1.0;
}
