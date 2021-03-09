#pragma once

/** ElkEnums contains various enumerations useful in ELK, such as real/imag component definitions in
 * Kernels, BCs, etc.
 */

namespace elk
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
} // namespace elk
