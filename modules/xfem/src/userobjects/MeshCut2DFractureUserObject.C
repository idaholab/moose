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
  params.addRequiredParam<Real>("growth_increment",
                                "Length to grow crack if k>k_critical or stress>stress_threshold");
  params.addParam<Real>("k_critical", "Critical fracture toughness.");
  params.addParam<Real>("stress_threshold", "Stress threshold for growing crack");
  params.addParam<VectorPostprocessorName>(
      "ki_vectorpostprocessor", "II_KI_1", "The name of the vectorpostprocessor that contains KI");
  params.addParam<VectorPostprocessorName>("kii_vectorpostprocessor",
                                           "II_KII_1",
                                           "The name of the vectorpostprocessor that contains KII");
  params.addParam<VectorPostprocessorName>(
      "stress_vectorpostprocessor",
      "The name of the vectorpostprocessor that contains crack front stress");
  return params;
}

MeshCut2DFractureUserObject::MeshCut2DFractureUserObject(const InputParameters & parameters)
  : MeshCut2DUserObjectBase(parameters),
    _growth_increment(getParam<Real>("growth_increment")),
    _use_k(isParamValid("k_critical")),
    _use_stress(isParamValid("stress_threshold")),
    _k_critical(_use_k ? getParam<Real>("k_critical") : std::numeric_limits<Real>::max()),
    _stress_threshold(_use_stress ? getParam<Real>("stress_threshold")
                                  : std::numeric_limits<Real>::max()),
    _ki_vpp(_use_k ? &getVectorPostprocessorValue(
                         "ki_vectorpostprocessor",
                         getParam<VectorPostprocessorName>("ki_vectorpostprocessor"))
                   : nullptr),
    _kii_vpp(_use_k ? &getVectorPostprocessorValue(
                          "kii_vectorpostprocessor",
                          getParam<VectorPostprocessorName>("kii_vectorpostprocessor"))
                    : nullptr),
    _stress_vpp(_use_stress
                    ? &getVectorPostprocessorValue("stress_vectorpostprocessor", "crack_tip_stress")
                    : nullptr)
{
  if (!_use_k && !_use_stress)
    paramError("k_critical",
               "Must set crack extension criterion with k_critical or stress_threshold.");
}

void
MeshCut2DFractureUserObject::initialize()
{
  _is_mesh_modified = false;
  findActiveBoundaryGrowth();
  growFront();
  addNucleatedCracksToMesh();
  // update _crack_front_definition with nucleated nodes
  _crack_front_definition->updateNumberOfCrackFrontPoints(
      _original_and_current_front_node_ids.size());
  _crack_front_definition->isCutterModified(_is_mesh_modified);
}

void
MeshCut2DFractureUserObject::findActiveBoundaryGrowth()
{
  // k1 is empty (but not a nullptr) on the very first time step because this UO is called before
  // the InteractionIntegral or crackFrontStress vpp
  if ((!_ki_vpp || _ki_vpp->size() == 0) && (!_stress_vpp || _stress_vpp->size() == 0))
    return;

  if (_use_k && ((_ki_vpp->size() != _kii_vpp->size()) ||
                 (_ki_vpp->size() != _original_and_current_front_node_ids.size())))
    mooseError("ki_vectorpostprocessor and kii_vectorpostprocessor should have the same number of "
               "crack tips as CrackFrontDefinition.",
               "\n  ki size = ",
               _ki_vpp->size(),
               "\n  kii size = ",
               _kii_vpp->size(),
               "\n  cracktips in MeshCut2DFractureUserObject = ",
               _original_and_current_front_node_ids.size());

  if (_use_stress && ((_stress_vpp->size() != _original_and_current_front_node_ids.size())))
    mooseError("stress_vectorpostprocessor should have the same number of crack front points as "
               "CrackFrontDefinition.  If it is empty, check that CrackFrontNonlocalStress "
               "vectorpostprocess has execute_on = TIMESTEP_BEGIN",
               "\n  stress_vectorpostprocessor size = ",
               _stress_vpp->size(),
               "\n  cracktips in MeshCut2DFractureUserObject = ",
               _original_and_current_front_node_ids.size());

  std::vector<Real> k_squared = getKSquared(*_ki_vpp, *_kii_vpp);
  _active_front_node_growth_vectors.clear();
  for (unsigned int i = 0; i < _original_and_current_front_node_ids.size(); ++i)
  {
    if (_use_k && k_squared[i] > (_k_critical * _k_critical) && _ki_vpp->at(i) > 0)
    {
      // growth direction in crack front coord (cfc) system based on the  max hoop stress
      // criterion
      Real ki = _ki_vpp->at(i);
      Real kii = _kii_vpp->at(i);
      Real sqrt_k = std::sqrt(ki * ki + kii * kii);
      Real theta = 2 * std::atan((ki - sqrt_k) / (4 * kii));
      RealVectorValue dir_cfc;
      dir_cfc(0) = std::cos(theta);
      dir_cfc(1) = std::sin(theta);
      dir_cfc(2) = 0;

      // growth direction in global coord system based on the max hoop stress criterion
      RealVectorValue dir_global =
          _crack_front_definition->rotateFromCrackFrontCoordsToGlobal(dir_cfc, i);
      Point dir_global_pt(dir_global(0), dir_global(1), dir_global(2));
      Point nodal_offset = dir_global_pt * _growth_increment;
      _active_front_node_growth_vectors.push_back(
          std::make_pair(_original_and_current_front_node_ids[i].second, nodal_offset));
    }
    else if (_use_stress && _stress_vpp->at(i) > _stress_threshold)
    {
      // just extending the crack in the same direction it was going
      RealVectorValue dir_cfc(1.0, 0.0, 0.0);
      RealVectorValue dir_global =
          _crack_front_definition->rotateFromCrackFrontCoordsToGlobal(dir_cfc, i);
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
