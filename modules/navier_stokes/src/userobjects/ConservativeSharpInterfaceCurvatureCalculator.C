#include "ConservativeSharpInterfaceCurvatureCalculator.h"

#include "FVUtils.h"
#include "MooseFunctorArguments.h"
#include "MooseMesh.h"
#include "SubProblem.h"

#include "libmesh/libmesh_common.h"

#include <cmath>
#include <limits>
#include <unordered_set>

registerMooseObject("NavierStokesApp", ConservativeSharpInterfaceCurvatureCalculator);

namespace
{
Real
safeMagnitude(const RealVectorValue & v)
{
  using std::sqrt;
  return sqrt(v * v);
}

RealVectorValue
safeUnitVector(const RealVectorValue & v)
{
  const Real mag_v = safeMagnitude(v);
  return mag_v > std::numeric_limits<Real>::epsilon() ? v / mag_v : RealVectorValue();
}

Real
clampValue(const Real value, const Real lower, const Real upper)
{
  return std::max(lower, std::min(value, upper));
}

Real
radiansFromDegrees(const Real degrees)
{
  return degrees * libMesh::pi / 180.0;
}

template <typename T>
const T &
expandedEntry(const std::vector<T> & values,
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
ConservativeSharpInterfaceCurvatureCalculator::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += NonADFunctorInterface::validParams();
  params += BlockRestrictable::validParams();
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN, EXEC_LINEAR};
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC |
                                    Moose::RelationshipManagerType::COUPLING,
                                [](const InputParameters & obj_params, InputParameters & rm_params)
                                {
                                  rm_params.set<unsigned short>("layers") = 2;
                                  rm_params.set<bool>("use_point_neighbors") = false;
                                  rm_params.set<bool>("attach_geometric_early") = true;
                                  rm_params.set<bool>("use_displaced_mesh") =
                                      obj_params.get<bool>("use_displaced_mesh");
                                });

  params.addClassDescription(
      "Produce reference-solver-like face-smoothed interface normals and cell curvature from a volume "
      "fraction functor for sharp-interface linear-FV coupling. The implementation mirrors the "
      "shipped interfaceProperties path K = -div(nHatf), with optional curvature-input alpha "
      "smoothing and wall-contact-angle correction.");

  params.addRequiredParam<MooseFunctorName>(
      "volume_fraction_functor",
      "The volume-fraction / phase-fraction functor used to reconstruct interface geometry.");

  MooseEnum delta_n_mode("mesh_scaled_reference fixed", "mesh_scaled_reference");
  params.addParam<MooseEnum>(
      "delta_n_mode",
      delta_n_mode,
      "How the unit-normal regularization delta_n is chosen. 'mesh_scaled_reference' mirrors "
      "reference solver's deltaN = 1e-8 / cbrt(average(cellVolume)).");
  params.addParam<Real>(
      "delta_n_scale",
      1e-8,
      "Scale factor used when delta_n_mode = mesh_scaled_reference. The effective value becomes "
      "delta_n_scale / cbrt(average cell volume), matching reference solver's default "
      "interfaceProperties behavior.");
  params.addParam<Real>(
      "delta_n_fixed_value", 1e-8, "Fixed value used when delta_n_mode = fixed.");

  params.addParam<bool>(
      "use_reference_simple_curvature",
      true,
      "Use the baseline reference solver simple curvature expression K = -div(nHatf). The optional "
      "higher-order correction path remains intentionally unsupported in this step.");

  params.addParam<unsigned int>(
      "n_alpha_smooth_curvature",
      0,
      "Optional number of curvature-input smoothing sweeps. Each sweep performs a MOOSE FV "
      "analog of alpha <- average(interpolate(alpha)) before grad(alpha) is rebuilt. A value "
      "of 0 matches the classic baseline path.");

  params.addParam<std::vector<BoundaryName>>(
      "contact_angle_boundaries",
      {},
      "Boundary names or IDs on which the reference-solver-like wall contact-angle correction should be "
      "applied.");
  params.addParam<std::vector<Real>>(
      "static_contact_angles_degrees",
      {},
      "Static wall contact angles in degrees, one per selected boundary. A single value is "
      "broadcast to all selected boundaries.");
  params.addParam<MooseFunctorName>(
      "wall_contact_angle_degrees_functor",
      "",
      "Optional face-aware functor returning wall contact angle in degrees. When supplied, it "
      "overrides static_contact_angles_degrees on the selected boundaries.");
  params.addParam<Real>(
      "contact_angle_small_det",
      1e-12,
      "Positive floor used when the reference-solver-style wall-contact-angle determinant 1 - "
      "(nHat.nWall)^2 becomes very small.");

  params.addParam<MooseFunctorName>(
      "face_smoothed_alpha_gradient_name",
      "curvature_face_smoothed_alpha_gradient",
      "Output name for the reference-solver-like face-smoothed alpha gradient functor.");
  params.addParam<MooseFunctorName>(
      "provisional_face_unit_normal_name",
      "curvature_provisional_interface_unit_normal_face",
      "Output name for the provisional face unit normal functor used before wall-contact-angle "
      "correction is applied.");
  params.addParam<MooseFunctorName>(
      "face_unit_normal_name",
      "curvature_interface_unit_normal_face",
      "Output name for the corrected face unit normal functor.");
  params.addParam<MooseFunctorName>(
      "curvature_name", "curvature", "Output name for the cell-centered curvature functor.");

  return params;
}

