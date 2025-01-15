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
#include "algorithm"

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
  params.addParam<std::string>("stress_vector_name",
                               "crack_tip_stress",
                               "The name of the stress vector in the stress_vectorpostprocessor");
  params.addParam<VectorPostprocessorName>(
      "k_critical_vectorpostprocessor",
      "The name of the vectorpostprocessor that contains critical fracture toughness at crack tip");
  params.addParam<std::string>(
      "k_critical_vector_name",
      "The name of the k_critical vector in the k_critical_vectorpostprocessor");
  return params;
}

MeshCut2DFractureUserObject::MeshCut2DFractureUserObject(const InputParameters & parameters)
  : MeshCut2DUserObjectBase(parameters),
    _growth_increment(getParam<Real>("growth_increment")),
    _use_k(isParamValid("k_critical") || isParamValid("k_critical_vectorpostprocessor")),
    _use_stress(isParamValid("stress_threshold")),
    _k_critical(isParamValid("k_critical") ? getParam<Real>("k_critical")
                                           : std::numeric_limits<Real>::max()),
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
                    ? &getVectorPostprocessorValue("stress_vectorpostprocessor",
                                                   getParam<std::string>("stress_vector_name"))
                    : nullptr),
    _k_critical_vpp(
        isParamValid("k_critical_vectorpostprocessor")
            ? &getVectorPostprocessorValue("k_critical_vectorpostprocessor",
                                           getParam<std::string>("k_critical_vector_name"))
            : nullptr)
{
  if (!_use_k && !_use_stress)
    paramError("k_critical",
               "Must set crack extension criterion with k_critical, k_critical_vectorpostprocessor "
               "or stress_threshold.");

  if (isParamValid("k_critical") && isParamValid("k_critical_vectorpostprocessor"))
    paramError("k_critical",
               "Fracture toughness cannot be specified by both k_critical and "
               "k_critical_vectorpostprocessor.");
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
  // The k*_vpp & stress_vpp are empty (but not a nullptr) on the very first time step because this
  // UO is called before the InteractionIntegral or crackFrontStress vpp
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

  if (_k_critical_vpp && ((_k_critical_vpp->size() != _original_and_current_front_node_ids.size())))
    mooseError("k_critical_vectorpostprocessor must have the same number of crack front points as "
               "CrackFrontDefinition.",
               "\n  k_critical_vectorpostprocessor size = ",
               _k_critical_vpp->size(),
               "\n  cracktips in MeshCut2DFractureUserObject = ",
               _original_and_current_front_node_ids.size());

  _active_front_node_growth_vectors.clear();
  for (unsigned int i = 0; i < _original_and_current_front_node_ids.size(); ++i)
  {
    if (_use_k)
    {
      Real k_crit = _k_critical;
      if (_k_critical_vpp)
        k_crit = std::min(_k_critical_vpp->at(i), _k_critical);
      Real k_squared = _ki_vpp->at(i) * _ki_vpp->at(i) + _kii_vpp->at(i) * _kii_vpp->at(i);
      if (k_squared > (k_crit * k_crit) && _ki_vpp->at(i) > 0)
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
