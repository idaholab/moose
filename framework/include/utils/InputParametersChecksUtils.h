//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"
#include "Moose.h"

// C++ includes
#include <cstdlib>
#include <tuple>
#include <type_traits>

/**
 * Utility class to help check parameters.
 * This will be replaced by every check being baked into the validParams() logic, one day
 * @tparam C type of the class using this utility
 * C must be derived from both MooseBaseParameterInterface and MooseBaseErrorInterface
 */
template <typename C>
class InputParametersChecksUtils
{
public:
  InputParametersChecksUtils(const C * customer_class) : _customer_class(customer_class) {}

protected:
  /// Check in debug mode that this parameter has been added to the validParams
  /// @param param parameter that should be defined
  template <typename T>
  void assertParamDefined(const std::string & param) const;
  /// Check that two parameters are either both set or both not set
  /// @param param1 first parameter to check
  /// @param param2 second parameter to check
  void checkParamsBothSetOrNotSet(const std::string & param1, const std::string & param2) const;
  /// Check that a parameter is set only if the first one is set to true
  /// @param param1 first parameter to check, check the second if true
  /// @param param2 second parameter to check, that should be set if first one is true
  void checkSecondParamSetOnlyIfFirstOneTrue(const std::string & param1,
                                             const std::string & param2) const;
  /// Check that a parameter is set only if the first one is set
  /// @param param1 first parameter to check, check the second if set
  /// @param param2 second parameter to check, that should be set if first one is set
  void checkSecondParamSetOnlyIfFirstOneSet(const std::string & param1,
                                            const std::string & param2) const;
  /// Check that the two vector parameters are of the same length
  /// @param param1 first vector parameter to compare the size of
  /// @param param2 second vector parameter to compare the size of
  template <typename T, typename S>
  void checkVectorParamsSameLength(const std::string & param1, const std::string & param2) const;
  /// Check that this vector parameter (with name defined in \p param1) has the same length as the MultiMooseEnum (with name defined in \p param2)
  /// @param param1 vector parameter to compare the size of
  /// @param param2 multiMooseEnum parameter to compare the size of
  template <typename T>
  void checkVectorParamAndMultiMooseEnumLength(const std::string & param1,
                                               const std::string & param2) const;
  /// Check that the two-D vectors have exactly the same length in both dimensions
  /// @param param1 first two-D vector parameter to check the dimensions of
  /// @param param2 second two-D vector parameter to check the dimensions of
  template <typename T, typename S>
  void checkTwoDVectorParamsSameLength(const std::string & param1,
                                       const std::string & param2) const;
  /// Check that there is no overlap between the items in each vector parameters
  /// Each vector parameter should also have unique items
  /// @param param_vecs vector of parameters that should not overlap with each other
  template <typename T>
  void checkVectorParamsNoOverlap(const std::vector<std::string> & param_vecs) const;
  /// Check that there is no overlap between the respective items in each vector of the two-D parameters
  /// Each vector of the two-D vector parameter should also have unique items
  /// @param param_vecs vector of parameters that should not overlap with each other
  template <typename T>
  void checkTwoDVectorParamsNoRespectiveOverlap(const std::vector<std::string> & param_vecs) const;
  /// Check that each inner vector of a two-D vector parameter are the same size as another one-D vector parameter
  /// @param param1 two-D vector parameter to check the dimensions of
  /// @param param2 one-D vector parameter to set the desired size
  template <typename T, typename S>
  void checkTwoDVectorParamInnerSameLengthAsOneDVector(const std::string & param1,
                                                       const std::string & param2) const;
  /// Check that the size of a two-D vector parameter matches the size of a MultiMooseEnum parameter
  /// @param param1 two-D vector parameter to check the unrolled size of
  /// @param param2 MultiMooseEnum parameter to set the desired size
  template <typename T>
  void checkTwoDVectorParamMultiMooseEnumSameLength(const std::string & param1,
                                                    const std::string & param2,
                                                    const bool error_for_param2) const;
  /// Check that the user did not pass an empty vector
  /// @param param1 vector parameter that should not be empty
  template <typename T>
  void checkVectorParamNotEmpty(const std::string & param1) const;
  /// Check that two vector parameters are the same length if both are set
  /// @param param1 first vector parameter to check the size of
  /// @param param2 second vector parameter to check the size of
  template <typename T, typename S>
  void checkVectorParamsSameLengthIfSet(const std::string & param1,
                                        const std::string & param2,
                                        const bool ignore_empty_default_param2 = false) const;
  /// Check that a vector parameter is the same length as two others combined
  /// @param param1 vector parameter that provides the target size
  /// @param param2 vector parameter that provides one term in the combined size
  /// @param param3 vector parameter that provides one term in the combined size
  template <typename T, typename S, typename U>
  void checkVectorParamLengthSameAsCombinedOthers(const std::string & param1,
                                                  const std::string & param2,
                                                  const std::string & param3) const;