ConservativeSharpInterfaceCurvatureCalculator::ConservativeSharpInterfaceCurvatureCalculator(const InputParameters & params)
  : GeneralUserObject(params),
    NonADFunctorInterface(this),
    BlockRestrictable(this),
    _moose_mesh(UserObject::_subproblem.mesh()),
    _face_smoothed_alpha_gradient(_moose_mesh,
                                  blockIDs(),
                                  getParam<MooseFunctorName>("face_smoothed_alpha_gradient_name")),
    _provisional_face_unit_normal(
        _moose_mesh, blockIDs(), getParam<MooseFunctorName>("provisional_face_unit_normal_name")),
    _face_unit_normal(_moose_mesh,
                      blockIDs(),
                      getParam<MooseFunctorName>("face_unit_normal_name")),
    _curvature(_moose_mesh, blockIDs(), getParam<MooseFunctorName>("curvature_name"), false),
    _volume_fraction(this->template getFunctor<Real>("volume_fraction_functor")),
    _wall_contact_angle_degrees_functor_name(
        getParam<MooseFunctorName>("wall_contact_angle_degrees_functor")),
    _delta_n_mode(getParam<MooseEnum>("delta_n_mode")),
    _delta_n_scale(getParam<Real>("delta_n_scale")),
    _delta_n_fixed_value(getParam<Real>("delta_n_fixed_value")),
    _use_reference_simple_curvature(getParam<bool>("use_reference_simple_curvature")),
    _n_alpha_smooth_curvature(getParam<unsigned int>("n_alpha_smooth_curvature")),
    _contact_angle_small_det(getParam<Real>("contact_angle_small_det")),
    _face_smoothed_alpha_gradient_name(
        getParam<MooseFunctorName>("face_smoothed_alpha_gradient_name")),
    _provisional_face_unit_normal_name(
        getParam<MooseFunctorName>("provisional_face_unit_normal_name")),
    _face_unit_normal_name(getParam<MooseFunctorName>("face_unit_normal_name")),
    _curvature_name(getParam<MooseFunctorName>("curvature_name")),
    _effective_delta_n(0.0)
{
  if (_delta_n_scale <= 0.0)
    paramError("delta_n_scale", "delta_n_scale must be positive.");
  if (_delta_n_fixed_value <= 0.0)
    paramError("delta_n_fixed_value", "delta_n_fixed_value must be positive.");
  if (_contact_angle_small_det <= 0.0)
    paramError("contact_angle_small_det", "contact_angle_small_det must be positive.");

  for (const auto tid : make_range(libMesh::n_threads()))
  {
    UserObject::_subproblem.addFunctor(
        _face_smoothed_alpha_gradient_name, _face_smoothed_alpha_gradient, tid);
    UserObject::_subproblem.addFunctor(
        _provisional_face_unit_normal_name, _provisional_face_unit_normal, tid);
    UserObject::_subproblem.addFunctor(_face_unit_normal_name, _face_unit_normal, tid);
    UserObject::_subproblem.addFunctor(_curvature_name, _curvature, tid);
  }

  parseStaticContactAngles();
}

