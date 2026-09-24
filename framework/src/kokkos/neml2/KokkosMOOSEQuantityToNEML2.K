//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMOOSEQuantityToNEML2.h"

#ifdef NEML2_ENABLED

#include <ATen/ops/from_blob.h>

registerKokkosUserObject("MooseApp", KokkosMOOSERealToNEML2);
registerKokkosUserObject("MooseApp", KokkosMOOSESymmetricRankTwoTensorToNEML2);

template <typename T>
InputParameters
KokkosMOOSEQuantityToNEML2<T>::validParams()
{
  InputParameters params = Moose::Kokkos::ElementUserObject::validParams();
  params += MOOSEToNEML2::validParams();
  params.addClassDescription("Gather a Kokkos material property into a NEML2 input variable on the "
                             "device, without transferring it through host memory.");
  params.addRequiredParam<MaterialPropertyName>("from_moose",
                                                "Kokkos material property to read from");

  ExecFlagEnum execute_options = MooseUtils::getDefaultExecFlagEnum();
  execute_options = {EXEC_INITIAL, EXEC_LINEAR, EXEC_NONLINEAR};
  params.set<ExecFlagEnum>("execute_on") = execute_options;

  return params;
}

template <typename T>
KokkosMOOSEQuantityToNEML2<T>::KokkosMOOSEQuantityToNEML2(const InputParameters & parameters)
  : Moose::Kokkos::ElementUserObject(parameters),
    MOOSEToNEML2(parameters),
    _prop(this->template getKokkosMaterialProperty<T>("from_moose"))
{
}

template <typename T>
void
KokkosMOOSEQuantityToNEML2<T>::compute()
{
  // The numbering depends only on the mesh and this object's block coverage
  if (!_layout.built())
  {
    _layout.build(kokkosAssembly(), kokkosMesh(), blockIDs());
    _buffer.create(_layout.size() * _n_components);
  }

  Moose::Kokkos::ElementUserObject::compute();
}

template <typename T>
neml2::Tensor
KokkosMOOSEQuantityToNEML2<T>::gatheredData() const
{
  if (!_layout.built())
    mooseError("The gathered data was requested before this object computed it. The NEML2 executor "
               "must run after the Kokkos user objects it gathers from.");

  auto shape = neml2::TensorShape{static_cast<neml2::Size>(_layout.size())};
  for (const auto d : KokkosNEML2BaseShape<T>::shape())
    shape.push_back(d);

  // The buffer stays alive for the lifetime of this object, so the tensor can alias it rather than
  // copying. NEML2 only reads its inputs.
  return neml2::Tensor(at::from_blob(_buffer.deviceData(),
                                    shape,
                                    torch::TensorOptions()
                                        .dtype(torch::kFloat64)
                                        .device(_app.getLibtorchDevice())),
                       1);
}

template class KokkosMOOSEQuantityToNEML2<Real>;
template class KokkosMOOSEQuantityToNEML2<Moose::Kokkos::Real6>;

#endif
