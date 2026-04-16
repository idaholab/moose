//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseError.h"

#include <cstdint>

/**
 * Generates a keyed pseudo-random permutation of the integers [0, n) using a
 * balanced Feistel network. Given the same seed and n, the mapping is fully
 * deterministic and bijective: every input in [0, n) maps to a unique output
 * in [0, n). Different seeds produce statistically independent permutations.
 *
 * Because n need not be a power of two, the Feistel network operates on a
 * padded domain of size 2^(2*half_bits) >= n and uses cycle-walking: if the
 * raw output falls outside [0, n) it is re-applied until a valid index is
 * reached. This guarantees termination because the padded permutation is
 * itself a bijection and n > 0 ensures at least one valid output exists.
 *
 * The permutation is also invertible via invert(), which runs the Feistel
 * rounds in reverse order.
 */
class MooseRandomPerturbation
{
public:
  /**
   * Construct a permutation of [0, \p n) keyed by \p seed.
   *
   * @param seed  64-bit seed; split into two 32-bit subkeys for the round function.
   * @param n     Size of the domain to permute; must be > 0.
   * @param rounds Number of Feistel rounds. More rounds improve mixing at the
   *               cost of throughput; 8 is sufficient for sampling applications.
   */
  MooseRandomPerturbation(uint64_t seed, unsigned int n, unsigned int rounds = 8)
    : _k0(static_cast<uint32_t>(seed)),
      _k1(static_cast<uint32_t>(seed >> 32)),
      _n(n),
      _half_bits((ceilLog2(n) + 1) / 2),
      _half_mask((uint32_t(1) << _half_bits) - 1),
      _rounds(rounds)
  {
    mooseAssert(_n > 0, "n must be > 0");
    mooseAssert(_rounds > 0, "rounds must be greater than 0.");
  }

  /**
   * Map \p x to its permuted index in [0, n).
   *
   * Applies the Feistel network to the padded domain and cycle-walks until
   * the result falls within [0, n).
   *
   * @param x  Input index; must satisfy x < n.
   * @return   A unique index in [0, n). Calling permute for every x in [0, n)
   *           yields each value in [0, n) exactly once.
   */
  uint32_t permute(uint32_t x) const
  {
    mooseAssert(x < _n, "x must be < n");

    uint32_t y = x;
    do
    {
      y = permutePadded(y);
    } while (y >= _n);

    return y;
  }

  /**
   * Recover the original index from a permuted value, i.e. invert(permute(x)) == x.
   *
   * Runs the Feistel rounds in reverse order and cycle-walks back into [0, n).
   *
   * @param y  Permuted index; must satisfy y < n.
   * @return   The unique x in [0, n) such that permute(x) == y.
   */
  uint32_t invert(uint32_t y) const
  {
    mooseAssert(y < _n, "y must be < n");

    uint32_t x = y;
    do
    {
      x = invertPadded(x);
    } while (x >= _n);

    return x;
  }

private:
  /**
   * Apply one full pass of the balanced Feistel network over the padded
   * domain [0, 2^(2*half_bits)). The result may fall outside [0, n); the
   * caller is responsible for cycle-walking.
   *
   * @param x  Input value in the padded domain.
   * @return   Permuted value in the padded domain.
   */
  uint32_t permutePadded(uint32_t x) const
  {
    // Split x into the upper (L) and lower (R) half_bits-wide halves.
    uint32_t L = (x >> _half_bits) & _half_mask;
    uint32_t R = x & _half_mask;

    for (unsigned int r = 0; r < _rounds; ++r)
    {
      // Standard balanced Feistel step: new_L = R, new_R = L XOR F(R).
      const uint32_t F = roundFunction(R, r);
      const uint32_t newL = R;
      const uint32_t newR = (L ^ F) & _half_mask;
      L = newL;
      R = newR;
    }

    return ((L & _half_mask) << _half_bits) | (R & _half_mask);
  }

  /**
   * Invert one full pass of the Feistel network by running rounds in reverse.
   *
   * @param y  Value in the padded domain produced by permutePadded.
   * @return   The original input to permutePadded that produced \p y.
   */
  uint32_t invertPadded(uint32_t y) const
  {
    uint32_t L = (y >> _half_bits) & _half_mask;
    uint32_t R = y & _half_mask;

    // Run rounds in reverse; recover prev_R = L then prev_L = R XOR F(prev_R).
    for (int r = static_cast<int>(_rounds) - 1; r >= 0; --r)
    {
      const uint32_t prevR = L;
      const uint32_t F = roundFunction(prevR, static_cast<unsigned int>(r));
      const uint32_t prevL = (R ^ F) & _half_mask;
      L = prevL;
      R = prevR;
    }

    return ((L & _half_mask) << _half_bits) | (R & _half_mask);
  }

  /**
   * Keyed round function F(half, round) used in each Feistel step.
   *
   * Mixes the half-block with both subkeys and a round-dependent constant
   * derived from the golden-ratio fractional bits (0x9e3779b9), then passes
   * the result through a 32-bit avalanche hash (mix32).
   *
   * @param half   The half-block value (R in the forward direction).
   * @param round  Zero-based round index, used to vary the constant each round.
   * @return       Mixed value masked to \p _half_bits width.
   */
  uint32_t roundFunction(uint32_t half, unsigned int round) const
  {
    uint32_t x = half;
    x ^= _k0;
    x += 0x9e3779b9U * static_cast<uint32_t>(round + 1); // round-dependent constant
    x ^= _k1;
    x = mix32(x);
    return x & _half_mask;
  }

  /**
   * Return the number of bits needed to represent values in [0, n-1], i.e.
   * ceil(log2(n)). Returns 0 for n == 1 (no bits needed to index a single element).
   *
   * @param n  Must be > 0.
   */
  static unsigned int ceilLog2(uint32_t n)
  {
    mooseAssert(n > 0, "n must be > 0");

    unsigned int bits = 0;
    uint32_t v = n - 1;
    while (v > 0)
    {
      ++bits;
      v >>= 1;
    }
    return bits;
  }

  /**
   * Bijective 32-bit avalanche hash (finalizer from Murmur3 / degski hash).
   * Used inside the round function to achieve good bit diffusion.
   *
   * @param x  32-bit input.
   * @return   Hashed 32-bit output; the mapping is invertible.
   */
  static uint32_t mix32(uint32_t x)
  {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
  }

  /// Lower 32 bits of the seed, used as the first subkey in the round function
  const uint32_t _k0;
  /// Upper 32 bits of the seed, used as the second subkey in the round function
  const uint32_t _k1;
  /// Size of the permutation domain [0, n)
  const unsigned int _n;
  /// Number of bits in each Feistel half-block: ceil((ceil(log2(n)) + 1) / 2)
  const unsigned int _half_bits;
  /// Bitmask of width _half_bits, used to keep half-block arithmetic in range
  const uint32_t _half_mask;
  /// Number of Feistel rounds to apply per permute/invert call
  const unsigned int _rounds;
};
