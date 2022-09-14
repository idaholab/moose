//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PINSFVMomentumDiffusionComplete.h"
#include "INSFVRhieChowInterpolator.h"
#include "NS.h"
#include "SystemBase.h"
#include "RelationshipManager.h"
#include "Factory.h"

registerMooseObject("NavierStokesApp", PINSFVMomentumDiffusionComplete);

InputParameters
PINSFVMomentumDiffusionComplete::validParams()
{
  auto params = PINSFVMomentumDiffusion::validParams();
  params.addClassDescription(
      "Implements the full Laplace form of the viscous porous stress in the Navier-Stokes equation.");
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addParam<bool>("harmonic_interpolation", true, "Whether to use harmonic interpolation for viscosity.");
  params.set<unsigned short>("ghost_layers") = 2;
  return params;
}

PINSFVMomentumDiffusionComplete::PINSFVMomentumDiffusionComplete(const InputParameters & params)
  : PINSFVMomentumDiffusion(params),
  _dim(_subproblem.mesh().dimension()),
  _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
  _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
  _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
  _harmonic_interpolation(getParam<bool>("harmonic_interpolation"))
{
  if ((_var.faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage) &&
      (_tid == 0))
  {
    auto & factory = _app.getFactory();

    auto rm_params = factory.getValidParams("ElementSideNeighborLayers");

    rm_params.set<std::string>("for_whom") = name();
    rm_params.set<MooseMesh *>("mesh") = &const_cast<MooseMesh &>(_mesh);
    rm_params.set<Moose::RelationshipManagerType>("rm_type") =
        Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
        Moose::RelationshipManagerType::COUPLING;
    FVKernel::setRMParams(
        _pars, rm_params, std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));
    mooseAssert(rm_params.areAllRequiredParamsValid(),
                "All relationship manager parameters should be valid.");

    auto rm_obj = factory.create<RelationshipManager>(
        "ElementSideNeighborLayers", name() + "_skew_correction", rm_params);

    // Delete the resources created on behalf of the RM if it ends up not being added to the
    // App.
    if (!_app.addRelationshipManager(rm_obj))
      factory.releaseSharedObjects(*rm_obj);
  }
}