  /// Check if the user commited errors during the definition of block-wise parameters
  /// @param block_param_name the name of the parameter that provides the groups of blocks
  /// @param parameter_names vector of the names of the parameters that are defined on a per-block basis
  template <typename T>
  void checkBlockwiseConsistency(const std::string & block_param_name,
                                 const std::vector<std::string> & parameter_names) const;
  /// Return whether two parameters are consistent
  /// @param other_param InputParameters object from another object to check the 'param_name' parameter in
  /// @param param_name the name of the parameter to check for consistency
  template <typename T>
  bool parameterConsistent(const InputParameters & other_param,
                           const std::string & param_name) const;
  /// Emits a warning if two parameters are not equal to each other
  /// @param other_param InputParameters object from another object to check the 'param_name' parameter in
  /// @param param_name the name of the parameter to check for consistency
  template <typename T>
  void warnInconsistent(const InputParameters & parameters, const std::string & param_name) const;
  /// Error messages for parameters that should depend on another parameter
  /// @param param1 the parameter has not been set to the desired value (for logging purposes)
  /// @param value_not_set the desired value (for logging purposes)
  /// @param dependent_params all the parameters that should not have been since 'param1' was not set to 'value_not_set'
  void errorDependentParameter(const std::string & param1,
                               const std::string & value_not_set,
                               const std::vector<std::string> & dependent_params) const;
  /// Error messages for parameters that should depend on another parameter but with a different error message
  /// @param param1 the parameter has not been set to the desired value (for logging purposes)
  /// @param value_set the value it has been set to and which is not appropriate (for logging purposes)
  /// @param dependent_params all the parameters that should not have been since 'param1' was not set to 'value_not_set'
  void errorInconsistentDependentParameter(const std::string & param1,
                                           const std::string & value_set,
                                           const std::vector<std::string> & dependent_params) const;

private:
  // Convenience routines so that defining new checks feels very similar to coding checks in
  // MooseObjects and Actions (MooseParameterInterface-derived classes)
  /// Forwards parameter check to the class using this utility
  template <typename T>
  T forwardGetParam(const std::string & param_name) const
  {
    return _customer_class->template getParam<T>(param_name);
  }
  /// Forwards obtaining parameters to the class using this utility
  const InputParameters & forwardParameters() const { return _customer_class->parameters(); }
  /// Forwards parameter check to the class using this utility
  bool forwardIsParamSetByUser(const std::string & param_name) const
  {
    return _customer_class->isParamSetByUser(param_name);
  }
  /// Forwards parameter check to the class using this utility
  bool forwardIsParamValid(const std::string & param_name) const
  {
    return _customer_class->isParamValid(param_name);
  }
  /// Forwards error to the class using this utility to get better error messages
  template <typename... Args>
  void forwardParamError(Args &&... args) const
  {
    _customer_class->paramError(std::forward<Args>(args)...);
  }
  /// Forwards error to the class using this utility to get better error messages
  template <typename... Args>
  void forwardMooseError(Args &&... args) const
  {
    _customer_class->mooseError(std::forward<Args>(args)...);
  }
  /// Forwards warning to the class using this utility to get better error messages
  template <typename... Args>
  void forwardMooseWarning(Args &&... args) const
  {
    _customer_class->mooseWarning(std::forward<Args>(args)...);
  }
  /// Get the type of the class using this utility
  const std::string & forwardType() const { return _customer_class->type(); }
  /// Get the name of the class using this utility
  virtual const std::string & forwardName() const { return _customer_class->name(); }
  /// Get the blocks for the class using this utility
  const std::vector<SubdomainName> & forwardBlocks() const
  {
    // TODO Use SFINAE to create a version for classes that do not define blocks()
    // TODO Use SFINAE to force blocks() to return a reference as well
    return _customer_class->blocks();
  }

