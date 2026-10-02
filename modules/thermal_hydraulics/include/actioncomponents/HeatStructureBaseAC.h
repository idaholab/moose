//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActionComponent.h"
#include "ComponentMeshTransformHelper.h"
#include "NamingInterface.h"
#include "HeatStructurePhysics.h"

/**
 * Base class for 2D heat-conducting solid ActionComponents (see HeatStructureCylindricalAC/
 * HeatStructurePlateAC), with a mesh built through the MeshGenerator system rather than classic
 * THM's hand-built Component2D mesh.
 *
 * The mesh is divided along the transverse ('y') direction into named regions (materials), each
 * built as its own GeneratedMeshGenerator, then combined with StackGenerator (which stitches
 * consecutive regions' shared top/bottom boundaries). Boundaries are named to match classic THM's
 * convention: '<name>:inner'/'<name>:outer' (aggregate transverse faces), '<name>:start'/
 * '<name>:end' (aggregate axial faces), '<name>:<region>:start'/'<name>:<region>:end' (per-region
 * axial faces), and '<name>:<region_i>:<region_j>' (interior region interfaces). Unlike classic
 * Component2D, there is no support for multiple named axial sections (a single uniform 'length'/
 * 'n_elems' only), matching the same simplification already made for FlowChannel1PhaseAC.
 *
 * This class creates and attaches its own HeatStructurePhysics, the same way FlowChannel1PhaseAC
 * creates and attaches its own FlowChannel1PhasePhysics - so no separate [Physics] block is needed.
 */
class HeatStructureBaseAC : public virtual ActionComponent,
                            public ComponentMeshTransformHelper,
                            public NamingInterface
{
public:
  static InputParameters validParams();
  HeatStructureBaseAC(const InputParameters & params);

  /// Returns this component's heat conduction physics
  const HeatStructurePhysics & getPhysics() const
  {
    mooseAssert(_physics, "The physics has not been created yet");
    return *_physics;
  }

protected:
  virtual void addMeshGenerators() override;
  virtual void addMaterials() override;
  virtual void addPhysics() override;

  /// Returns the transverse ('y') offset of the innermost region's inner face from the component's
  /// own axis - the inner radius for a cylindrical heat structure, or 0 for a plate
  virtual Real yOffset() const = 0;

  /// Adds a single region's material properties from the 'solid_properties'/'solid_properties_T_ref'
  /// convenience parameters
  void addConstantDensitySolidPropertiesMaterial(const UserObjectName & sp_name,
                                                 Real T_ref,
                                                 const SubdomainName & block);

  /// Names of the transverse regions
  std::vector<std::string> _region_names;
  /// Width of each transverse region
  std::vector<Real> _widths;
  /// Number of elements of each transverse region
  std::vector<unsigned int> _n_part_elems;

  /// Length of the heat structure
  const Real _length;
  /// Number of axial elements
  const unsigned int _n_elems;

  /// The heat conduction physics this component creates and attaches to itself
  HeatStructurePhysics * _physics;
};
