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

namespace Moose
{
namespace Kokkos
{

class MaterialBase : public ::MaterialBase,
                     public MeshHolder,
                     public AssemblyHolder,
                     public SystemHolder
{
public:
  static InputParameters validParams();

  MaterialBase(const InputParameters & parameters);
  MaterialBase(const MaterialBase & object);

  virtual void initialSetup() override;

  // Unused because all elements are computed in parallel
  virtual void subdomainSetup() override final {}

  /**
   * Kokkos function tags
   */
  ///@{
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
  ///@}

protected:
  // Declare Kokkos material property
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension> declareKokkosProperty(const std::string & name,
                                                       const std::vector<unsigned int> dims = {})
  {
    std::string prop_name = name;
    if (_pars.have_parameter<MaterialPropertyName>(name))
      prop_name = _pars.get<MaterialPropertyName>(name);

    return declareKokkosPropertyByName<T, dimension>(prop_name, dims);
  }
  template <typename T, unsigned int dimension = 0>
  MaterialProperty<T, dimension>
  declareKokkosPropertyByName(const std::string & prop_name,
                              const std::vector<unsigned int> dims = {})
  {
    return declareKokkosPropertyInternal<T, dimension>(prop_name, dims);
  }

private:
  template <typename T, unsigned int dimension>
  MaterialProperty<T, dimension>
  declareKokkosPropertyInternal(const std::string & prop_name,
                                const std::vector<unsigned int> dims = {})
  {
    static_assert(dimension <= 4, "Up to four-dimensional Kokkos material properties are allowed.");

    if (dims.size() != dimension)
      mooseError("The declared Kokkos material property '",
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

    auto prop = materialData().declareKokkosProperty<T, dimension>(
        prop_name_modified, dims, *this, isBoundaryMaterial());

    registerPropName(prop_name_modified, false, 0);

    return prop;
  }

private:
  // List of local element IDs this Kokkos material is operating on for element material property
  // evaluation
  Array<dof_id_type> _element_ids;
  // List of local element ID - side index pairs this Kokkos material is operating on for face
  // material property evaluation
  Array<Pair<dof_id_type, unsigned int>> _element_side_ids;

protected:
  // Get the number of local elements this Kokkos material is operating on for element material
  // property evaluation
  KOKKOS_FUNCTION auto numElements() const { return _element_ids.size(); }
  // Get the number of local sides this Kokkos material is operating on for face material property
  // evaluation
  KOKKOS_FUNCTION auto numElementSides() const { return _element_side_ids.size(); }
  /// Get the local element ID this Kokkos thread is operating on
  KOKKOS_FUNCTION auto elementID(size_t idx) const { return _element_ids[idx]; }
  // Get the local element ID - side index pair this Kokkos thread is operating on
  KOKKOS_FUNCTION auto elementSideID(size_t idx) const { return _element_side_ids[idx]; }

protected:
  // TODO: Move to TransientInterface
  // Time
  Scalar<Real> _t;
  // Old time
  Scalar<const Real> _t_old;
  // The number of the time step
  Scalar<int> _t_step;
  // Time step size
  Scalar<Real> _dt;
  // Size of the old time step
  Scalar<Real> _dt_old;

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

} // namespace Kokkos
} // namespace Moose