  // A pointer to the class using this
  const C * const _customer_class;
};

template <typename C>
template <typename T>
void
InputParametersChecksUtils<C>::assertParamDefined(const std::string & libmesh_dbg_var(param)) const
{
  mooseAssert(forwardParameters().template have_parameter<T>(param),
              "Parameter '" + param + "' is not defined with type '" +
                  MooseUtils::prettyCppType<T>() + "' in object type '" +
                  MooseUtils::prettyCppType(forwardType()) + "'. Check your code.");
}

template <typename C>
template <typename T, typename S>
void
InputParametersChecksUtils<C>::checkVectorParamsSameLength(const std::string & param1,
                                                           const std::string & param2) const
{
  assertParamDefined<std::vector<T>>(param1);
  assertParamDefined<std::vector<S>>(param2);

  if (forwardIsParamValid(param1) && forwardIsParamValid(param2))
  {
    const auto size_1 = forwardGetParam<std::vector<T>>(param1).size();
    const auto size_2 = forwardGetParam<std::vector<S>>(param2).size();
    if (size_1 != size_2)
      forwardParamError(param1,
                        "Vector parameters '" + param1 + "' (size " + std::to_string(size_1) +
                            ") and '" + param2 + "' (size " + std::to_string(size_2) +
                            ") must be the same size");
  }
  // handle empty vector defaults
  else if (forwardIsParamValid(param1) || forwardIsParamValid(param2))
    if (forwardGetParam<std::vector<T>>(param1).size() ||
        forwardGetParam<std::vector<T>>(param2).size())
      checkParamsBothSetOrNotSet(param1, param2);
}

template <typename C>
template <typename T>
void
InputParametersChecksUtils<C>::checkVectorParamAndMultiMooseEnumLength(
    const std::string & param1, const std::string & param2) const
{
  assertParamDefined<std::vector<T>>(param1);
  assertParamDefined<MultiMooseEnum>(param2);

  if (forwardIsParamValid(param1) && forwardIsParamValid(param2))
  {
    const auto size_1 = forwardGetParam<std::vector<T>>(param1).size();
    const auto size_2 = forwardGetParam<MultiMooseEnum>(param2).size();
    if (size_1 != size_2)
      forwardParamError(param1,
                        "Vector parameters '" + param1 + "' (size " + std::to_string(size_1) +
                            ") and '" + param2 + "' (size " + std::to_string(size_2) +
                            ") must be the same size");
  }
  // handle empty vector defaults
  else if (forwardIsParamValid(param1) || forwardIsParamValid(param2))
    if (forwardGetParam<std::vector<T>>(param1).size() ||
        forwardGetParam<MultiMooseEnum>(param2).size())
      checkParamsBothSetOrNotSet(param1, param2);
}

template <typename C>
template <typename T, typename S>
void
InputParametersChecksUtils<C>::checkTwoDVectorParamsSameLength(const std::string & param1,
                                                               const std::string & param2) const
{
  checkVectorParamsSameLength<std::vector<T>, std::vector<S>>(param1, param2);
  if (forwardIsParamValid(param1) && forwardIsParamValid(param2))
  {
    const auto value1 = forwardGetParam<std::vector<std::vector<T>>>(param1);
    const auto value2 = forwardGetParam<std::vector<std::vector<S>>>(param2);
    for (const auto index : index_range(value1))
      if (value1[index].size() != value2[index].size())
        forwardParamError(
            param1,
            "Vector at index " + std::to_string(index) + " of 2D vector parameter '" + param1 +
                "' is not the same size as its counterpart from 2D vector parameter '" + param2 +
                "'");
  }
  // handle empty vector defaults
  else if (forwardIsParamValid(param1) || forwardIsParamValid(param2))
    if (forwardGetParam<std::vector<T>>(param1).size() ||
        forwardGetParam<std::vector<T>>(param2).size())
      checkParamsBothSetOrNotSet(param1, param2);
}

