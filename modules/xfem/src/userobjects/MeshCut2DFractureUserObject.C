//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCut2DFractureUserObject.h"

#include "XFEMFuncs.h"
#include "MooseError.h"
#include "MooseMesh.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_tools.h"

#include "CrackFrontDefinition.h"

registerMooseObject("XFEMApp", MeshCut2DFractureUserObject);

InputParameters
MeshCut2DFractureUserObject::validParams()
{
  InputParameters params = MeshCut2DUserObjectBase::validParams();
  params.addClassDescription("XFEM mesh cutter for 2D models that defines cuts with a"
                             "mesh and uses fracture integrals to determine growth");
  params.addRequiredParam<Real>("k_critical", "Critical fracture toughness.");
  params.addRequiredParam<Real>("growth_increment", "Length to grow crack if k>k_critical");
  params.addParam<unsigned int>("ring_number", 1, "Fracture integral ring number");
  return params;
}

MeshCut2DFractureUserObject::MeshCut2DFractureUserObject(const InputParameters & parameters)
  : MeshCut2DUserObjectBase(parameters),
    _k_critical(getParam<Real>("k_critical")),
    _growth_increment(getParam<Real>("growth_increment")),
    _ring_number_string(std::to_string(getParam<unsigned int>("ring_number")))
{
}

void
MeshCut2DFractureUserObject::initialSetup()
{
  _crack_front_definition =
      &_fe_problem.getUserObject<CrackFrontDefinition>("crackFrontDefinition");
}

void
MeshCut2DFractureUserObject::initialize()
{
  findActiveBoundaryGrowth();
  growFront();
  _crack_front_definition->isCutterModified(wasCutterMeshModified());
}

void
MeshCut2DFractureUserObject::findActiveBoundaryGrowth()
{
  const VectorPostprocessorValue & k1 = getVectorPostprocessorValueByName(
      "II_KI_" + _ring_number_string, "II_KI_" + _ring_number_string);
  const VectorPostprocessorValue & k2 = getVectorPostprocessorValueByName(
      "II_KII_" + _ring_number_string, "II_KII_" + _ring_number_string);

  // k1 is empty on the very first time step because this UO is called before the
  // InteractionIntegral vpp
  if (k1.empty())
    return;

  mooseAssert(k1.size() == k2.size(), "KI and KII VPPs should have the same size");
  mooseAssert(k1.size() == _original_and_current_front_node_ids.size(),
              "the number of crack front nodes in the should equal to the "
              "size of VPP defined at the crack front");
  std::vector<Real> k_squared = getKSquared(k1, k2);
  _active_front_node_growth_vectors.clear();
  for (unsigned int i = 0; i < _original_and_current_front_node_ids.size(); ++i)
  {
    if (k_squared[i] > (_k_critical * _k_critical) && k1[i] > 0)
    {
      // growth direction in crack front coord (cfc) system based on the  max hoop stress
      // criterion
      Real theta = 2 * std::atan((k1[i] - std::sqrt(k1[i] * k1[i] + k2[i] * k2[i])) / (4 * k2[i]));
      RealVectorValue dir_cfc;
      dir_cfc(0) = std::cos(theta);
      dir_cfc(1) = std::sin(theta);
      dir_cfc(2) = 0;

      // growth direction in global coord system based on the max hoop stress criterion
      RealVectorValue dir_global;
      dir_global = _crack_front_definition->rotateFromCrackFrontCoordsToGlobal(dir_cfc, i);
      Point dir_global_pt(dir_global(0), dir_global(1), dir_global(2));
      Point nodal_offset = dir_global_pt * _growth_increment;
      _active_front_node_growth_vectors.push_back(
          std::make_pair(_original_and_current_front_node_ids[i].second, nodal_offset));
    }
  }
}

std::vector<Real>
MeshCut2DFractureUserObject::getKSquared(const std::vector<Real> & k1,
                                         const std::vector<Real> & k2) const
{
  std::vector<Real> k_squared(k1.size());
  for (unsigned int i = 0; i < k_squared.size(); ++i)
    k_squared[i] = k1[i] * k1[i] + k2[i] * k2[i];

  return k_squared;
}
