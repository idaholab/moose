/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                       BlackBear                              */
/*                                                              */
/*           (c) 2017 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#pragma once

#include "Material.h"
#include "UserObject.h"

#ifdef NEML2_ENABLED
#include "neml2/models/Model.h"
#include "neml2/misc/parser_utils.h"
#else
#include "NEML2Utils.h"
#endif

/**
 * Interface class to provide common input parameters, members, and methods for MOOSEObjects that
 * use NEML2 models.
 */
template <class T>
class NEML2ModelInterface : public T
{
public:
  static InputParameters validParams();

  template <typename... P>
  NEML2ModelInterface(const InputParameters & params, P &&... args);

#ifdef NEML2_ENABLED
protected:
  /**
   * Validate the NEML2 material model. This method should throw a moose error with the first
   * encountered problem. Note that the developer is responsible for calling this method at the
   * appropriate times, for example, at initialSetup().
   */
  virtual void validateModel() const;

  /// Get the NEML2 model
  neml2::Model & model() const { return _model; }

  /// Get the target compute device
  const torch::Device & device() const { return _device; }

  /**
   * @brief Convert a raw string to a LabeledAxisAccessor
   *
   * @param raw_str
   * @return neml2::LabeledAxisAccessor
   */
  neml2::LabeledAxisAccessor getLabeledAxisAccessor(const std::string & raw_str) const;

private:
  /// The NEML2 material model
  neml2::Model & _model;

  /// The device on which to evaluate the NEML2 model
  const torch::Device _device;
#endif
};

template <class T>
InputParameters
NEML2ModelInterface<T>::validParams()
{
  InputParameters params = T::validParams();
  params.addRequiredParam<std::string>(
      "model",
      "Name of the NEML2 model, i.e., the string inside the brackets [] in the NEML2 input file "
      "that corresponds to the model you want to use.");
  params.addParam<std::string>(
      "device",
      "cpu",
      "Device on which to evaluate the NEML2 model. The string supplied must follow the following "
      "schema: (cpu|cuda)[:<device-index>] where cpu or cuda specifies the device type, and "
      ":<device-index> optionally specifies a device index. For example, device='cpu' sets the "
      "target compute device to be CPU, and device='cuda:1' sets the target compute device to be "
      "CUDA with device ID 1.");
  return params;
}

template <class T>
template <typename... P>
NEML2ModelInterface<T>::NEML2ModelInterface(const InputParameters & params, P &&... args)
  : T(params, args...)
#ifdef NEML2_ENABLED
    ,
    _model(neml2::Factory::get_object<neml2::Model>("Models", params.get<std::string>("model"))),
    _device(params.get<std::string>("device"))
#endif
{
#ifdef NEML2_ENABLED
  // Send the model to the compute device
  _model.to(_device);
#else
  NEML2Utils::requireNEML2(*this);
#endif
}

#ifdef NEML2_ENABLED
template <class T>
void
NEML2ModelInterface<T>::validateModel() const
{
  // Forces and old forces on the input axis must match, i.e. all the variables on the old_forces
  // subaxis must also exist on the forces subaxis:
  if (_model.input().has_subaxis("old_forces"))
    for (auto var : _model.input().subaxis("old_forces").variable_accessors(/*recursive=*/true))
      if (!_model.input().subaxis("forces").has_variable(var))
        mooseError("The NEML2 model has old force variable ",
                   var,
                   " as input, but does not have the corresponding force variable as input.");

  // Similarly, state (on the output axis) and old state (on the input axis) must match, i.e. all
  // the variables on the input's old_state subaxis must also exist on the output's state subaxis:
  if (_model.input().has_subaxis("old_state"))
    for (auto var : _model.input().subaxis("old_state").variable_accessors(/*recursive=*/true))
      if (!_model.output().subaxis("state").has_variable(var))
        mooseError("The NEML2 model has old state variable ",
                   var,
                   " as input, but does not have the corresponding state variable as output.");
}

template <class T>
neml2::LabeledAxisAccessor
NEML2ModelInterface<T>::getLabeledAxisAccessor(const std::string & raw_str) const
{
  return neml2::utils::parse<neml2::LabeledAxisAccessor>(raw_str);
}
#endif
