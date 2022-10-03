//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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
                               std::vector<std::vector<std::unique_ptr<Assembly>>> & assembly);

  // Splitting Constructor
  ComputeMaterialsObjectThread(ComputeMaterialsObjectThread & x, Threads::split split);

  virtual void post() override;
  virtual void subdomainChanged() override;
  virtual void onElement(const Elem * elem) override;
  virtual void onBoundary(const Elem * elem,
                          unsigned int side,
                          BoundaryID bnd_id,
                          const Elem * lower_d_elem = nullptr) override;
  virtual void onInternalSide(const Elem * elem, unsigned int side) override;
  virtual void onInterface(const Elem * elem, unsigned int side, BoundaryID bnd_id) override;

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

  /// This is populated using _fe_problem.getResidualMaterialsWarehouse because it has the union
  /// of traditional materials and the residual version of AD materials. We don't need the Jacobian
  /// version of the ADMaterial for doing stateful stuff
  const MaterialWarehouse & _materials;

  /// This is populated using _fe_problem.getResidualInterfaceMaterialsWarehouse because it has the
  /// union of traditional interface materials and the residual version of AD interface
  /// materials. We don't need the Jacobian version of the ADInterfaceMaterial for doing stateful
  /// stuff
  const MaterialWarehouse & _interface_materials;

  const MaterialWarehouse & _discrete_materials;

  std::vector<std::vector<std::unique_ptr<Assembly>>> & _assembly;
  bool _need_internal_side_material;

  const bool _has_stateful_props;
  const bool _has_bnd_stateful_props;
  const bool _has_neighbor_stateful_props;
};
