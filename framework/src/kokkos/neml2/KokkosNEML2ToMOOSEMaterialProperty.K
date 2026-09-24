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

registerKokkosMaterial("MooseApp", KokkosNEML2ToMOOSERealMaterialProperty);
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

  if (!_value.device().is_cuda())
    mooseError("The NEML2 output '",
               getParam<std::string>("from_neml2"),
               "' is not resident on the device. Set the NEML2 'device' and 'output_device' to a "
               "CUDA device so that this material can read it without a host transfer.");

  // A NEML2 output is a view whenever it is a slice of the model's state axis or a broadcast
  // produced by dynamic_expand(). Aliasing needs contiguous storage, so a view is compacted on the
  // device and the copy is held for as long as it is aliased.
  if (!_value.is_contiguous())
    _contiguous = _value.contiguous();

  const auto & tensor = _value.is_contiguous() ? _value : _contiguous;

  // The numbering depends only on the mesh and the model's block coverage
  if (!_layout.built())
  {
    // Subdomain-major numbering coincides with the element loop's on a single subdomain only, so
    // across several this material can read the output only if the inputs were gathered in the same
    // order it reads them
    if (blockIDs().size() > 1 && !_execute_neml2_model.batchOrderDiffersFromElementLoop())
      mooseError("This material covers ",
                 blockIDs().size(),
                 " subdomains and reads the NEML2 output in the Kokkos assembly's subdomain-major "
                 "order, but the model's inputs were gathered in the order the element loop visits "
                 "elements, which interleaves the subdomains. Reading one numbering as though it were "
                 "the other pairs each quadrature point with another point's result.\n\nGather every "
                 "batched input with the Kokkos gatherers so that one numbering applies throughout, or "
                 "retrieve the output with NEML2ToMOOSEMaterialProperty, which shares the element "
                 "loop's numbering.");

    _layout.build(kokkosAssembly(), kokkosMesh(), blockIDs());
  }

  const auto n_reals = static_cast<dof_id_type>(tensor.numel());

  if (n_reals != _layout.size() * _n_components)
    mooseError("The NEML2 output '",
               getParam<std::string>("from_neml2"),
               "' has ",
               n_reals / _n_components,
               " batch entries, but this material covers ",
               _layout.size(),
               " quadrature points. The NEML2 model and this material must be restricted to the "
               "same blocks.");

  if (!_source.isAlloc())
    _source.init(n_reals);

  // NEML2 reassigns its output tensors on every solve, so the pointer is refreshed each time
  _source.aliasDevice(const_cast<Real *>(tensor.template data_ptr<Real>()));

  Moose::Kokkos::Material::computeProperties();
}

template class KokkosNEML2ToMOOSEMaterialProperty<Real>;
template class KokkosNEML2ToMOOSEMaterialProperty<Moose::Kokkos::Real6>;
template class KokkosNEML2ToMOOSEMaterialProperty<Moose::Kokkos::Real66>;

#endif