void
ConservativeSharpInterfaceCurvatureCalculator::parseStaticContactAngles()
{
  _static_contact_angle_radians.clear();

  const auto boundaries = getParam<std::vector<BoundaryName>>("contact_angle_boundaries");
  if (boundaries.empty())
    return;

  const auto boundary_ids = _moose_mesh.getBoundaryIDs(boundaries, false);
  const auto static_angles_deg = getParam<std::vector<Real>>("static_contact_angles_degrees");

  if (_wall_contact_angle_degrees_functor_name.empty() && static_angles_deg.empty())
    paramError("static_contact_angles_degrees",
               "Provide static_contact_angles_degrees or wall_contact_angle_degrees_functor when "
               "contact_angle_boundaries are specified.");

  if (!static_angles_deg.empty())
    for (const auto i : make_range(boundaries.size()))
      _static_contact_angle_radians[boundary_ids[i]] =
          radiansFromDegrees(expandedEntry(
              static_angles_deg, i, "static_contact_angles_degrees", boundaries.size()));
}

void
ConservativeSharpInterfaceCurvatureCalculator::rebuildSharpInterfaceFaceInfo()
{
  _sharp_interface_face_info.clear();

  std::unordered_set<dof_id_type> inserted_face_ids;
  _sharp_interface_face_info.reserve(_moose_mesh.nFace());

  const auto add_face = [this, &inserted_face_ids](const FaceInfo * const fi)
  {
    if (fi && isFaceGeometricallyRelevant(*fi) && inserted_face_ids.insert(fi->id()).second)
      _sharp_interface_face_info.push_back(fi);
  };

  for (const auto * fi : _moose_mesh.faceInfo())
    add_face(fi);

  _sharp_interface_face_info.shrink_to_fit();
}

bool
ConservativeSharpInterfaceCurvatureCalculator::elemInBlocks(const Elem * elem) const
{
  return elem && elem != libMesh::remote_elem && (blockIDs().empty() || hasBlocks(elem->subdomain_id()));
}

bool
ConservativeSharpInterfaceCurvatureCalculator::faceTouchesBlocks(const FaceInfo * fi) const
{
  return elemInBlocks(fi->elemPtr()) || elemInBlocks(fi->neighborPtr());
}

bool
ConservativeSharpInterfaceCurvatureCalculator::isFaceGeometricallyRelevant(const FaceInfo & fi) const
{
  if (&fi.elem() == libMesh::remote_elem)
    return false;

  bool on_active_blocks = elemInBlocks(fi.elemPtr());

  if (fi.neighborPtr())
  {
    if (&fi.neighbor() == libMesh::remote_elem)
      return false;

    on_active_blocks = on_active_blocks || elemInBlocks(fi.neighborPtr());
  }

  if (!on_active_blocks)
    return false;

  if (!Moose::FV::onBoundary(blockIDs(), fi))
    return true;

  const auto & boundary_elem =
      (fi.neighborPtr() && elemInBlocks(fi.neighborPtr())) ? fi.neighbor() : fi.elem();

  for (auto * const neighbor : boundary_elem.neighbor_ptr_range())
    if (neighbor == libMesh::remote_elem)
      return false;

  return true;
}

Real
ConservativeSharpInterfaceCurvatureCalculator::faceMeasure(const FaceInfo * fi) const
{
  return fi->faceArea() * fi->faceCoord();
}

Real
ConservativeSharpInterfaceCurvatureCalculator::elemMeasure(const FaceInfo * fi, const bool neighbor) const
{
  const auto * elem_info = neighbor ? fi->neighborInfo() : fi->elemInfo();
  mooseAssert(elem_info, "Missing ElemInfo in ConservativeSharpInterfaceCurvatureCalculator::elemMeasure");
  return (neighbor ? fi->neighborVolume() : fi->elemVolume()) * elem_info->coordFactor();
}

void
ConservativeSharpInterfaceCurvatureCalculator::updateEffectiveDeltaN()
{
  if (_delta_n_mode == "fixed")
  {
    _effective_delta_n = _delta_n_fixed_value;
    return;
  }

  std::unordered_map<dof_id_type, Real> unique_elem_volumes;
  for (const auto * fi : _sharp_interface_face_info)
  {
    if (elemInBlocks(fi->elemPtr()))
      unique_elem_volumes.emplace(fi->elemPtr()->id(), elemMeasure(fi, false));
    if (elemInBlocks(fi->neighborPtr()))
      unique_elem_volumes.emplace(fi->neighborPtr()->id(), elemMeasure(fi, true));
  }

  Real local_volume_sum = 0.0;
  dof_id_type local_cell_count = 0;
  for (const auto & pr : unique_elem_volumes)
  {
    local_volume_sum += pr.second;
    local_cell_count += 1;
  }

  _communicator.sum(local_volume_sum);
  _communicator.sum(local_cell_count);

  if (!local_cell_count)
    mooseError("No cells were found for ConservativeSharpInterfaceCurvatureCalculator when computing delta_n.");

  const Real average_volume = local_volume_sum / static_cast<Real>(local_cell_count);
  _effective_delta_n = _delta_n_scale / std::cbrt(average_volume);
}

