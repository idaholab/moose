#include "DynamicWallContactAngleFunctorMaterial.h"

#include "MooseFunctorArguments.h"
#include "MooseMesh.h"

#include <algorithm>
#include <cmath>
#include <limits>

registerMooseObject("NavierStokesApp", DynamicWallContactAngleFunctorMaterial);

namespace
{
Real
dynamicSafeMagnitude(const RealVectorValue & v)
{
  using std::sqrt;
  return sqrt(v * v);
}

RealVectorValue
dynamicSafeUnitVector(const RealVectorValue & v)
{
  const Real mag_v = dynamicSafeMagnitude(v);
  return mag_v > std::numeric_limits<Real>::epsilon() ? v / mag_v : RealVectorValue();
}

std::string
dynamicLowerCase(const std::string & input)
{
  std::string out = input;
  std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return out;
}

template <typename T>
const T &
dynamicExpandedEntry(const std::vector<T> & values,
                     const std::size_t i,
                     const std::string & param_name,
                     const std::size_t required_size)
{
  if (values.size() == 1)
    return values.front();

  if (values.size() != required_size)
    mooseError(param_name,
               " must either contain one entry or exactly ",
               required_size,
               " entries to match contact_angle_boundaries.");

  return values[i];
}
}

InputParameters
DynamicWallContactAngleFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_ALWAYS};

  params.addClassDescription(
      "Create a face-aware dynamic wall-contact-angle functor using the same provisional face "
      "unit normal seen by the reference-solver-style curvature reconstruction. The implemented law "
      "matches reference solver's dynamicAlphaContactAngle theta(U, nHat) form.");

  params.addRequiredParam<MooseFunctorName>(
      "provisional_interface_unit_normal_functor",
      "Face unit normal functor before wall contact-angle correction. This should come from the "
      "curvature producer's provisional face-normal map so the dynamic model sees the same "
      "face-smoothed alpha/gradient path as the curvature calculation.");

  params.addRequiredParam<std::vector<BoundaryName>>(
      "contact_angle_boundaries",
      "Boundary names or IDs associated with contact-angle handling. Only those marked as "
      "'dynamic' in contact_angle_models are consumed by this material.");
  params.addParam<std::vector<std::string>>(
      "contact_angle_models",
      {},
      "Per-boundary contact-angle model names. Entries equal to 'dynamic' activate the "
      "reference-solver-style dynamic law on the corresponding boundary. Empty means no dynamic "
      "contact-angle boundaries are created here.");
  params.addParam<std::vector<Real>>(
      "equilibrium_contact_angles_deg",
      {},
      "Per-boundary equilibrium contact angles theta0 in degrees for dynamic boundaries.");
  params.addParam<std::vector<Real>>(
      "advancing_contact_angles_deg",
      {},
      "Per-boundary advancing contact angles thetaA in degrees for dynamic boundaries.");
  params.addParam<std::vector<Real>>(
      "receding_contact_angles_deg",
      {},
      "Per-boundary receding contact angles thetaR in degrees for dynamic boundaries.");
  params.addParam<std::vector<Real>>(
      "contact_angle_velocity_scales",
      {},
      "Per-boundary velocity scales uTheta used by the reference solver dynamic contact-angle law.");

  params.addParam<std::vector<MooseFunctorName>>(
      "velocity_component_functors",
      {},
      "Internal velocity component functors [u, v, w] used to approximate reference solver's "
      "patchInternalField() for the dynamic contact-angle law.");
  params.addParam<MooseFunctorName>(
      "wall_velocity_functor",
      "",
      "Optional wall-velocity vector functor used for the boundary velocity Up. When omitted, "
      "default_wall_velocity is used.");
  params.addParam<RealVectorValue>(
      "default_wall_velocity",
      RealVectorValue(),
      "Constant fallback wall velocity used when wall_velocity_functor is not supplied.");

  params.addParam<Real>(
      "parallel_direction_small",
      1e-12,
      "Positive regularization added when normalizing the wall-parallel interface direction.");
  params.addParam<Real>(
      "u_theta_small",
      1e-12,
      "Positive floor below which the dynamic law falls back to theta0, matching reference solver's "
      "uTheta < SMALL behavior.");

  params.addParam<MooseFunctorName>(
      "wall_contact_angle_degrees_name",
      "dynamic_wall_contact_angle_degrees",
      "Output name for the face-aware dynamic wall contact angle functor in degrees.");

  return params;
}

