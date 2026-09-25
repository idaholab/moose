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
  params.addParam<std::string>(
      "neml2_input_derivative",
      "If supplied, read the derivative of the NEML2 output variable with respect to this instead of "
      "the variable itself");
  return params;
}

template <typename T>
KokkosNEML2ToMOOSEMaterialProperty<T>::KokkosNEML2ToMOOSEMaterialProperty(
    const InputParameters & parameters)
  : Moose::Kokkos::Material(parameters),
    _execute_neml2_model(getUserObject<NEML2ModelExecutor>("neml2_executor")),
    _value(
        isParamValid("neml2_input_derivative")
            ? _execute_neml2_model.getOutputDerivative(
                  neml2::VariableName(getParam<std::string>("from_neml2")),
                  neml2::VariableName(getParam<std::string>("neml2_input_derivative")))
            : _execute_neml2_model.getOutput(
                  neml2::VariableName(getParam<std::string>("from_neml2")))),
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

  if (_value.dynamic_dim() != 1)
    mooseError("The NEML2 output '",
               getParam<std::string>("from_neml2"),
               "' has ",
               _value.dynamic_dim(),
               " batch dimensions, and this material reads a flat batch. An input that supplies the "
               "model through NEML2FEInterpolation produces a batch shaped per element and per "
               "quadrature point, which this material cannot index.");

  // A derivative that does not vary along the batch arrives as a broadcast: one set of components with
  // a batch stride of zero. Compacting it would duplicate that set once per batch entry, so it is
  // aliased as it is and every quadrature point reads entry zero. A strided view that is not a
  // broadcast, such as a state variable sliced out of the model's state axis, is compacted instead,
  // which copies only the values actually needed.
  _broadcast = isBatchBroadcast();

  if (!_broadcast && !_value.is_contiguous())
    _contiguous = _value.contiguous();

  const auto & tensor = (_broadcast || _value.is_contiguous()) ? _value : _contiguous;

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

  if (static_cast<dof_id_type>(_value.size(0)) != _layout.size())
    mooseError("The NEML2 output '",
               getParam<std::string>("from_neml2"),
               "' has ",
               _value.size(0),
               " batch entries, but this material covers ",
               _layout.size(),
               " quadrature points. The NEML2 model and this material must be restricted to the "
               "same blocks.");

  // Storing one value per subdomain or per element only reproduces the output if the output is the
  // same for every quadrature point, which is exactly what a broadcast is
  if (!_broadcast)
    for (const auto block : blockIDs())
      if (_prop.constantOption(block) != Moose::Kokkos::PropertyConstantOption::NONE)
        mooseError("This material stores '",
                   getParam<MaterialPropertyName>("to_moose"),
                   "' more coarsely than per quadrature point, but the NEML2 output '",
                   getParam<std::string>("from_neml2"),
                   "' varies along the batch, so the stored value would be one arbitrary quadrature "
                   "point's. Leave constant_on at NONE unless the output is a quantity NEML2 "
                   "broadcasts, such as the tangent of a model whose tangent does not vary.");

  // A broadcast aliases one set of components rather than one per batch entry
  const auto n_reals =
      _broadcast ? _n_components : static_cast<dof_id_type>(tensor.numel());

  if (!_source.isAlloc())
    _source.init(n_reals);

  // NEML2 reassigns its output tensors on every solve, so the pointer is refreshed each time
  _source.aliasDevice(const_cast<Real *>(tensor.template data_ptr<Real>()));

  Moose::Kokkos::Material::computeProperties();
}

template <typename T>
bool
KokkosNEML2ToMOOSEMaterialProperty<T>::isBatchBroadcast() const
{
  if (_value.size(0) < 2 || _value.stride(0) != 0)
    return false;

  // Only the batch dimension may be broadcast; the components of one entry must be packed so that
  // they can be read as a contiguous block
  int64_t packed = 1;

  for (int64_t d = _value.dim() - 1; d >= 1; --d)
  {
    if (_value.stride(d) != packed)
      mooseError("The NEML2 output '",
                 getParam<std::string>("from_neml2"),
                 "' does not vary along the batch but its components are not packed, so neither "
                 "aliasing nor compacting it would read the right values.");

    packed *= _value.size(d);
  }

  if (packed != static_cast<int64_t>(_n_components))
    mooseError("The NEML2 output '",
               getParam<std::string>("from_neml2"),
               "' has ",
               packed,
               " components per batch entry, but the material property it is read into holds ",
               _n_components,
               ".");

  return true;
}

template class KokkosNEML2ToMOOSEMaterialProperty<Real>;
template class KokkosNEML2ToMOOSEMaterialProperty<Moose::Kokkos::Real6>;
template class KokkosNEML2ToMOOSEMaterialProperty<Moose::Kokkos::Real66>;

#endif
