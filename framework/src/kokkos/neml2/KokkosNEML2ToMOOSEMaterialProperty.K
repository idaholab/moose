//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosNEML2ToMOOSEMaterialProperty.h"

#ifdef NEML2_ENABLED

#include "NEML2ModelExecutor.h"
#include "NEML2Utils.h"

registerKokkosMaterial("MooseApp", KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty);
registerKokkosMaterial("MooseApp", KokkosNEML2ToMOOSESymmetricRankFourTensorMaterialProperty);

template <typename T>
InputParameters
KokkosNEML2ToMOOSEMaterialProperty<T>::validParams()
{
  InputParameters params = Moose::Kokkos::Material::validParams();
  params.addClassDescription("Copy a NEML2 output into a Kokkos material property on the device, "
                             "without transferring it through host memory.");
  params.addRequiredParam<UserObjectName>("neml2_executor",
                                          "User object managing the execution of the NEML2 model");
  params.addRequiredParam<MaterialPropertyName>("to_moose", "Kokkos material property to write to");
  params.addRequiredParam<std::string>("from_neml2", "NEML2 output variable to read from");
  return params;
}

template <typename T>
KokkosNEML2ToMOOSEMaterialProperty<T>::KokkosNEML2ToMOOSEMaterialProperty(
    const InputParameters & parameters)
  : Moose::Kokkos::Material(parameters),
    _execute_neml2_model(getUserObject<NEML2ModelExecutor>("neml2_executor")),
    _value(
        _execute_neml2_model.getOutput(neml2::VariableName(getParam<std::string>("from_neml2")))),
    _prop(declareKokkosProperty<T>("to_moose"))
{
}

template <typename T>
void
KokkosNEML2ToMOOSEMaterialProperty<T>::computeProperties()
{
  if (!_execute_neml2_model.outputReady())
    return;

  const auto & tensor = _value;

  if (!tensor.is_contiguous())
    mooseError("The NEML2 output '",
               getParam<std::string>("from_neml2"),
               "' is not contiguous, so its storage cannot be aliased. Reading it through the "
               "wrong strides would silently produce incorrect values.");

  if (!tensor.device().is_cuda())
    mooseError("The NEML2 output '",
               getParam<std::string>("from_neml2"),
               "' is not resident on the device. Set the NEML2 'device' and 'output_device' to a "
               "CUDA device so that this material can read it without a host transfer.");

  const auto n_reals = static_cast<dof_id_type>(tensor.numel());

  if (!_source.isAlloc())
    _source.init(n_reals);

  // NEML2 reassigns its output tensors on every solve, so the pointer is refreshed each time
  _source.aliasDevice(const_cast<Real *>(tensor.template data_ptr<Real>()));

  // The batch offsets depend only on the mesh and the model's element coverage
  if (!_offsets_current)
  {
    _batch_offset.create(kokkosMesh().getNumLocalElements());

    for (const auto * elem : _mesh.getMesh().active_local_element_ptr_range())
      if (hasBlocks(elem->subdomain_id()))
        _batch_offset[kokkosMesh().getContiguousElementID(elem)] =
            _execute_neml2_model.getBatchIndex(elem->id());

    _batch_offset.copyToDevice();
    _offsets_current = true;
  }

  Moose::Kokkos::Material::computeProperties();
}

template class KokkosNEML2ToMOOSEMaterialProperty<Moose::Kokkos::Real6>;
template class KokkosNEML2ToMOOSEMaterialProperty<Moose::Kokkos::Real66>;

#endif