template <typename C>
template <typename T, typename S>
void
InputParametersChecksUtils<C>::checkTwoDVectorParamInnerSameLengthAsOneDVector(
    const std::string & param1, const std::string & param2) const
{
  assertParamDefined<std::vector<std::vector<T>>>(param1);
  assertParamDefined<std::vector<S>>(param2);
  for (const auto & sub_vec_i : index_range(forwardGetParam<std::vector<std::vector<T>>>(param1)))
  {
    const auto size_1 = forwardGetParam<std::vector<std::vector<T>>>(param1)[sub_vec_i].size();
    const auto size_2 = forwardGetParam<std::vector<S>>(param2).size();
    if (size_1 != size_2)
      forwardParamError(param1,
                        "Vector at index " + std::to_string(sub_vec_i) + " (size " +
                            std::to_string(size_1) +
                            ") "
                            " of this parameter should be the same length as parameter '" +
                            param2 + "' (size " + std::to_string(size_2) + ")");
  }
}

template <typename C>
template <typename T>
void
InputParametersChecksUtils<C>::checkTwoDVectorParamMultiMooseEnumSameLength(
    const std::string & param1, const std::string & param2, const bool error_for_param2) const
{
  assertParamDefined<std::vector<std::vector<T>>>(param1);
  assertParamDefined<MultiMooseEnum>(param2);
  const auto vec1 = forwardGetParam<std::vector<std::vector<T>>>(param1);
  const auto enum2 = forwardGetParam<MultiMooseEnum>(param2);
  const auto size_1 = vec1.empty() ? 0 : vec1.size() * vec1[0].size();
  const auto size_2 = enum2.size();
  if (size_1 != size_2)
  {
    if (error_for_param2)
      forwardParamError(param2,
                        "Vector enumeration parameter (size " + std::to_string(size_2) +
                            ") is not the same size as the vector of vector parameter '" + param1 +
                            "' (size " + std::to_string(size_1) + ")");
    else
      forwardParamError(param1,
                        "Vector of vector parameter '" + param1 + "' (total size " +
                            std::to_string(size_1) +
                            ") is not the same size as vector-enumeration parameter '" + param2 +
                            "' (size " + std::to_string(size_2) + ")");
  }
}

template <typename C>
template <typename T, typename S, typename U>
void
InputParametersChecksUtils<C>::checkVectorParamLengthSameAsCombinedOthers(
    const std::string & param1, const std::string & param2, const std::string & param3) const
{
  assertParamDefined<std::vector<T>>(param1);
  assertParamDefined<std::vector<S>>(param2);
  assertParamDefined<std::vector<U>>(param3);
  const auto size_1 = forwardGetParam<std::vector<T>>(param1).size();
  const auto size_2 = forwardGetParam<std::vector<S>>(param2).size();
  const auto size_3 = forwardGetParam<std::vector<U>>(param3).size();

  if (size_1 != size_2 + size_3)
    forwardParamError(param1,
                      "Vector parameter '" + param1 + "' (size " + std::to_string(size_1) +
                          ") should be the same size as parameter '" + param2 + "' and '" + param3 +
                          " combined (total size " + std::to_string(size_2 + size_3) + ")");
}

template <typename C>
template <typename T>
void
InputParametersChecksUtils<C>::checkVectorParamsNoOverlap(
    const std::vector<std::string> & param_vec) const
{
  std::set<std::string> unique_params;
  for (const auto & param : param_vec)
  {
    assertParamDefined<std::vector<T>>(param);

    for (const auto & value : forwardGetParam<std::vector<T>>(param))
      if (!unique_params.insert(value).second)
      {
        auto copy_params = param_vec;
        copy_params.erase(std::find(copy_params.begin(), copy_params.end(), param));
        forwardMooseError("Item '" + value + "' specified in vector parameter '" + param +
                          "' is also present in one or more of the parameters '" +
                          Moose::stringify(copy_params) + "', which is not allowed.");
      }
  }
}

