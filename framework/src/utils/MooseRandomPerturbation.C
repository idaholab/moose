//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseRandomPerturbation.h"

MooseRandomPerturbation::MooseRandomPerturbation(uint64_t seed, unsigned int n, unsigned int rounds)
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

uint32_t
MooseRandomPerturbation::permute(uint32_t x) const
{
  mooseAssert(x < _n, "x must be < n");

  uint32_t y = x;
  do
  {
    y = permutePadded(y);
  } while (y >= _n);

  return y;
}

uint32_t
MooseRandomPerturbation::invert(uint32_t y) const
{
  mooseAssert(y < _n, "y must be < n");

  uint32_t x = y;
  do
  {
    x = invertPadded(x);
  } while (x >= _n);

  return x;
}

uint32_t
MooseRandomPerturbation::permutePadded(uint32_t x) const
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

uint32_t
MooseRandomPerturbation::invertPadded(uint32_t y) const
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

uint32_t
MooseRandomPerturbation::roundFunction(uint32_t half, unsigned int round) const
{
  uint32_t x = half;
  x ^= _k0;
  x += 0x9e3779b9U * static_cast<uint32_t>(round + 1); // round-dependent constant
  x ^= _k1;
  x = mix32(x);
  return x & _half_mask;
}

unsigned int
MooseRandomPerturbation::ceilLog2(uint32_t n)
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

uint32_t
MooseRandomPerturbation::mix32(uint32_t x)
{
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}