Moose::FaceArg
ConservativeSharpInterfaceCurvatureCalculator::makeCenteredFaceArg(const FaceInfo * fi,
                                                       const Moose::StateArg * limiter_state) const
{
  return Moose::FaceArg{
      fi, Moose::FV::LimiterType::CentralDifference, true, false, nullptr, limiter_state};
}

void
ConservativeSharpInterfaceCurvatureCalculator::buildCellAlphaField(
    const Moose::StateArg & time_arg,
    std::unordered_map<dof_id_type, Real> & cell_alpha) const
{
  cell_alpha.clear();

  for (const auto * fi : _sharp_interface_face_info)
  {
    if (elemInBlocks(fi->elemPtr()))
      cell_alpha.emplace(fi->elemPtr()->id(),
                         MetaPhysicL::raw_value(
                             _volume_fraction(Moose::ElemArg{fi->elemPtr(), false}, time_arg)));
    if (elemInBlocks(fi->neighborPtr()))
      cell_alpha.emplace(fi->neighborPtr()->id(),
                         MetaPhysicL::raw_value(
                             _volume_fraction(Moose::ElemArg{fi->neighborPtr(), false}, time_arg)));
  }
}

Real
ConservativeSharpInterfaceCurvatureCalculator::interpolateCellScalarToFace(
    const FaceInfo * fi,
    const std::unordered_map<dof_id_type, Real> & cell_field) const
{
  const bool elem_ok = elemInBlocks(fi->elemPtr()) && cell_field.count(fi->elemPtr()->id());
  const bool neighbor_ok = fi->neighborPtr() && elemInBlocks(fi->neighborPtr()) &&
                           cell_field.count(fi->neighborPtr()->id());

  if (elem_ok && neighbor_ok)
    return fi->gC() * cell_field.at(fi->elemPtr()->id()) +
           (1.0 - fi->gC()) * cell_field.at(fi->neighborPtr()->id());
  else if (elem_ok)
    return cell_field.at(fi->elemPtr()->id());
  else if (neighbor_ok)
    return cell_field.at(fi->neighborPtr()->id());

  return 0.0;
}

void
ConservativeSharpInterfaceCurvatureCalculator::smoothCellAlphaField(
    std::unordered_map<dof_id_type, Real> & cell_alpha) const
{
  if (!_n_alpha_smooth_curvature)
    return;

  for (unsigned int sweep = 0; sweep < _n_alpha_smooth_curvature; ++sweep)
  {
    std::unordered_map<dof_id_type, Real> alpha_sum;
    std::unordered_map<dof_id_type, Real> area_sum;

    for (const auto * fi : _sharp_interface_face_info)
    {
      const Real alpha_f = interpolateCellScalarToFace(fi, cell_alpha);
      const Real area = faceMeasure(fi);

      if (elemInBlocks(fi->elemPtr()))
      {
        alpha_sum[fi->elemPtr()->id()] += alpha_f * area;
        area_sum[fi->elemPtr()->id()] += area;
      }

      if (elemInBlocks(fi->neighborPtr()))
      {
        alpha_sum[fi->neighborPtr()->id()] += alpha_f * area;
        area_sum[fi->neighborPtr()->id()] += area;
      }
    }

    for (auto & pr : cell_alpha)
      if (const auto it = area_sum.find(pr.first); it != area_sum.end() && it->second > 0.0)
      {
        const Real alpha_avg = alpha_sum[pr.first] / it->second;
        pr.second = clampValue(alpha_avg, 0.0, 1.0);
      }
  }
}

