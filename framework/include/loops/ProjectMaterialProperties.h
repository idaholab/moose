//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ThreadedElementLoop.h"

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
                            std::vector<std::shared_ptr<MaterialData>> & material_data,
                            std::vector<std::shared_ptr<MaterialData>> & bnd_material_data,
                            MaterialPropertyStorage & material_props,
                            MaterialPropertyStorage & bnd_material_props,
                            std::vector<std::vector<std::unique_ptr<Assembly>>> & assembly);

  // Splitting Constructor
  ProjectMaterialProperties(ProjectMaterialProperties & x, Threads::split split);

  virtual ~ProjectMaterialProperties();

  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void onBoundary(const Elem * elem,
                          unsigned int side,
                          BoundaryID bnd_id,
                          const Elem * lower_d_elem = nullptr) override;
  virtual void onInternalSide(const Elem * elem, unsigned int side) override;

  void join(const ProjectMaterialProperties & /*y*/);

protected:
  /// Whether or not you are projecting refinements.  Set to false for coarsening.
  bool _refine;
  FEProblemBase & _fe_problem;
  std::vector<std::shared_ptr<MaterialData>> & _material_data;
  std::vector<std::shared_ptr<MaterialData>> & _bnd_material_data;
  MaterialPropertyStorage & _material_props;
  MaterialPropertyStorage & _bnd_material_props;
  std::vector<std::vector<std::unique_ptr<Assembly>>> & _assembly;
  bool _need_internal_side_material;

  /// Materials warehouse
  const MaterialWarehouse & _materials;
  /// Discrete materials warehouse
  const MaterialWarehouse & _discrete_materials;

private:
  bool shouldComputeInternalSide(const Elem & /*elem*/, const Elem & /*neighbor*/) const override;
};

inline bool
ProjectMaterialProperties::shouldComputeInternalSide(const Elem & /*elem*/,
                                                     const Elem & /*neighbor*/) const
{
  return true;
}
