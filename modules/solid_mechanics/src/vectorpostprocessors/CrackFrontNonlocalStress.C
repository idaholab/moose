//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackFrontNonlocalStress.h"
#include "Moose.h"
#include "MooseEnum.h"
#include "RankTwoTensor.h"
#include "Conversion.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature.h"
#include "libmesh/utility.h"
#include "CrackFrontDefinition.h"
#include "RankTwoScalarTools.h"
#include "libmesh/utility.h"

registerMooseObject("SolidMechanicsApp", CrackFrontNonlocalStress);

InputParameters
CrackFrontNonlocalStress::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>("crack_front_definition",
                                          "The CrackFrontDefinition user object name");
  params.addRequiredParam<Real>("box_length", "Distance in front of crack front.");
  params.addRequiredParam<Real>("box_height", "Distance normal to front of crack front.");
  params.addParam<Real>("box_width", "Distance tangent to front of crack front.");

  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.set<bool>("use_displaced_mesh") = false;
  // EXEC_NONLINEAR to work with xfem_udpates
  params.set<ExecFlagEnum>("execute_on") = {EXEC_TIMESTEP_BEGIN};
  params.addClassDescription("Computes the average stress normal to the crack face.");
  return params;
}

CrackFrontNonlocalStress::CrackFrontNonlocalStress(const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    _box_length(getParam<Real>("box_length")),
    _box_width(isParamValid("box_width") ? getParam<Real>("box_width") : 1),
    _box_height(getParam<Real>("box_height")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _x(declareVector("x")),
    _y(declareVector("y")),
    _z(declareVector("z")),
    _position(declareVector("id")),
    _avg_crack_tip_stress(declareVector("crack_tip_stress"))
{
  if (_mesh.dimension() == 3 && !isParamSetByUser("box_width"))
    paramError("box_width", "Must define box_width in 3D problems.");
  // add user object dependencies by name (the UOs do not need to exist yet for this)
  _depend_uo.insert(getParam<UserObjectName>("crack_front_definition"));
}

void
CrackFrontNonlocalStress::initialSetup()
{
  // gather coupled user objects late to ensure they are constructed. Do not add them as
  // dependencies (that's already done in the constructor).
  const auto uo_name = getParam<UserObjectName>("crack_front_definition");
  _crack_front_definition =
      &(getUserObjectByName<CrackFrontDefinition>(uo_name, /*is_dependency = */ false));
}

void
CrackFrontNonlocalStress::initialize()
{
  std::size_t num_pts = _crack_front_definition->getNumCrackFrontPoints();

  _volume.assign(num_pts, 0.0);
  _x.assign(num_pts, 0.0);
  _y.assign(num_pts, 0.0);
  _z.assign(num_pts, 0.0);
  _position.assign(num_pts, 0.0);
  _avg_crack_tip_stress.assign(num_pts, 0.0);
}

void
CrackFrontNonlocalStress::execute()
{
  // icfp crack front point index
  for (std::size_t icfp = 0; icfp < _avg_crack_tip_stress.size(); icfp++)
  {
    Point crack_face_normal = _crack_front_definition->getCrackFrontNormal(icfp);
    RankTwoTensor stress_avg;
    for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
    {
      Real q = BoxWeightingFunction(icfp, _q_point[qp]);
      if (q == 0)
        continue;

      Real normal_stress = RankTwoScalarTools::directionValueTensor(_stress[qp], crack_face_normal);
      _avg_crack_tip_stress[icfp] += _JxW[qp] * _coord[qp] * normal_stress * q;
      _volume[icfp] += _JxW[qp] * _coord[qp] * q;
    }
  }
}

void
CrackFrontNonlocalStress::finalize()
{
  gatherSum(_avg_crack_tip_stress);
  gatherSum(_volume);

  for (std::size_t icfp = 0; icfp < _avg_crack_tip_stress.size(); ++icfp)
  {
    if (_volume[icfp] != 0)
      _avg_crack_tip_stress[icfp] = _avg_crack_tip_stress[icfp] / _volume[icfp];
    else
      _avg_crack_tip_stress[icfp] = 0;

    const auto cfp = _crack_front_definition->getCrackFrontPoint(icfp);
    _x[icfp] = (*cfp)(0);
    _y[icfp] = (*cfp)(1);
    _z[icfp] = (*cfp)(2);
    _position[icfp] = _crack_front_definition->getDistanceAlongFront(icfp);
  }
}

void
CrackFrontNonlocalStress::threadJoin(const UserObject & y)
{
  const auto & uo = static_cast<const CrackFrontNonlocalStress &>(y);

  for (auto i = beginIndex(_avg_crack_tip_stress); i < _avg_crack_tip_stress.size(); ++i)
  {
    _volume[i] += uo._volume[i];
    _avg_crack_tip_stress[i] += uo._avg_crack_tip_stress[i];
  }
}

Real
CrackFrontNonlocalStress::BoxWeightingFunction(std::size_t crack_front_point_index,
                                               const Point & qp_coord) const
{
  const Point * cf_pt = _crack_front_definition->getCrackFrontPoint(crack_front_point_index);
  RealVectorValue crack_node_to_current_node = qp_coord - *cf_pt;

  // crackfront coordinates are:
  // crack_node_to_current_node_rot[0]= crack direction
  // crack_node_to_current_node_rot[0]= normal to crack face
  // crack_node_to_current_node_rot[0]= tangent to crack face along crack front (not used in 2D)
  RealVectorValue crack_node_to_current_node_rot =
      _crack_front_definition->rotateToCrackFrontCoords(crack_node_to_current_node,
                                                        crack_front_point_index);
  Real q = 0.0;
  if ((crack_node_to_current_node_rot(0) > 0) &&
      (crack_node_to_current_node_rot(0) <= _box_length) &&
      (std::abs(crack_node_to_current_node_rot(1)) <= _box_height / 2) &&
      (std::abs(crack_node_to_current_node_rot(2)) <= _box_width / 2))
  {
    q = 1.0;
  }

  return q;
}
