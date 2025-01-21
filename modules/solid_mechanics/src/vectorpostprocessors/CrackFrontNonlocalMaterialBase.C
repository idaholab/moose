//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackFrontNonlocalMaterialBase.h"
#include "CrackFrontDefinition.h"

InputParameters
CrackFrontNonlocalMaterialBase::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  params.addRequiredParam<Real>(
      "box_length", "Dimension of property-averaging box in direction of crack extension.");
  params.addRequiredParam<Real>(
      "box_height", "Dimension of property-averaging box in direction normal to crack.");
  params.addParam<Real>("box_width", 1.0, "Distance tangent to front of crack front.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.set<bool>("use_displaced_mesh") = false;
  params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_BEGIN};
  params.addClassDescription("Computes the average material property at a crack front.");
  return params;
}

CrackFrontNonlocalMaterialBase::CrackFrontNonlocalMaterialBase(const InputParameters & parameters,
                                                               const std::string & property_name)
  : ElementVectorPostprocessor(parameters),
    _property_name(property_name),
    _box_length(getParam<Real>("box_length")),
    _box_width(getParam<Real>("box_width")),
    _box_height(getParam<Real>("box_height")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _x(declareVector("x")),
    _y(declareVector("y")),
    _z(declareVector("z")),
    _position(declareVector("id")),
    // get the property name instead of materialname
    _avg_crack_tip_scalar(declareVector("crack_tip_" + _base_name + _property_name))
{
  if (_mesh.dimension() == 3 && !isParamSetByUser("box_width"))
    paramError("box_width", "Must define box_width in 3D problems.");
  // add user object dependencies by name (the UOs do not need to exist yet for this)
  _depend_uo.insert(getParam<UserObjectName>("crack_front_definition"));
}

void
CrackFrontNonlocalMaterialBase::initialSetup()
{
  const auto uo_name = getParam<UserObjectName>("crack_front_definition");
  _crack_front_definition =
      &(getUserObjectByName<CrackFrontDefinition>(uo_name, /*is_dependency = */ false));
}

void
CrackFrontNonlocalMaterialBase::initialize()
{
  std::size_t num_pts = _crack_front_definition->getNumCrackFrontPoints();

  _volume.assign(num_pts, 0.0);
  _x.assign(num_pts, 0.0);
  _y.assign(num_pts, 0.0);
  _z.assign(num_pts, 0.0);
  _position.assign(num_pts, 0.0);
  _avg_crack_tip_scalar.assign(num_pts, 0.0);
}

void
CrackFrontNonlocalMaterialBase::execute()
{
  // icfp crack front point index
  for (const auto icfp : index_range(_avg_crack_tip_scalar))
  {
    Point crack_front_normal = _crack_front_definition->getCrackFrontNormal(icfp);
    for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    {
      Real q = BoxWeightingFunction(icfp, _q_point[qp]);
      if (q == 0)
        continue;

      Real scalar = getQPCrackFrontScalar(qp, crack_front_normal);
      _avg_crack_tip_scalar[icfp] += _JxW[qp] * _coord[qp] * scalar * q;
      _volume[icfp] += _JxW[qp] * _coord[qp] * q;
    }
  }
}

void
CrackFrontNonlocalMaterialBase::finalize()
{
  gatherSum(_avg_crack_tip_scalar);
  gatherSum(_volume);
  for (const auto icfp : index_range(_avg_crack_tip_scalar))
  {
    if (_volume[icfp] != 0)
      _avg_crack_tip_scalar[icfp] = _avg_crack_tip_scalar[icfp] / _volume[icfp];
    else
      _avg_crack_tip_scalar[icfp] = 0;

    const auto cfp = _crack_front_definition->getCrackFrontPoint(icfp);
    _x[icfp] = (*cfp)(0);
    _y[icfp] = (*cfp)(1);
    _z[icfp] = (*cfp)(2);
    _position[icfp] = _crack_front_definition->getDistanceAlongFront(icfp);
  }
}

void
CrackFrontNonlocalMaterialBase::threadJoin(const UserObject & y)
{
  const auto & uo = static_cast<const CrackFrontNonlocalMaterialBase &>(y);
  for (const auto i : index_range(_avg_crack_tip_scalar))
  {
    _volume[i] += uo._volume[i];
    _avg_crack_tip_scalar[i] += uo._avg_crack_tip_scalar[i];
  }
}

Real
CrackFrontNonlocalMaterialBase::BoxWeightingFunction(std::size_t crack_front_point_index,
                                                     const Point & qp_coord) const
{
  const Point * cf_pt = _crack_front_definition->getCrackFrontPoint(crack_front_point_index);
  RealVectorValue crack_node_to_current_node = qp_coord - *cf_pt;

  // crackfront coordinates are:
  // crack_node_to_current_node_rot[0]= crack direction
  // crack_node_to_current_node_rot[1]= normal to crack face
  // crack_node_to_current_node_rot[2]= tangent to crack face along crack front (not used in 2D)
  RealVectorValue crack_node_to_current_node_rot =
      _crack_front_definition->rotateToCrackFrontCoords(crack_node_to_current_node,
                                                        crack_front_point_index);
  if ((crack_node_to_current_node_rot(0) > 0) &&
      (crack_node_to_current_node_rot(0) <= _box_length) &&
      (std::abs(crack_node_to_current_node_rot(1)) <= _box_height / 2) &&
      (std::abs(crack_node_to_current_node_rot(2)) <= _box_width / 2))
    return 1.0;

  return 0.0;
}
