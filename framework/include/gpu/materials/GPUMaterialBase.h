//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUTypes.h"
#include "GPUMaterialPropertyValue.h"

#include "MaterialBase.h"

class GPUMaterialBase : public MaterialBase,
                        public GPUMeshHolder,
                        public GPUAssemblyHolder,
                        public GPUSystemHolder
{
public:
  static InputParameters validParams();

  GPUMaterialBase(const InputParameters & parameters);
  GPUMaterialBase(const GPUMaterialBase & object);

  virtual void initialSetup() override;

  // Unused for GPUs
  virtual void subdomainSetup() override final {}

  // GPU function tags
  struct ElementInit
  {
  };
  struct SideInit
  {
  };
  struct NeighborInit
  {
  };
  struct ElementCompute
  {
  };
  struct SideCompute
  {
  };
  struct NeighborCompute
  {
  };

protected:
  // Declare GPU material property
  template <typename T, unsigned int dimension = 0>
  GPUMaterialProperty<T, dimension> declareGPUProperty(const std::string & name,
                                                       const std::vector<unsigned int> dims = {})
  {
    std::string prop_name = name;
    if (_pars.have_parameter<MaterialPropertyName>(name))
      prop_name = _pars.get<MaterialPropertyName>(name);

    return declareGPUPropertyByName<T, dimension>(prop_name, dims);
  }
  template <typename T, unsigned int dimension = 0>
  GPUMaterialProperty<T, dimension>
  declareGPUPropertyByName(const std::string & prop_name, const std::vector<unsigned int> dims = {})
  {
    return declareGPUPropertyInternal<T, dimension>(prop_name, dims);
  }

private:
  template <typename T, unsigned int dimension>
  GPUMaterialProperty<T, dimension>
  declareGPUPropertyInternal(const std::string & prop_name,
                             const std::vector<unsigned int> dims = {})
  {
    static_assert(dimension <= 4, "Up to four-dimensional GPU material properties are allowed.");

    if (dims.size() != dimension)
      mooseError("The declared GPU material property '",
                 prop_name,
                 "'\nhas a different dimension (",
                 dimension,
                 ") with the provided dimension (",
                 dims.size(),
                 ").");

    const auto prop_name_modified =
        _declare_suffix.empty()
            ? prop_name
            : MooseUtils::join(std::vector<std::string>({prop_name, _declare_suffix}), "_");

    auto prop = materialData().declareGPUProperty<T, dimension>(
        prop_name_modified, dims, *this, isBoundaryMaterial());

    registerPropName(prop_name_modified, false, 0);

    return prop;
  }

private:
  // List of local element IDs this GPU material is operating on for element material property
  // evaluation
  GPUArray<dof_id_type> _element_ids;
  // List of local element ID - side index pairs this GPU material is operating on for face material
  // property evaluation
  GPUArray<GPUPair<dof_id_type, unsigned int>> _element_side_ids;

protected:
  // Get the number of local elements this GPU material is operating on for element material
  // property evaluation
  KOKKOS_FUNCTION auto numElements() const { return _element_ids.size(); }
  // Get the number of local sides this GPU material is operating on for face material property
  // evaluation
  KOKKOS_FUNCTION auto numElementSides() const { return _element_side_ids.size(); }
  /// Get the local element ID this GPU thread is operating on
  KOKKOS_FUNCTION auto elementID(size_t idx) const { return _element_ids[idx]; }
  // Get the local element ID - side index pair this GPU thread is operating on
  KOKKOS_FUNCTION auto elementSideID(size_t idx) const { return _element_side_ids[idx]; }

protected:
  // TODO: Move to TransientInterface
  // Time
  GPUScalar<Real> _t;
  // Old time
  GPUScalar<const Real> _t_old;
  // The number of the time step
  GPUScalar<int> _t_step;
  // Time step size
  GPUScalar<Real> _dt;
  // Size of the old time step
  GPUScalar<Real> _dt_old;

protected:
  // Sets the variables this object depend on
  void setVariableDependency();
  // Sets the quadrature projection status flags for the variables, tags, and subdomains covered by
  // this object
  void setProjectionFlags();

protected:
  // Get the material data type
  virtual Moose::MaterialDataType materialDataType() = 0;
};