template <typename C>
template <typename T>
void
InputParametersChecksUtils<C>::checkTwoDVectorParamsNoRespectiveOverlap(
    const std::vector<std::string> & param_vec) const
{
  // Outer loop, each param is the name of a parameter for a vector of vectors
  for (const auto & param : param_vec)
  {
    assertParamDefined<std::vector<std::vector<T>>>(param);
    const auto & twoD_vec = forwardGetParam<std::vector<std::vector<T>>>(param);
    std::vector<std::set<T>> unique_params(twoD_vec.size());

    // Loop over each outer vector and compare the inner vectors respectively to other parameters
    for (const auto i : index_range(twoD_vec))
    {
      for (const auto & value : twoD_vec[i])
        if (!unique_params[i].insert(value).second)
        {
          auto copy_params = param_vec;
          copy_params.erase(std::find(copy_params.begin(), copy_params.end(), param));
          forwardMooseError("Item '" + value + "' specified in vector parameter '" + param +
                            "' is also present in one or more of the two-D vector parameters '" +
                            Moose::stringify(copy_params) +
                            "' in the inner vector of the same index, which is not allowed.");
        }
    }
  }
}

template <typename C>
template <typename T>
void
InputParametersChecksUtils<C>::checkVectorParamNotEmpty(const std::string & param) const
{
  assertParamDefined<std::vector<T>>(param);
  if (!forwardGetParam<std::vector<T>>(param).size())
    forwardParamError(param, "Parameter '" + param + "' should not be set to an empty vector.");
}

template <typename C>
template <typename T, typename S>
void
InputParametersChecksUtils<C>::checkVectorParamsSameLengthIfSet(
    const std::string & param1,
    const std::string & param2,
    const bool ignore_empty_default_param2) const
{
  assertParamDefined<std::vector<T>>(param1);
  assertParamDefined<std::vector<S>>(param2);

  if (forwardIsParamValid(param1) && forwardIsParamValid(param2))
  {
    const auto size_1 = forwardGetParam<std::vector<T>>(param1).size();
    const auto size_2 = forwardGetParam<std::vector<S>>(param2).size();
    if (ignore_empty_default_param2 && (size_2 == 0) && !forwardIsParamSetByUser(param2))
      return;
    if (size_1 != size_2)
      forwardParamError(param1,
                        "Parameter '" + param1 + "' (size " + std::to_string(size_1) + ") and '" +
                            param2 + "' (size " + std::to_string(size_2) +
                            ") must be the same size if set.");
  }
}

template <typename C>
template <typename T>
bool
InputParametersChecksUtils<C>::parameterConsistent(const InputParameters & other_param,
                                                   const std::string & param_name) const
{
  assertParamDefined<T>(param_name);
  mooseAssert(other_param.have_parameter<T>(param_name),
              "This should have been a parameter from the parameters being compared");
  bool consistent = true;
  if (forwardParameters().isParamValid(param_name) && other_param.isParamValid(param_name))
  {
    if constexpr (std::is_same_v<MooseEnum, T>)
    {
      if (!forwardGetParam<T>(param_name).compareCurrent(other_param.get<T>(param_name)))
        consistent = false;
    }
    else if (forwardGetParam<T>(param_name) != other_param.get<T>(param_name))
      consistent = false;
  }
  return consistent;
}

template <typename C>
template <typename T>
void
InputParametersChecksUtils<C>::checkBlockwiseConsistency(
    const std::string & block_param_name, const std::vector<std::string> & parameter_names) const
{
  const std::vector<std::vector<SubdomainName>> & block_names =
      forwardGetParam<std::vector<std::vector<SubdomainName>>>(block_param_name);

  if (block_names.size())
  {
    // We only check block-restrictions if the customer class is not restricted to `ANY_BLOCK_ID`.
    // If the users define blocks that are not on the mesh, they will receive errors from the
    // objects created by the customer class
    const auto & object_blocks = forwardBlocks();
    if (std::find(object_blocks.begin(), object_blocks.end(), "ANY_BLOCK_ID") ==
        object_blocks.end())
      for (const auto & block_group : block_names)
        for (const auto & block : block_group)
          if (std::find(object_blocks.begin(), object_blocks.end(), block) == object_blocks.end())
            forwardParamError(block_param_name,
                              "Block '" + block + "' is not present in the block restriction of " +
                                  forwardName() +
                                  "!\nBlock restriction: " + Moose::stringify(object_blocks));

    for (const auto & param_name : parameter_names)
    {
      const std::vector<T> & param_vector = forwardGetParam<std::vector<T>>(param_name);
      if (block_names.size() != param_vector.size())
        forwardParamError(param_name,
                          "The number of entries in '" + param_name + "' (" +
                              std::to_string(param_vector.size()) +
                              ") is not the same as the number of blocks"
                              " (" +
                              std::to_string(block_names.size()) + ") in '" + block_param_name +
                              "'!");
    }
  }
  else
  {
    unsigned int previous_size = 0;
    for (const auto param_i : index_range(parameter_names))
    {
      const std::vector<T> & param_vector =
          forwardGetParam<std::vector<T>>(parameter_names[param_i]);
      if (param_i == 0)
      {
        if (param_vector.size() > 1)
          forwardParamError(parameter_names[param_i],
                            "The user should only use one or zero entries in " +
                                parameter_names[param_i] + " if " + block_param_name +
                                " not defined!");
        previous_size = param_vector.size();
      }
      else
      {
        if (previous_size != param_vector.size())
          forwardParamError(parameter_names[param_i],
                            "The number of entries in '" + parameter_names[param_i] +
                                "' is not the same as the number of entries in '" +
                                parameter_names[param_i - 1] + "'!");
      }
    }
  }
}