DynamicWallContactAngleFunctorMaterial::DynamicWallContactAngleFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters),
    _provisional_interface_unit_normal(
        getFunctor<RealVectorValue>("provisional_interface_unit_normal_functor")),
    _velocity_component_functors{{nullptr, nullptr, nullptr}},
    _wall_velocity_functor(getParam<MooseFunctorName>("wall_velocity_functor").empty()
                               ? nullptr
                               : &getFunctor<RealVectorValue>("wall_velocity_functor")),
    _default_wall_velocity(getParam<RealVectorValue>("default_wall_velocity")),
    _parallel_direction_small(getParam<Real>("parallel_direction_small")),
    _u_theta_small(getParam<Real>("u_theta_small")),
    _wall_contact_angle_degrees_name(
        getParam<MooseFunctorName>("wall_contact_angle_degrees_name"))
{
  if (_parallel_direction_small <= 0.0)
    paramError("parallel_direction_small", "parallel_direction_small must be positive.");
  if (_u_theta_small <= 0.0)
    paramError("u_theta_small", "u_theta_small must be positive.");

  const auto velocity_names = getParam<std::vector<MooseFunctorName>>("velocity_component_functors");
  if (velocity_names.size() > _velocity_component_functors.size())
    paramError("velocity_component_functors",
               "At most ",
               _velocity_component_functors.size(),
               " velocity component functors may be supplied.");

  for (const auto i : make_range(velocity_names.size()))
    if (!velocity_names[i].empty())
      _velocity_component_functors[i] = &getFunctor<Real>(velocity_names[i]);

  initializeDynamicBoundarySpecs();

  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());
  addFunctorProperty<Real>(
      _wall_contact_angle_degrees_name,
      [this](const auto & r, const auto & t) -> Real { return evaluateWallContactAngleDegrees(r, t); },
      clearance_schedule);
}

Real
DynamicWallContactAngleFunctorMaterial::evaluateWallContactAngleDegrees(
    const Moose::FaceArg & face_arg, const Moose::StateArg & time_arg) const
{
  const FaceInfo * fi = face_arg.fi;
  if (!fi || fi->neighborPtr())
    return std::numeric_limits<Real>::quiet_NaN();

  BoundaryID boundary_id;
  const DynamicBoundarySpec * spec = nullptr;
  if (!getDynamicBoundarySpec(fi, boundary_id, spec))
    return std::numeric_limits<Real>::quiet_NaN();

  if (spec->u_theta < _u_theta_small)
    return spec->theta0_deg;

  const RealVectorValue nf = dynamicSafeUnitVector(fi->normal());

  RealVectorValue Uwall =
      evaluateBoundaryInternalVelocity(face_arg, time_arg) - evaluateBoundaryWallVelocity(face_arg, time_arg);
  Uwall -= (nf * Uwall) * nf;

  const RealVectorValue provisional_n_hat =
      MetaPhysicL::raw_value(_provisional_interface_unit_normal(face_arg, time_arg));
  RealVectorValue nWall = provisional_n_hat - (nf * provisional_n_hat) * nf;
  nWall /= (dynamicSafeMagnitude(nWall) + _parallel_direction_small);

  const Real uwall = nWall * Uwall;
  return spec->theta0_deg +
         (spec->theta_adv_deg - spec->theta_rec_deg) * std::tanh(uwall / spec->u_theta);
}

