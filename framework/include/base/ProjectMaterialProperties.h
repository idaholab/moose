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

// MOOSE includes
#include "ThreadedElementLoop.h"

// libMesh includes
#include "libmesh/elem_range.h"

class FEProblemBase;
class NonlinearSystemBase;
class MaterialPropertyStorage;
class MaterialData;
class Assembly;

class ProjectMaterialProperties : public ThreadedElementLoop<ConstElemPointerRange>
{
public:
  ProjectMaterialProperties(bool refine,
                            FEProblemBase & fe_problem,
                            NonlinearSystemBase & sys,
                            std::vector<std::shared_ptr<MaterialData>> & material_data,
                            std::vector<std::shared_ptr<MaterialData>> & bnd_material_data,
                            MaterialPropertyStorage & material_props,
                            MaterialPropertyStorage & bnd_material_props,
                            std::vector<Assembly *> & assembly);

  // Splitting Constructor
  ProjectMaterialProperties(ProjectMaterialProperties & x, Threads::split split);

  virtual ~ProjectMaterialProperties();

  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;
  virtual void onInternalSide(const Elem * elem, unsigned int side) override;

  void join(const ProjectMaterialProperties & /*y*/);

protected:
  /// Whether or not you are projecting refinements.  Set to false for coarsening.
  bool _refine;
  FEProblemBase & _fe_problem;
  NonlinearSystemBase & _sys;
  std::vector<std::shared_ptr<MaterialData>> & _material_data;
  std::vector<std::shared_ptr<MaterialData>> & _bnd_material_data;
  MaterialPropertyStorage & _material_props;
  MaterialPropertyStorage & _bnd_material_props;
  std::vector<Assembly *> & _assembly;
  bool _need_internal_side_material;
};

#endif // PROJECTMATERIALPROPERTIES_H