template <typename C>
template <typename T>
void
InputParametersChecksUtils<C>::warnInconsistent(const InputParameters & other_param,
                                                const std::string & param_name) const
{
  const bool consistent = parameterConsistent<T>(other_param, param_name);
  if (!consistent)
    forwardMooseWarning("Parameter " + param_name + " is inconsistent between Physics \"" +
                        forwardName() + "\" of type \"" + forwardType() +
                        "\" and the parameter set for \"" +
                        other_param.get<std::string>("_action_name") + "\" of type \"" +
                        other_param.get<std::string>("action_type") + "\"");
}

template <typename C>
void
InputParametersChecksUtils<C>::errorDependentParameter(
    const std::string & param1,
    const std::string & value_not_set,
    const std::vector<std::string> & dependent_params) const
{
  for (const auto & dependent_param : dependent_params)
    if (forwardIsParamSetByUser(dependent_param))
      forwardParamError(dependent_param,
                        "Parameter '" + dependent_param +
                            "' should not be set by the user if parameter '" + param1 +
                            "' has not been set to '" + value_not_set + "'");
}

template <typename C>
void
InputParametersChecksUtils<C>::errorInconsistentDependentParameter(
    const std::string & param1,
    const std::string & value_set,
    const std::vector<std::string> & dependent_params) const
{
  for (const auto & dependent_param : dependent_params)
    if (forwardIsParamSetByUser(dependent_param))
      forwardParamError(dependent_param,
                        "Parameter '" + dependent_param +
                            "' should not be set by the user if parameter '" + param1 +
                            "' has been set to '" + value_set + "'");
}

template <typename C>
void
InputParametersChecksUtils<C>::checkParamsBothSetOrNotSet(const std::string & param1,
                                                          const std::string & param2) const
{
  if ((forwardIsParamValid(param1) + forwardIsParamValid(param2)) % 2 != 0)
    forwardParamError(param1,
                      "Parameters '" + param1 + "' and '" + param2 +
                          "' must be either both set or both not set.");
}

template <typename C>
void
InputParametersChecksUtils<C>::checkSecondParamSetOnlyIfFirstOneTrue(
    const std::string & param1, const std::string & param2) const
{
  mooseAssert(forwardParameters().template have_parameter<bool>(param1),
              "Cannot check if parameter " + param1 +
                  " is true if it's not a bool parameter of this object");
  if (!forwardGetParam<bool>(param1) && forwardIsParamSetByUser(param2))
    forwardParamError(param2,
                      "Parameter '" + param1 + "' cannot be set to false if parameter '" + param2 +
                          "' is set by the user");
}

template <typename C>
void
InputParametersChecksUtils<C>::checkSecondParamSetOnlyIfFirstOneSet(
    const std::string & param1, const std::string & param2) const
{
  if (!forwardIsParamSetByUser(param1) && forwardIsParamSetByUser(param2))
    forwardParamError(param2,
                      "Parameter '" + param2 + "' should not be set if parameter '" + param1 +
                          "' is not specified.");
}
