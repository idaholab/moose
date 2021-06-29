//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVWallFunctionBC.h"

registerMooseObject("NavierStokesApp", INSFVWallFunctionBC);

ADReal
find_u_star(Real mu, Real rho, ADReal u, Real dist)
{
  constexpr int MAX_ITERS {50};
  constexpr Real REL_TOLERANCE {1e-7};

  constexpr Real von_karman {0.41};

  Real nu = mu / rho;

  ADReal u_star = std::sqrt(nu * u / dist);

  for (int i = 0; i < MAX_ITERS; ++i)
  {
    ADReal residual = u_star / von_karman * std::log(u_star * dist / (0.135 * nu)) - u;
    ADReal deriv = (1 + std::log(u_star * dist / (0.135 * nu))) / von_karman;
    ADReal new_u_star = u_star - residual / deriv;

    ADReal rel_err = std::abs(new_u_star - u_star) / new_u_star;
    u_star = new_u_star;
    if (rel_err < REL_TOLERANCE) return u_star;
  }

  mooseError("Could not find the friction velocity for INSFVWallFunctionBC");
}

InputParameters
INSFVWallFunctionBC::validParams()
{
  InputParameters params = FVFluxBC::validParams();
  params.addRequiredCoupledVar("u", "The velocity in the x direction.");
  params.addCoupledVar("v", "The velocity in the y direction.");
  params.addCoupledVar("w", "The velocity in the z direction.");
  params.addParam<Real>("rho", "fluid density");
  params.addRequiredParam<MaterialPropertyName>("mu", "Dynamic viscosity");
  //params.addParam<std::vector<BoundaryName>>("walls", "Boundaries that correspond to solid walls");
  MooseEnum momentum_component("x=0 y=1 z=2", "x");
  params.addRequiredParam<MooseEnum>(
      "momentum_component",
      momentum_component,
      "The component of the momentum equation that this kernel applies to.");
  return params;
}

INSFVWallFunctionBC::INSFVWallFunctionBC(const InputParameters & params)
  : FVFluxBC(params),
    _dim(_subproblem.mesh().dimension()),
    _axis_index(getParam<MooseEnum>("momentum_component")),
    _u_var(dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("u", 0))),
    _v_var(params.isParamValid("v")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("v", 0))
               : nullptr),
    _w_var(params.isParamValid("w")
               ? dynamic_cast<const INSFVVelocityVariable *>(getFieldVar("w", 0))
               : nullptr),
    _rho(getParam<Real>("rho")),
    _mu(getADMaterialProperty<Real>("mu"))
    //_wall_boundary_names(getParam<std::vector<BoundaryName>>("walls"))
{
  //const MeshBase & mesh = _subproblem.mesh().getMesh();
  //if (!mesh.is_replicated())
  //  mooseError("WallDistanceMixingLengthAux only supports replicated meshes");
}

ADReal
INSFVWallFunctionBC::computeQpResidual()
{
  // Get the velocity vector
  const FaceInfo & fi = *_face_info;
  const Elem & elem = fi.elem();
  ADRealVectorValue velocity(_u_var->getElemValue(&elem));
  if (_v_var)
    velocity(1) = _v_var->getElemValue(&elem);
  if (_w_var)
    velocity(2) = _w_var->getElemValue(&elem);

  // Compute the velocity magnitude (parallel_speed) and
  //direction of the tangential velocity component (parallel_dir)
  Point wall_vec = fi.elemCentroid() - fi.faceCentroid(); //distance calculation may not be true for irregular elements
  Real dist = wall_vec.norm();
  Point wall_vec_unitary = wall_vec/dist; //added normalization
  ADReal perpendicular_speed = velocity * wall_vec_unitary;
  ADRealVectorValue parallel_velocity = velocity - perpendicular_speed * wall_vec_unitary;
  ADReal parallel_speed = parallel_velocity.norm();
  ADRealVectorValue parallel_dir = parallel_velocity / parallel_speed;


  if (parallel_speed.value() < 1e-6)
    return 0;

  if (!std::isfinite(parallel_speed.value()))
    return parallel_speed;

  // Compute the friction velocity and the wall shear stress
  ADReal u_star = find_u_star(_mu[_qp].value(), _rho, parallel_speed, dist);
  ADReal tau = u_star * u_star * _rho;
  //std::cout << dist * u_star * _rho / _mu[_qp].value() << "\n";

  // Compute the shear stress component for this momentum equation
  if (_axis_index == 0) {
    return tau * parallel_dir(0);
  } else if (_axis_index == 1) {
    return tau * parallel_dir(1);
  } else {
    return tau * parallel_dir(2);
  }
}
