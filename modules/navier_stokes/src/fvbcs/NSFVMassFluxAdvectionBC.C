//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVMassFluxAdvectionBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSFVMassFluxAdvectionBC);

InputParameters
NSFVMassFluxAdvectionBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addClassDescription(
      "Computes the residual for a quantity being advected by a moving mass of fluid.");
  params.addRequiredParam<MaterialPropertyName>("advected_quantity",
                                                "the property being advected by the mass flux");
  MooseEnum flux_interp_method("average upwind", "upwind");

  params.addParam<MooseEnum>("flux_interp_method",
                             flux_interp_method,
                             "The interpolation to use for the advected quantity. Options are "
                             "'upwind' and 'average', with the default being 'upwind'.");
  return params;
}

NSFVMassFluxAdvectionBC::NSFVMassFluxAdvectionBC(const InputParameters & params)
  : FVFluxBC(params),
    _mass_flux_elem(getADMaterialProperty<RealVectorValue>(NS::mass_flux)),
    _mass_flux_neighbor(getNeighborADMaterialProperty<RealVectorValue>(NS::mass_flux)),
    _adv_elem(getADMaterialProperty<Real>("advected_quantity")),
    _adv_neighbor(getNeighborADMaterialProperty<Real>("advected_quantity"))
{
  using namespace Moose::FV;

  const auto & flux_interp_method = getParam<MooseEnum>("flux_interp_method");
  if (flux_interp_method == "average")
    _flux_interp_method = InterpMethod::Average;
  else if (flux_interp_method == "upwind")
    _flux_interp_method = InterpMethod::Upwind;
  else
    mooseError("Unrecognized interpolation type ", static_cast<std::string>(flux_interp_method));
}

ADReal
NSFVMassFluxAdvectionBC::computeQpResidual()
{
  using namespace Moose::FV;

  ADRealVectorValue flux;
  interpolate(_flux_interp_method,
              flux,
              _adv_elem[_qp],
              _adv_neighbor[_qp],
              _mass_flux_elem[_qp],
              _mass_flux_neighbor[_qp],
              *_face_info);

  return _normal * flux;
}
