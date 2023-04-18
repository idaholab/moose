//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "IntersectionElemsHelper.h"
#include "MooseMesh.h"

class RayTracingOverlayMeshTest : public GeneralUserObject
{
public:
  RayTracingOverlayMeshTest(const InputParameters & parameters);

  static InputParameters validParams();

  void initialize();
  void finalize(){};
  void execute(){};

  std::map<dof_id_type, std::vector<dof_id_type>>
  buildElemMapping(const std::map<const Elem *, std::vector<const Elem *>> & map);

  const std::map<dof_id_type, std::vector<dof_id_type>> & getOverlayElemstoMain() const
  {
    return _overlay_elems_to_main;
  };
  const std::map<dof_id_type, std::vector<dof_id_type>> & getMainElemstoOverlay() const
  {
    return _main_elems_to_overlay;
  };

  const MeshGeneratorName & getOverlayMeshName() const { return _overlay_mesh_name; };
  const MeshGeneratorName & getMainMeshName() const { return _main_mesh_name; };

private:
  const MeshGeneratorName _main_mesh_name;
  const MeshGeneratorName _overlay_mesh_name;
  libMesh::MeshBase & _main_mesh;
  std::unique_ptr<libMesh::MeshBase> _overlay_mesh;

  // Contain intersection elems in overlay mesh
  std::map<dof_id_type, std::vector<dof_id_type>> _overlay_elems_to_main;
  // Contain intersection elems in main mesh
  std::map<dof_id_type, std::vector<dof_id_type>> _main_elems_to_overlay;
};
