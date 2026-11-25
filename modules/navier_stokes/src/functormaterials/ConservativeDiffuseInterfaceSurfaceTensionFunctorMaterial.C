//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial.h"

#include "NSFVLeastSquares.h"
#include "MooseMesh.h"
#include "MooseFunctorArguments.h"
#include "SubProblem.h"
#include "MathFVUtils.h"

registerMooseObject("NavierStokesApp", ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial);

InputParameters
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_ALWAYS};

  params.addClassDescription(
      "Create the continuum surface force functors of a diffuse interface: the interface "
      "curvature and the surface tension force density sigma * kappa * grad(alpha).");

  params.addRequiredParam<MooseFunctorName>(
      "volume_fraction_functor", "The volume fraction whose interface carries the surface force.");
  params.addRequiredParam<MooseFunctorName>("surface_tension_coefficient",
                                            "The surface tension coefficient.");
  params.addParam<Real>("normal_regularization",
                        1e-14,
                        "Added to |grad(alpha)| when forming the interface unit normal, so that a "
                        "vanishing gradient gives a finite normal rather than a division by zero.");
  params.addParam<MooseFunctorName>("interface_curvature_name",
                                    "interface_curvature",
                                    "Output name for the interface curvature functor.");
  params.addParam<MooseFunctorName>("surface_tension_force_density_name",
                                    "surface_tension_force_density",
                                    "Output name for the surface tension force density functor.");
  return params;
}

ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::
    ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _volume_fraction(getFunctor<Real>("volume_fraction_functor")),
    _surface_tension_coefficient(getFunctor<Real>("surface_tension_coefficient")),
    _normal_regularization(getParam<Real>("normal_regularization")),
    _moose_mesh(_subproblem.mesh()),
    _dim(_subproblem.mesh().dimension())
{
  if (_normal_regularization <= 0.0)
    paramError("normal_regularization", "The regularization must be positive.");

  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());

  addFunctorProperty<Real>(
      getParam<MooseFunctorName>("interface_curvature_name"),
      [this](const auto & r, const auto & t) -> Real { return curvature(r, t); },
      clearance_schedule);

  addFunctorProperty<RealVectorValue>(
      getParam<MooseFunctorName>("surface_tension_force_density_name"),
      [this](const auto & r, const auto & t) -> RealVectorValue { return forceDensity(r, t); },
      clearance_schedule);
}

bool
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::isInternalFace(const FaceInfo & fi) const
{
  return _volume_fraction.hasFaceSide(fi, true) && _volume_fraction.hasFaceSide(fi, false);
}

const ElemInfo &
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::boundaryElemInfo(
    const FaceInfo & fi) const
{
  // On a face the volume fraction only lives on one side of, that side owns the cell quantity.
  return _volume_fraction.hasBlocks(fi.elemSubdomainID()) ? *fi.elemInfo() : *fi.neighborInfo();
}

Real
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::cellCurvature(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  return cellCurvature(
      elem_info,
      NS::leastSquaresCellGradient(_volume_fraction, elem_info, _moose_mesh, _dim, time_arg),
      time_arg);
}

Real
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::cellCurvature(
    const ElemInfo & elem_info,
    const RealVectorValue & elem_gradient,
    const Moose::StateArg & time_arg) const
{
  const Elem * const elem = elem_info.elem();
  if (!elem)
    return 0.0;

  const Real cell_volume = NS::cellMeasure(elem_info);
  if (cell_volume <= 0.0)
    return 0.0;

  // kappa = -div(n), with div(n) assembled as the flux of the unit normal through the cell's
  // faces divided by the cell volume. The face normal is the face value of grad(alpha), averaged
  // from the two cells, normalized on the face: normalizing cell gradients and averaging the unit
  // vectors would shorten the face normal wherever the two cells disagree in direction.
  Real interface_normal_flux = 0.0;
  for (const auto side : make_range(elem->n_sides()))
  {
    const FaceInfo * const fi_loc = NS::faceInfoForElementSide(_moose_mesh, elem, side);
    if (!fi_loc)
      continue;

    RealVectorValue face_gradient = elem_gradient;
    if (isInternalFace(*fi_loc))
    {
      const ElemInfo * const neighbor_info =
          fi_loc->elemPtr() == elem ? fi_loc->neighborInfo() : fi_loc->elemInfo();
      const RealVectorValue neighbor_gradient = NS::leastSquaresCellGradient(
          _volume_fraction, *neighbor_info, _moose_mesh, _dim, time_arg);
      Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                             face_gradient,
                             elem_gradient,
                             neighbor_gradient,
                             *fi_loc,
                             fi_loc->elemPtr() == elem);
    }

    const RealVectorValue interface_normal =
        face_gradient / (face_gradient.norm() + _normal_regularization);
    const Real orientation = fi_loc->elemPtr() == elem ? 1.0 : -1.0;
    interface_normal_flux +=
        NS::faceMeasure(*fi_loc) * (interface_normal * (orientation * fi_loc->normal()));
  }

  return -interface_normal_flux / cell_volume;
}

RealVectorValue
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::cellForceDensity(
    const ElemInfo & elem_info, const Moose::StateArg & time_arg) const
{
  const Elem * const elem = elem_info.elem();
  if (!elem)
    return RealVectorValue();

  const Real sigma = _surface_tension_coefficient(Moose::ElemArg{elem, false}, time_arg);
  if (sigma == 0.0)
    return RealVectorValue();

  const RealVectorValue alpha_gradient =
      NS::leastSquaresCellGradient(_volume_fraction, elem_info, _moose_mesh, _dim, time_arg);

  return sigma * cellCurvature(elem_info, alpha_gradient, time_arg) * alpha_gradient;
}

template <typename T, typename CellValue>
T
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::faceValue(
    const Moose::FaceArg & r, const CellValue & cell_value) const
{
  const FaceInfo * const fi = r.fi;
  if (!fi)
    return T();

  if (!isInternalFace(*fi))
    return cell_value(boundaryElemInfo(*fi));

  T face_value = T();
  Moose::FV::interpolate(Moose::FV::InterpMethod::Average,
                         face_value,
                         cell_value(*fi->elemInfo()),
                         cell_value(*fi->neighborInfo()),
                         *fi,
                         true);
  return face_value;
}

Real
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::curvature(
    const Moose::FaceArg & r, const Moose::StateArg & t) const
{
  return faceValue<Real>(r, [this, &t](const ElemInfo & ei) { return cellCurvature(ei, t); });
}

RealVectorValue
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::forceDensity(
    const Moose::FaceArg & r, const Moose::StateArg & t) const
{
  return faceValue<RealVectorValue>(
      r, [this, &t](const ElemInfo & ei) { return cellForceDensity(ei, t); });
}

Real
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::curvature(const Moose::NodeArg &,
                                                                     const Moose::StateArg &) const
{
  mooseError("The interface curvature is a cell quantity and has no nodal value.");
}

RealVectorValue
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::forceDensity(
    const Moose::NodeArg &, const Moose::StateArg &) const
{
  mooseError("The surface tension force density is a cell quantity and has no nodal value.");
}

Real
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::curvature(const Moose::PointArg &,
                                                                     const Moose::StateArg &) const
{
  mooseError("The interface curvature is a cell quantity and has no value at a bare point.");
}

RealVectorValue
ConservativeDiffuseInterfaceSurfaceTensionFunctorMaterial::forceDensity(
    const Moose::PointArg &, const Moose::StateArg &) const
{
  mooseError(
      "The surface tension force density is a cell quantity and has no value at a bare point.");
}