Real
ConservativeSharpInterfaceCurvatureCalculator::contactAngleRadiansForFace(
    const FaceInfo * fi,
    const Moose::StateArg & time_arg,
    const Moose::StateArg * limiter_state) const
{
  if (fi->neighborPtr())
    return std::numeric_limits<Real>::quiet_NaN();

  const auto face_arg = makeCenteredFaceArg(fi, limiter_state);

  if (!_wall_contact_angle_degrees_functor_name.empty())
  {
    const auto & theta_functor = UserObject::_subproblem.getFunctor<Real>(
        _wall_contact_angle_degrees_functor_name, _tid, name(), false);
    const Real theta_deg = MetaPhysicL::raw_value(theta_functor(face_arg, time_arg));
    if (std::isfinite(theta_deg))
      return radiansFromDegrees(theta_deg);
  }

  const Real * static_theta = nullptr;
  for (const auto bid : fi->boundaryIDs())
    if (const auto it = _static_contact_angle_radians.find(bid);
        it != _static_contact_angle_radians.end())
    {
      if (static_theta && *static_theta != it->second)
        mooseError(name(),
                   ": face ",
                   fi->id(),
                   " belongs to multiple selected contact-angle boundaries with different static "
                   "angles.");
      static_theta = &it->second;
    }

  return static_theta ? *static_theta : std::numeric_limits<Real>::quiet_NaN();
}

RealVectorValue
ConservativeSharpInterfaceCurvatureCalculator::correctBoundaryContactAngle(
    const FaceInfo * fi,
    const RealVectorValue & provisional_n_hat_face,
    const RealVectorValue & face_gradient,
    const Moose::StateArg & time_arg,
    const Moose::StateArg * limiter_state,
    RealVectorValue * corrected_face_gradient) const
{
  if (corrected_face_gradient)
    *corrected_face_gradient = face_gradient;

  const Real theta = contactAngleRadiansForFace(fi, time_arg, limiter_state);
  if (std::isnan(theta))
    return provisional_n_hat_face;

  const RealVectorValue wall_normal = safeUnitVector(fi->normal());
  const Real a12 = clampValue(provisional_n_hat_face * wall_normal, -1.0, 1.0);
  const Real b1 = std::cos(theta);
  const Real b2 = std::cos(std::acos(a12) - theta);
  const Real det = std::max(_contact_angle_small_det, 1.0 - a12 * a12);
  const Real a = (b1 - a12 * b2) / det;
  const Real b = (b2 - a12 * b1) / det;

  RealVectorValue corrected_n_hat_face = a * wall_normal + b * provisional_n_hat_face;
  corrected_n_hat_face /= (safeMagnitude(corrected_n_hat_face) + effectiveDeltaN());

  if (corrected_face_gradient)
    *corrected_face_gradient = corrected_n_hat_face * safeMagnitude(face_gradient);

  return corrected_n_hat_face;
}

void
ConservativeSharpInterfaceCurvatureCalculator::meshChanged()
{
  GeneralUserObject::meshChanged();
  rebuildSharpInterfaceFaceInfo();
  updateEffectiveDeltaN();
}

void
ConservativeSharpInterfaceCurvatureCalculator::initialSetup()
{
  GeneralUserObject::initialSetup();
  parseStaticContactAngles();
  rebuildSharpInterfaceFaceInfo();
  updateEffectiveDeltaN();
}

void
ConservativeSharpInterfaceCurvatureCalculator::initialize()
{
  updateCurvatureMaps(false);
}

void
ConservativeSharpInterfaceCurvatureCalculator::execute()
{
  updateCurvatureMaps(false);
}

void
ConservativeSharpInterfaceCurvatureCalculator::finalize()
{
}

