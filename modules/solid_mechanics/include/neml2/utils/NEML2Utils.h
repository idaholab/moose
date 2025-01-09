//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef NEML2_ENABLED

#include "neml2/misc/parser_utils.h"
#include "neml2/tensors/tensors.h"
#include "neml2/models/LabeledAxisAccessor.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"
#include "MaterialProperty.h"

#endif

#include "InputParameters.h"

class MooseObject;
class Action;
class SubProblem;

namespace NEML2Utils
{

#ifdef NEML2_ENABLED
/// Assert that the NEML2 variable name sits on either the forces or the state subaxis
void assertVariable(const neml2::VariableName &);

/// Assert that the NEML2 variable name sits on either the old_forces or the old_state subaxis
void assertOldVariable(const neml2::VariableName &);

/// Parse a raw string into NEML2 variable name
neml2::VariableName parseVariableName(const std::string &);

template <typename T>
struct Layout
{
};
template <>
struct Layout<Real>
{
  static constexpr std::array<neml2::Size, 0> shape{};
  static constexpr std::array<neml2::Size, 1> strides{1};
};
template <>
struct Layout<RealVectorValue>
{
  static constexpr std::array<neml2::Size, 1> shape{3};
  static constexpr std::array<neml2::Size, 2> strides{3, 1};
};
template <>
struct Layout<RankTwoTensor>
{
  static constexpr std::array<neml2::Size, 2> shape{3, 3};
  static constexpr std::array<neml2::Size, 3> strides{9, 3, 1};
};
template <>
struct Layout<SymmetricRankTwoTensor>
{
  static constexpr std::array<neml2::Size, 1> shape{6};
  static constexpr std::array<neml2::Size, 2> strides{6, 1};
};
template <>
struct Layout<RankFourTensor>
{
  static constexpr std::array<neml2::Size, 4> shape{3, 3, 3, 3};
  static constexpr std::array<neml2::Size, 5> strides{81, 27, 9, 3, 1};
};
template <>
struct Layout<SymmetricRankFourTensor>
{
  static constexpr std::array<neml2::Size, 2> shape{6, 6};
  static constexpr std::array<neml2::Size, 3> strides{36, 6, 1};
};

/**
 * @brief Map from std::vector<T> to neml2::Tensor without copying the data
 *
 * This method is used in gatherers which gather data from MOOSE as input variables to the NEML2
 * material model. So in theory, we only need to provide Layout specializations for MOOSE types that
 * can potentially be used as NEML2 input variables.
 *
 * For this method to work, the underlying data in \p data must be reinterpretable as Real
 * (torch::kFloat64). The data class T must also be aligned and follow the striding implied by
 * Layout<T>::shape. The data class T must also have no padding or overhead.
 */
template <typename T>
neml2::Tensor
fromBlob(const std::vector<T> & data)
{
  // The const_cast is fine because torch works with non-const ptr so that it can optionally handle
  // deallocation. But we are not going to let torch do that.
  const auto torch_tensor =
      torch::from_blob(const_cast<T *>(data.data()),
                       neml2::utils::add_shapes(data.size(), Layout<T>::shape),
                       torch::TensorOptions().dtype(torch::kFloat64));
  return neml2::Tensor(torch_tensor, 1);
}

/**
 * @brief Directly copy a contiguous chunk of memory of a torch::Tensor to a MOOSE data of type T
 *
 * This assumes the torch::Tensor and T have the same layout, for example both row-major
 * with T = RankTwoTensor. If the layouts are different, we may need to reshape/reorder/transpose
 * the torch::Tensor before copying.
 *
 * For this method to work,
 * 1. the address of \p dest must align with the first element of the data,
 * 2. the number of elements in \p dest must match the number of elements in \p src,
 * 3. the \p src tensor must be of type torch::kFloat64, and
 * 4. data in \p dest must be reinterpretable as Real.
 */
template <typename T>
void
copyTensorToMOOSEData(const torch::Tensor & src, T & dest)
{
  if (src.dtype() != torch::kFloat64)
    mooseError(
        "Cannot copy torch::Tensor with dtype ", src.dtype(), " into ", demangle(typeid(T).name()));
  if (src.numel() != Layout<T>::strides[0])
    mooseError("Cannot copy torch::Tensor with shape ",
               src.sizes(),
               " into ",
               demangle(typeid(T).name()),
               " with different number of elements.");
  auto dest_tensor = torch::from_blob(reinterpret_cast<Real *>(&dest),
                                      Layout<T>::shape,
                                      torch::TensorOptions().dtype(torch::kFloat64));
  dest_tensor.copy_(src.reshape(Layout<T>::shape));
}

static std::string NEML2_help_message = R""""(
==============================================================================
To debug NEML2 related issues:
1. Build and run MOOSE in dbg mode.
2. Re-run the simulation using the dbg executable, and often times
   NEML2 will provide a more helpful error message.
3. If the error message is not helpful, or if there is still no error message,
   run the simulation through a debugger: See
   https://mooseframework.inl.gov/application_development/debugging.html
4. If the issue is due to a NEML2 bug, feel free to report it at
   https://github.com/applied-material-modeling/neml2/issues
==============================================================================
)"""";

#endif // NEML2_ENABLED

/// Determine whether the NEML2 material model should be evaluated
bool shouldCompute(const SubProblem &);

/**
 * Augment docstring if NEML2 is not enabled
 */
std::string docstring(const std::string & desc);

/**
 * Assert that NEML2 is enabled. A MooseError is raised if NEML2 is not enabled.
 */
void assertNEML2Enabled();

} // namespace NEML2Utils
