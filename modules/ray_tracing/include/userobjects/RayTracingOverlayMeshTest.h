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

  /**
   * Store the intersection elem IDs in two-way maps for testing
   * overlay -> main map use overlay elem as key
   * main -> overlay map use main elem at the boundary as key
   */
  std::map<dof_id_type, std::vector<dof_id_type>>
  buildElemMapping(const std::map<const Elem *, std::vector<const Elem *>> & map);

  // get intersection elem mapping use overlay elems as key
  const std::map<dof_id_type, std::vector<dof_id_type>> & getOverlayElemstoMain() const
  {
    return _overlay_elems_to_main;
  };
  // get intersection elem mapping use main elems as key
  const std::map<dof_id_type, std::vector<dof_id_type>> & getMainElemstoOverlay() const
  {
    return _main_elems_to_overlay;
  };
  // get the name of the overlay mesh
  const MeshGeneratorName & getOverlayMeshName() const { return _overlay_mesh_name; };
  // get the name of the main mesh
  const MeshGeneratorName & getMainMeshName() const { return _main_mesh_name; };

private:
  // name of the main mesh
  const MeshGeneratorName _main_mesh_name;
  // name of the overlay mesh
  const MeshGeneratorName _overlay_mesh_name;
  // main mesh
  libMesh::MeshBase & _main_mesh;
  // overlay mesh
  std::unique_ptr<libMesh::MeshBase> _overlay_mesh;
  // Contain intersection elems in overlay mesh
  // overlay -> main
  std::map<dof_id_type, std::vector<dof_id_type>> _overlay_elems_to_main;
  // Contain intersection elems in main mesh
  // main -> overlay
  std::map<dof_id_type, std::vector<dof_id_type>> _main_elems_to_overlay;
};
