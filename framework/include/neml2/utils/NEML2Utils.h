//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <algorithm>

#ifdef NEML2_ENABLED

// NEML2 v3's C++ API exchanges plain at::Tensor (libtorch/ATen); the rich neml2::Tensor /
// typed-tensor library is no longer shipped, so we work with ATen directly here.
#include <ATen/ATen.h>
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"
#include "MaterialProperty.h"

#endif

class MooseObject;
class Action;
class SubProblem;

namespace NEML2Utils
{
#ifdef NEML2_ENABLED

enum class MOOSEIOType
{
  // unbatched
  TIME,
  SCALAR,
  // batched
  FUNCTION,
  VARIABLE,
  MATERIAL
};

std::string stringify(MOOSEIOType type);

/// Parse NEML2 v3 lag notation: "var~N" is the N-th old value of a variable (var~1 == old var,
/// var~2 == older, ...). Returns {base_name, lag}; lag == 0 when there is no "~N" suffix. Defined
/// here (shared, namespaced, inline) so the NEML2 unity build does not see duplicate definitions.
inline std::pair<std::string, int>
parseLag(const std::string & name)
{
  const auto pos = name.rfind('~');
  if (pos == std::string::npos)
    return {name, 0};
  const auto suffix = name.substr(pos + 1);
  if (suffix.empty() || suffix.find_first_not_of("0123456789") != std::string::npos)
    return {name, 0};
  return {name.substr(0, pos), std::stoi(suffix)};
}

/// Inverse of parseLag: build the variable name for a base name + lag.
inline std::string
lagName(const std::string & base, int lag)
{
  return lag == 0 ? base : base + "~" + std::to_string(lag);
}

/// Whether a vector of strings contains a given string.
inline bool
contains(const std::vector<std::string> & v, const std::string & s)
{
  return std::find(v.begin(), v.end(), s) != v.end();
}

template <typename T>
struct Layout
{
};
template <>
struct Layout<Real>
{
  static constexpr std::array<int64_t, 0> shape{};
  static constexpr std::array<int64_t, 1> strides{1};
};
template <>
struct Layout<RealVectorValue>
{
  static constexpr std::array<int64_t, 1> shape{3};
  static constexpr std::array<int64_t, 2> strides{3, 1};
};
template <>
struct Layout<RankTwoTensor>
{
  static constexpr std::array<int64_t, 2> shape{3, 3};
  static constexpr std::array<int64_t, 3> strides{9, 3, 1};
};
template <>
struct Layout<SymmetricRankTwoTensor>
{
  static constexpr std::array<int64_t, 1> shape{6};
  static constexpr std::array<int64_t, 2> strides{6, 1};
};
template <>
struct Layout<RankFourTensor>
{
  static constexpr std::array<int64_t, 4> shape{3, 3, 3, 3};
  static constexpr std::array<int64_t, 5> strides{81, 27, 9, 3, 1};
};
template <>
struct Layout<SymmetricRankFourTensor>
{
  static constexpr std::array<int64_t, 2> shape{6, 6};
  static constexpr std::array<int64_t, 3> strides{36, 6, 1};
};

/**
 * @brief Map from std::vector<T> to an at::Tensor without copying the data
 *
 * This method is used in gatherers which gather data from MOOSE as input variables to the NEML2
 * material model. So in theory, we only need to provide Layout specializations for MOOSE types that
 * can potentially be used as NEML2 input variables. The returned tensor has shape
 * (data.size(), *Layout<T>::shape), i.e. a leading batch axis followed by the variable's base shape,
 * which is the (*B, *base_shape) convention NEML2 v3's C++ API expects.
 *
 * For this method to work, the underlying data in \p data must be reinterpretable as Real
 * (at::kDouble). The data class T must also be aligned and follow the striding implied by
 * Layout<T>::shape. The data class T must also have no padding or overhead.
 */
template <typename T>
at::Tensor
fromBlob(const std::vector<T> & data)
{
  // Full shape: leading batch dim followed by the base shape.
  std::vector<int64_t> shape;
  shape.reserve(1 + Layout<T>::shape.size());
  shape.push_back(static_cast<int64_t>(data.size()));
  shape.insert(shape.end(), Layout<T>::shape.begin(), Layout<T>::shape.end());
  // The const_cast is fine because torch works with non-const ptr so that it can optionally handle
  // deallocation. But we are not going to let torch do that.
  return at::from_blob(
      const_cast<T *>(data.data()), shape, at::TensorOptions().dtype(at::kDouble));
}

/**
 * @brief Directly copy a contiguous chunk of memory of a at::Tensor to a MOOSE data of type T
 *
 * This assumes the at::Tensor and T have the same layout, for example both row-major
 * with T = RankTwoTensor. If the layouts are different, we may need to reshape/reorder/transpose
 * the at::Tensor before copying.
 *
 * For this method to work,
 * 1. the address of \p dest must align with the first element of the data,
 * 2. the number of elements in \p dest must match the number of elements in \p src,
 * 3. the \p src tensor must be of type at::kDouble, and
 * 4. data in \p dest must be reinterpretable as Real.
 */
template <typename T>
void
copyTensorToMOOSEData(const at::Tensor & src, T & dest)
{
  if (src.dtype() != at::kDouble)
    mooseError(
        "Cannot copy at::Tensor with dtype ", src.dtype(), " into ", demangle(typeid(T).name()));
  if (src.numel() != Layout<T>::strides[0])
    mooseError("Cannot copy at::Tensor with shape ",
               src.sizes(),
               " into ",
               demangle(typeid(T).name()),
               " with different number of elements.");
  auto dest_tensor = at::from_blob(reinterpret_cast<Real *>(&dest),
                                   Layout<T>::shape,
                                   at::TensorOptions().dtype(at::kDouble));
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