ADReal
PINSFVMomentumDiffusionComplete::computeStrongResidual()
{

  /// ------------------ Useful variables ----------------------
  // Booleans to determine if a face have an associated element and a neighbor
  const bool has_elem = (_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
                         _face_type == FaceInfo::VarFaceNeighbors::BOTH);
  const bool has_neighbor = (_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ||
                             _face_type == FaceInfo::VarFaceNeighbors::BOTH);
  
  // Get the element and neighbour qp from face info
  const auto elem_face = elemFromFace();
  const auto neighbor_face = neighborFromFace();

  // Boolean to determine if the method requires skewness correction from the user selected interpolation method
  const bool skewness_correction = _var.faceInterpolationMethod() == Moose::FV::InterpMethod::SkewCorrectedAverage;

  // Argument to pass a face to a functor
  const auto face = Moose::FV::makeCDFace(*_face_info, faceArgSubdomains(), skewness_correction);

  // Current ace normal
  const auto _normal = _face_info->normal();
  /// ---------------------------------------------------------

  /// ------------------ Coupled variables ---------------------

  // Computing the gradient from coupled variables
  // Normally, we can do this with `_var.gradient(face)` but we will need the transpose gradient
  // So, we compute all at once
  ADRealTensorValue gradient;
  if (_dim == 1)
  {
    //const auto & grad_u = _u_var->adGradSln(face_info, skewness_correction);
    const auto & grad_u = _u_var->gradient(face);
    gradient = ADRealTensorValue(grad_u);
  }
  else if (_dim == 2)
  {
    // const auto & grad_u = _u_var->adGradSln(face_info, skewness_correction);
    // const auto & grad_v = _v_var->adGradSln(face_info, skewness_correction);
    const auto & grad_u = _u_var->gradient(face);
    const auto & grad_v = _v_var->gradient(face);
    gradient = ADRealTensorValue(grad_u, grad_v);
  }
  else // if (_dim == 3)
  {
    // const auto & grad_u = _u_var->adGradSln(face_info, skewness_correction);
    // const auto & grad_v = _v_var->adGradSln(face_info, skewness_correction);
    // const auto & grad_w = _w_var->adGradSln(face_info, skewness_correction);
    const auto & grad_u = _u_var->gradient(face);
    const auto & grad_v = _v_var->gradient(face);
    const auto & grad_w = _w_var->gradient(face);
    gradient = ADRealTensorValue(grad_u, grad_v, grad_w);
  }

  // Getting transpose of the gradient matrix
  auto gradient_transpose = gradient.transpose();

  // Get the full velocity vector
  ADRealVectorValue velocity((*_u_var)(elem_face));
  if (_v_var)
    velocity(1) = (*_v_var)(elem_face);
  if (_w_var)
    velocity(2) = (*_w_var)(elem_face);

  /// ---------------------------------------------------------

  /// ------ Interpolating the diffusion coefficient with different methods ------

  auto mu_elem = _mu(elem_face);
  auto mu_neighbor = _mu(neighbor_face);

  const Real distance_to_neighbor = _face_info->dCN().norm();
  const Real distance_to_element   = _face_info->eCN().norm();
  const Real total_distance        = distance_to_neighbor + distance_to_element;

  // Ideally, we would do the following, but we optimize some operations:
    //   const Real coef_elem = distance_to_neighbor / total_distance;
    //   const Real coef_neighbor = distance_to_element / total_distance;

    //   ADReal mu_face;
    //   if (_harmonic_interpolation)
    //     mu_face = 1.0 / (coef_neighbor / mu_neighbor + coef_elem / mu_elem);
    //   else
    //     mu_face = coef_neighbor * mu_neighbor + coef_elem * mu_elem;

  const Real coef_elem = distance_to_neighbor / total_distance;

  ADReal mu_face;
  if (_harmonic_interpolation)
  {
    const auto inv_mu_neighbor = 1.0 / mu_neighbor;
    mu_face = 1.0 / (coef_elem * (1/mu_elem - inv_mu_neighbor) + inv_mu_neighbor);
  }
  else
    mu_face = coef_elem * (mu_elem - mu_neighbor) + mu_neighbor;

  /// ---------------------------------------------------------
      
  /// ---------- Laplacian and Non Laplacian part of diffusion -----------

  // Laplacian part: \frac{\partial u_i}{\partial x_j} n_j
  const ADReal laplacian_part = _var.gradient(face) * _face_info->normal();

  // Non Laplacian part: \frac{\partial u_j}{\partial x_i} n_j
  const ADReal non_laplacian_part = gradient_transpose.row(_index) * _normal;

  // Adding the Laplacian and Non Laplacian part of diffusion to the residual
  ADReal residual = mu_face * (laplacian_part + non_laplacian_part);

  // Adding the RC coefficients
  // *Note* : the Non Laplacian part of diffusion does not contribute to the RC coefficients
  if (has_elem)
  {
    const auto dof_number = _face_info->elem().dof_number(_sys.number(), _var.number(), 0);
    // A gradient is a linear combination of degrees of freedom so it's safe to straight-up index
    // into the derivatives vector at the dof we care about
    _ae = laplacian_part.derivatives()[dof_number];
    _ae *= -mu_face;
  }
  if (has_neighbor)
  {
    const auto dof_number = _face_info->neighbor().dof_number(_sys.number(), _var.number(), 0);
    _an = laplacian_part.derivatives()[dof_number];
    _an *= mu_face;
  }

  /// ---------------------------------------------------------

  /// ------------- Porosity derived corrections --------------

  const auto eps_elem = has_elem ? _eps(elem_face) : _eps(neighbor_face);
  const auto eps_neighbor = has_neighbor ? _eps(neighbor_face) : _eps(elem_face);

  // Get the face porosity gradient separately
  const auto & grad_eps_face =
       (has_elem && has_neighbor)
            ? MetaPhysicL::raw_value(_eps.gradient(Moose::FV::makeCDFace(*_face_info)))
            : MetaPhysicL::raw_value(_eps.gradient(
                    makeElemArg(has_elem ? &_face_info->elem() : _face_info->neighborPtr())));

  // Laplacian part of the porosity correction: (mu * u_i / eps) * (\frac{\partial eps}{\partial x_j} * n_j)
  // Computing interpolation coefficient at the face
  const auto coeff_elem = mu_elem / eps_elem * _var(elem_face);
  const auto coeff_neighbor = mu_neighbor / eps_neighbor * _var(neighbor_face);
  ADReal coeff_face;
  interpolate(
        Moose::FV::InterpMethod::Average, coeff_face, coeff_elem, coeff_neighbor, *_face_info, true);

  // Non Laplacian part of the porosity correction: (mu * (\frac{\partial eps}{\partial x_i}) / eps) * (u_j * n_j)
  const auto coeff_elem_transpose = mu_elem / eps_elem * grad_eps_face(_index);
  const auto coeff_neighbor_transpose = mu_neighbor / eps_neighbor * grad_eps_face(_index);
  ADReal coeff_face_transpose;
  interpolate(
        Moose::FV::InterpMethod::Average, coeff_face_transpose, coeff_elem_transpose, coeff_neighbor_transpose, *_face_info, true);  

  // Adding porosity corrections to the residual

  residual -= (coeff_face * grad_eps_face + coeff_face_transpose * velocity) * _normal;

  /// ---------------------------------------------------------

  return -residual;
}