void
ConservativeSharpInterfaceCurvatureCalculator::updateCurvatureMaps(const bool verbose)
{
  if (!_use_reference_simple_curvature)
    mooseError(
        name(),
        ": only the baseline reference solver simple curvature path is implemented in this step. "
        "The optional higher-order correction term remains intentionally unsupported here.");

  rebuildSharpInterfaceFaceInfo();
  updateEffectiveDeltaN();
  _face_smoothed_alpha_gradient.clear();
  _provisional_face_unit_normal.clear();
  _face_unit_normal.clear();
  _curvature.clear();

  const auto time_arg = Moose::currentState();
  const auto limiter_time = _fe_problem.isTransient()
                                ? Moose::StateArg(1, Moose::SolutionIterationType::Time)
                                : Moose::StateArg(1, Moose::SolutionIterationType::Nonlinear);

  CellCenteredMapFunctor<Real, std::unordered_map<dof_id_type, Real>> smoothed_alpha(
      _moose_mesh, blockIDs(), name() + "_smoothed_alpha_work", true);

  if (_n_alpha_smooth_curvature)
  {
    std::unordered_map<dof_id_type, Real> cell_alpha;
    buildCellAlphaField(time_arg, cell_alpha);
    smoothCellAlphaField(cell_alpha);

    for (const auto & pr : cell_alpha)
      smoothed_alpha[pr.first] = pr.second;

    if (verbose)
      _console << name() << ": applied " << _n_alpha_smooth_curvature
               << " curvature-input smoothing sweep(s)." << std::endl;
  }

  std::unordered_map<dof_id_type, Real> curvature_flux_sum;
  std::unordered_map<dof_id_type, Real> element_volumes;

  for (const auto * fi : _sharp_interface_face_info)
  {
    const auto face_arg = makeCenteredFaceArg(fi, &limiter_time);
    const RealVectorValue face_grad =
        _n_alpha_smooth_curvature
            ? MetaPhysicL::raw_value(smoothed_alpha.gradient(face_arg, time_arg))
            : MetaPhysicL::raw_value(_volume_fraction.gradient(face_arg, time_arg));

    const Real mag_face_grad = safeMagnitude(face_grad);
    const RealVectorValue provisional_n_hat_face = face_grad / (mag_face_grad + effectiveDeltaN());

    _provisional_face_unit_normal[fi->id()] = provisional_n_hat_face;

    RealVectorValue corrected_face_grad = face_grad;
    const RealVectorValue n_hat_face = correctBoundaryContactAngle(fi,
                                                                   provisional_n_hat_face,
                                                                   face_grad,
                                                                   time_arg,
                                                                   &limiter_time,
                                                                   &corrected_face_grad);

    _face_smoothed_alpha_gradient[fi->id()] = corrected_face_grad;
    _face_unit_normal[fi->id()] = n_hat_face;

    const Real face_flux = (n_hat_face * safeUnitVector(fi->normal())) * faceMeasure(fi);

    if (elemInBlocks(fi->elemPtr()))
    {
      const auto elem_id = fi->elemPtr()->id();
      curvature_flux_sum[elem_id] += face_flux;
      element_volumes.emplace(elem_id, elemMeasure(fi, false));
    }

    if (elemInBlocks(fi->neighborPtr()))
    {
      const auto neighbor_id = fi->neighborPtr()->id();
      curvature_flux_sum[neighbor_id] -= face_flux;
      element_volumes.emplace(neighbor_id, elemMeasure(fi, true));
    }

    if (verbose)
      _console << "Curvature face " << fi->id() << ": |grad_alpha_f|=" << mag_face_grad
               << ", delta_n=" << effectiveDeltaN() << ", face_flux=" << face_flux
               << std::endl;
  }

  for (const auto & pr : element_volumes)
  {
    const auto elem_id = pr.first;
    const auto volume = pr.second;
    const auto flux_it = curvature_flux_sum.find(elem_id);
    const Real outward_flux = flux_it == curvature_flux_sum.end() ? 0.0 : flux_it->second;
    _curvature[elem_id] = -outward_flux / volume;
  }

  for (const auto tid : make_range(libMesh::n_threads()))
  {
    auto & face_smoothed_alpha_gradient = const_cast<Moose::Functor<RealVectorValue> &>(
        UserObject::_subproblem.getFunctor<RealVectorValue>(
            _face_smoothed_alpha_gradient_name, tid, name(), true));
    face_smoothed_alpha_gradient.assign(_face_smoothed_alpha_gradient);

    auto & provisional_face_unit_normal = const_cast<Moose::Functor<RealVectorValue> &>(
        UserObject::_subproblem.getFunctor<RealVectorValue>(
            _provisional_face_unit_normal_name, tid, name(), true));
    provisional_face_unit_normal.assign(_provisional_face_unit_normal);

    auto & face_unit_normal = const_cast<Moose::Functor<RealVectorValue> &>(
        UserObject::_subproblem.getFunctor<RealVectorValue>(_face_unit_normal_name, tid, name(), true));
    face_unit_normal.assign(_face_unit_normal);

    auto & curvature = const_cast<Moose::Functor<Real> &>(
        UserObject::_subproblem.getFunctor<Real>(_curvature_name, tid, name(), true));
    curvature.assign(_curvature);
  }
}