void
DynamicWallContactAngleFunctorMaterial::initializeDynamicBoundarySpecs()
{
  _dynamic_boundary_specs.clear();

  const auto boundaries = getParam<std::vector<BoundaryName>>("contact_angle_boundaries");
  if (boundaries.empty())
    return;

  const auto models = getParam<std::vector<std::string>>("contact_angle_models");
  if (models.empty())
    return;

  const auto equilibrium = getParam<std::vector<Real>>("equilibrium_contact_angles_deg");
  const auto advancing = getParam<std::vector<Real>>("advancing_contact_angles_deg");
  const auto receding = getParam<std::vector<Real>>("receding_contact_angles_deg");
  const auto u_thetas = getParam<std::vector<Real>>("contact_angle_velocity_scales");
  const auto boundary_ids = _mesh.getBoundaryIDs(boundaries, false);

  for (const auto i : make_range(boundaries.size()))
  {
    const auto model =
        dynamicLowerCase(dynamicExpandedEntry(models, i, "contact_angle_models", boundaries.size()));
    if (model != "dynamic")
      continue;

    if (equilibrium.empty())
      paramError("equilibrium_contact_angles_deg",
                 "equilibrium_contact_angles_deg is required for every dynamic contact-angle "
                 "boundary.");
    if (advancing.empty())
      paramError("advancing_contact_angles_deg",
                 "advancing_contact_angles_deg is required for every dynamic contact-angle "
                 "boundary.");
    if (receding.empty())
      paramError("receding_contact_angles_deg",
                 "receding_contact_angles_deg is required for every dynamic contact-angle "
                 "boundary.");
    if (u_thetas.empty())
      paramError("contact_angle_velocity_scales",
                 "contact_angle_velocity_scales is required for every dynamic contact-angle "
                 "boundary.");

    DynamicBoundarySpec spec;
    spec.theta0_deg = dynamicExpandedEntry(
        equilibrium, i, "equilibrium_contact_angles_deg", boundaries.size());
    spec.theta_adv_deg = dynamicExpandedEntry(
        advancing, i, "advancing_contact_angles_deg", boundaries.size());
    spec.theta_rec_deg = dynamicExpandedEntry(
        receding, i, "receding_contact_angles_deg", boundaries.size());
    spec.u_theta = dynamicExpandedEntry(
        u_thetas, i, "contact_angle_velocity_scales", boundaries.size());

    if (spec.u_theta <= 0.0)
      paramError("contact_angle_velocity_scales",
                 "All dynamic contact-angle velocity scales must be positive.");

    _dynamic_boundary_specs[boundary_ids[i]] = spec;
  }
}

bool
DynamicWallContactAngleFunctorMaterial::getDynamicBoundarySpec(
    const FaceInfo * fi,
    BoundaryID & boundary_id,
    const DynamicBoundarySpec *& spec) const
{
  spec = nullptr;
  boundary_id = BoundaryID();

  for (const auto bid : fi->boundaryIDs())
    if (const auto it = _dynamic_boundary_specs.find(bid); it != _dynamic_boundary_specs.end())
    {
      if (spec && boundary_id != bid)
        mooseError(name(),
                   ": face ",
                   fi->id(),
                   " belongs to multiple dynamic contact-angle boundaries. Provide a single "
                   "unambiguous dynamic specification per boundary face.");
      boundary_id = bid;
      spec = &it->second;
    }

  return spec != nullptr;
}

RealVectorValue
DynamicWallContactAngleFunctorMaterial::evaluateBoundaryInternalVelocity(
    const Moose::FaceArg & face_arg,
    const Moose::StateArg & time_arg) const
{
  RealVectorValue internal_velocity;
  const auto elem_arg = face_arg.makeElem();
  for (const auto i : make_range(_velocity_component_functors.size()))
    if (_velocity_component_functors[i])
      internal_velocity(i) =
          MetaPhysicL::raw_value((*_velocity_component_functors[i])(elem_arg, time_arg));

  return internal_velocity;
}

RealVectorValue
DynamicWallContactAngleFunctorMaterial::evaluateBoundaryWallVelocity(
    const Moose::FaceArg & face_arg,
    const Moose::StateArg & time_arg) const
{
  if (_wall_velocity_functor)
    return MetaPhysicL::raw_value((*_wall_velocity_functor)(face_arg, time_arg));

  return _default_wall_velocity;
}
