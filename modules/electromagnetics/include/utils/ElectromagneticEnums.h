#pragma once

/** ElectromagneticEnums contains various enumerations useful in the
 * Electromagnetic module, such as real/imag component definitions in
 * Kernels, BCs, etc.
 */

namespace electromagnetics
{
enum ComponentEnum
{
  REAL,
  IMAGINARY
};
enum RobinEnum
{
  ABSORBING,
  PORT
};
} // namespace electromagnetics
