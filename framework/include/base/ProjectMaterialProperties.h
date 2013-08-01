/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef PROJECTMATERIALPROPERTIES_H
#define PROJECTMATERIALPROPERTIES_H

#include "ThreadedElementLoop.h"
// libMesh includes
#include "libmesh/elem_range.h"

class FEProblem;
class NonlinearSystem;
class MaterialWarehouse;
class MaterialPropertyStorage;
class MaterialData;
class Assembly;

class ProjectMaterialProperties : public ThreadedElementLoop<ConstElemPointerRange>
{
public:
  ProjectMaterialProperties(bool refine,
                            FEProblem & fe_problem, NonlinearSystem & sys, std::vector<MaterialData *> & material_data,
                            std::vector<MaterialData *> & bmd_material_data, MaterialPropertyStorage & material_props,
                            MaterialPropertyStorage & bnd_material_props, std::vector<MaterialWarehouse> & materials,
                            std::vector<Assembly *> & assembly);

  // Splitting Constructor
  ProjectMaterialProperties(ProjectMaterialProperties & x, Threads::split split);

  virtual ~ProjectMaterialProperties();

  virtual void subdomainChanged();
  virtual void onElement(const Elem *elem);
  virtual void onBoundary(const Elem *elem, unsigned int side, BoundaryID bnd_id);
  virtual void onInternalSide(const Elem *elem, unsigned int side);

  void join(const ProjectMaterialProperties & /*y*/);

protected:
  /// Whether or not you are projecting refinements.  Set to false for coarsening.
  bool _refine;
  FEProblem & _fe_problem;
  NonlinearSystem & _sys;
  std::vector<MaterialData *> & _material_data;
  std::vector<MaterialData *> & _bnd_material_data;
  MaterialPropertyStorage & _material_props;
  MaterialPropertyStorage & _bnd_material_props;
  std::vector<MaterialWarehouse> & _materials;
  std::vector<Assembly *> & _assembly;
};

#endif //PROJECTMATERIALPROPERTIES_H
