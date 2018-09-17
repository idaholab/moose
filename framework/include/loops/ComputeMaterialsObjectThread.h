//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEMATERIALOBJECTTHREAD_H
#define COMPUTEMATERIALOBJECTTHREAD_H

#include "ThreadedElementLoop.h"

#include "libmesh/elem_range.h"

// Forward declarations
class FEProblemBase;
class NonlinearSystemBase;
class MaterialPropertyStorage;
class MaterialData;
class Assembly;

class ComputeMaterialsObjectThread : public ThreadedElementLoop<ConstElemRange>
{
public:
  ComputeMaterialsObjectThread(FEProblemBase & fe_problem,
                               std::vector<std::shared_ptr<MaterialData>> & material_data,
                               std::vector<std::shared_ptr<MaterialData>> & bnd_material_data,
                               std::vector<std::shared_ptr<MaterialData>> & neighbor_material_data,
                               MaterialPropertyStorage & material_props,
                               MaterialPropertyStorage & bnd_material_props,
                               MaterialPropertyStorage & neighbor_material_props,
                               std::vector<std::unique_ptr<Assembly>> & assembly);

  // Splitting Constructor
  ComputeMaterialsObjectThread(ComputeMaterialsObjectThread & x, Threads::split split);

  virtual void post() override;
  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void onBoundary(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;
  virtual void onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;
  virtual void onInternalSide(const Elem * elem, unsigned int side) override;

  void join(const ComputeMaterialsObjectThread & /*y*/);

protected:
  FEProblemBase & _fe_problem;
  NonlinearSystemBase & _nl;
  std::vector<std::shared_ptr<MaterialData>> & _material_data;
  std::vector<std::shared_ptr<MaterialData>> & _bnd_material_data;
  std::vector<std::shared_ptr<MaterialData>> & _neighbor_material_data;
  MaterialPropertyStorage & _material_props;
  MaterialPropertyStorage & _bnd_material_props;
  MaterialPropertyStorage & _neighbor_material_props;

  /// Reference to the Material object warehouses
  const MaterialWarehouse & _materials;
  const MaterialWarehouse & _discrete_materials;

  std::vector<std::unique_ptr<Assembly>> & _assembly;
  bool _need_internal_side_material;

  const bool _has_stateful_props;
  const bool _has_bnd_stateful_props;
  const bool _has_neighbor_stateful_props;
};

#endif // COMPUTEMATERIALOBJECTTHREAD_H
