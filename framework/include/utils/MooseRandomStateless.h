//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseError.h"
#include "DataIO.h"

// External library includes
#include "randistrs.h"

// STL includes
#include <map>

/**
 * A deterministic, indexable random number generator built on top of the
 * `randistrs` library.
 *
 * This class provides per-stream, seed-able random generators that can produce
 * the n-th sample in a reproducible sequence without maintaining large
 * generator states between calls. It achieves this by keeping a small cached
 * generator state for efficient sequential access, while still allowing
 * deterministic random access to any index `n` via recomputation.
 *
 * Usage Example:
 *
 * ```cpp
 * MooseRandomStateless rng;
 * rng.seed(0, 12345);
 *
 * Real x = rng.rand(0, 5);  // 6th random number for stream 0
 * unsigned int y = rng.randl(0, 10);  // 11th integer in full 32-bit range
 * unsigned int z = rng.randl(0, 7, 0, 100);  // 8th integer in [0, 100)
 *
 * rng.advance(100);  // Advance cache by 100 for speedier access
 * Real x = rng.rand(0, 17);  // The 117th number in the sequence
 * rng.seed(0, 12345);  // Undo the advancement
 * ```
 *
 * Thread Safety:
 * - Accessing different stream indices (`i`) concurrently is safe.
 * - Concurrent access to the same index is not thread-safe, since
 *   internal generator state is cached and mutated during calls to
 *   `evaluate()`. If you need concurrency, create separate stream indices.
 *
 * Performance:
 * - Sequential access (e.g., `n = k, k+1, k+2, ...`) is O(1) per call.
 * - Small forward jumps reuse cached state efficiently.
 * - Backward jumps or large random jumps trigger full reseeding and
 *   re-evaluation up to `n`, which is *O(n)* in the distance skipped.
 *
 * Design Overview:
 * - Each stream index (`i`) has an independent `GeneratorHandler`, created
 *   through `seed(i, seed)`.
 * - `GeneratorHandler` owns several `Generator<T>` instances, each
 *   parameterized by a distribution:
 *     - `mts_ldrand`: Uniform real [0, 1)
 *     - `mts_lrand`: Uniform unsigned int
 *     - `rds_iuniform`: Uniform integer [lower, upper)
 * - `Generator<T>` encapsulates:
 *     - An initial `mt_state` seeded by `mts_seed32new()`
 *     - A cached current state and current index for sequential efficiency
 *     - A stateless, deterministic evaluation method `evaluate(n)`
 * - The random distributions use the same algorithm and seeding semantics
 *   as MOOSE's existing `MooseRandom` class.
 *
 * Limitations:
 * - Not thread-safe for simultaneous access to the same stream ID.
 * - Large backward jumps are linearly expensive.
 *
 * Implementation Notes:
 * - `evaluate()` is declared `const` but mutates internal state marked as
 *   `mutable` to allow transparent caching; this is a deliberate trade-off
 *   for performance.
 * - Each generator instance maintains its own `mt_state`; there is no global
 *   RNG state shared across indices.
 * - There are separate generators for every instance of `rds_iuniform` with
 *   unique ranges (upper - lower). This is because `rds_iuniform` increments
 *   its state differently for each of these ranges. Creating separate
 *   generators for each inputted range is unfortunate, but necessary for
 *   stateless-ness.
 */
class MooseRandomStateless
{
public:
  /**
   * @brief Seed a specific generator index with a given seed value.
   *
   * Creates a new `GeneratorHandler` for the provided stream index `i`.
   * Re-seeding an existing index replaces its handler and resets its state.
   *
   * @param i Stream index (e.g., element or processor ID)
   * @param seed Base seed for random number generation
   */
  void seed(std::size_t i, unsigned int seed)
  {
    _generators[i] = std::make_unique<GeneratorHandler>(seed);
  }

  /**
   * @brief Return the n-th uniform Real random number in [0, 1) for a stream.
   *
   * Uses the cached state for sequential access; otherwise, recomputes from seed.
   *
   * @param i Stream index (previously seeded)
   * @param n 0-based index of the random number to generate
   * @return Random Real number in [0, 1)
   */
  Real rand(std::size_t i, std::size_t n) const
  {
    return getGenerator(i).getRandGenerator().evaluate(n);
  }

  /**
   * @brief Return the n-th unsigned integer from the full 32-bit uniform range.
   * This is useful for generating seeds for new generators.
   *
   * @param i Stream index
   * @param n 0-based index of the random number to generate
   * @return Unsigned integer sampled uniformly in [0, 2^32)
   */
  unsigned int randl(std::size_t i, std::size_t n) const
  {
    return getGenerator(i).getRandlGenerator().evaluate(n);
  }

  /**
   * @brief Return the n-th bounded integer random number in [lower, upper).
   *
   * Uses a cached generator per unique range.
   *
   * @param i Stream index
   * @param n 0-based index of the random number to generate
   * @param lower Lower bound (inclusive)
   * @param upper Upper bound (exclusive)
   * @return Unsigned integer sampled uniformly in [lower, upper)
   */
  unsigned int randl(std::size_t i, std::size_t n, unsigned int lower, unsigned int upper) const
  {
    mooseAssert(upper >= lower, "randl: upper < lower");
    const auto range = upper - lower;
    return lower + getGenerator(i).getRandlbGenerator(range).evaluate(n);
  }

  /**
   * @brief Advance all generators by the specified number.
   *
   * @param count The number of calls to advance.
   */
  void advance(std::size_t count)
  {
    for (auto & [i, handler] : _generators)
      handler->advance(count);
  }

  /**
   * @brief Template class wrapping a single random number generator function.
   *
   * @tparam T Output type of the RNG (e.g., `Real` or `unsigned int`)
   *
   * Each `Generator` stores:
   *  - The RNG function pointer (from randistrs)
   *  - An initial state (seeded once)
   *  - A cached current state and index for fast sequential sampling
   */
  template <typename T>
  class Generator
  {
  public:
    /**
     * @brief Construct a new Generator with a function and seed.
     *
     * @param rng_func RNG function to apply to `mt_state` (e.g., `mts_ldrand`)
     * @param seed Seed used to initialize the base state
     */
    Generator(std::function<T(mt_state *)> rng_func, unsigned int seed)
      : _rng_func(std::move(rng_func))
    {
      // Create a new state with the given seed
      mts_seed32new(&_initial_state, seed);
      _current_state = _initial_state;
      _current_index = 0;
    }

    /**
     * @brief Evaluate the n-th random number in the sequence.
     *
     * Uses cached state when `n >= _current_index` for sequential access.
     * If `n < _current_index`, it resets to the initial seed and recomputes.
     *
     * @param n 0-based index in the random sequence
     * @return T The n-th random value
     */
    T evaluate(std::size_t n) const
    {
      if (n < _current_index)
      {
        _current_state = _initial_state;
        _current_index = 0;
      }

      T val = T();
      for (; _current_index <= n; ++_current_index)
        val = _rng_func(&_current_state);

      return val;
    }

    /**
     * @brief Advance the internal RNG state by a fixed count.
     *
     * This resets `_current_state` to `_initial_state` after advancing.
     *
     * @param count Number of RNG calls to skip
     */
    void advance(std::size_t count)
    {
      advance(_initial_state, count);
      _current_state = _initial_state;
      _current_index = 0;
    }

  private:
    /// Advance a given RNG state `count` times.
    void advance(mt_state & state, std::size_t count)
    {
      for (std::size_t i = 0; i < count; ++i)
        _rng_func(&state);
    }

    /// RNG function pointer
    const std::function<T(mt_state *)> _rng_func;
    /// Base RNG state (seeded)
    mt_state _initial_state;
    /// Cached working RNG state
    mutable mt_state _current_state;
    /// Index of next number to be generated
    mutable std::size_t _current_index;
  };

private:
  /**
   * @brief Container class that manages multiple generators per stream.
   *
   * Each `GeneratorHandler` manages:
   *  - One Real generator for `mts_ldrand`
   *  - One unsigned int generator for `mts_lrand`
   *  - A set of bounded integer generators keyed by their range
   */
  class GeneratorHandler
  {
  public:
    /// Deleted copy constructor and assignment to prevent deep-copy of mutable state
    GeneratorHandler(const GeneratorHandler &) = delete;
    GeneratorHandler & operator=(const GeneratorHandler &) = delete;

    /**
     * @brief Construct a new GeneratorHandler with a given seed.
     *
     * Initializes internal generators for real and integer distributions.
     *
     * @param seed Base seed value for all RNGs in this handler
     */
    GeneratorHandler(unsigned int seed)
      : _seed(seed), _rand_generator(mts_ldrand, seed), _randl_generator(mts_lrand, seed)
    {
    }

    /// @return Reference to the uniform Real generator
    const Generator<Real> & getRandGenerator() const { return _rand_generator; }

    /// @return Reference to the full-range integer generator
    const Generator<unsigned int> & getRandlGenerator() const { return _randl_generator; }

    /**
     * @brief Get or create a bounded integer generator for a given range.
     *
     * Creates a new generator if one for this range does not exist.
     *
     * @param range The integer range of bounded integer (upper - lower)
     * @return Reference to the corresponding Generator instance
     */
    const Generator<unsigned int> & getRandlbGenerator(unsigned int range) const
    {
      auto rng_func = [&range](mt_state * state) { return rds_iuniform(state, 0, range); };
      auto [it, is_new] = _randlb_generators.try_emplace(range, rng_func, _seed);
      if (is_new)
        it->second.advance(_advance_count);
      return it->second;
    }

    /**
     * @brief Advance the RNGs by a fixed count.
     *
     * @param count Number of RNG calls to skip
     */
    void advance(std::size_t count)
    {
      _rand_generator.advance(count);
      _randl_generator.advance(count);
      for (auto & [range, gen] : _randlb_generators)
        gen.advance(count);
      _advance_count += count;
    }

  private:
    /// Seed shared by all internal generators
    const unsigned int _seed;
    /// Uniform Real [0, 1)
    Generator<Real> _rand_generator;
    /// Uniform uint32
    Generator<unsigned int> _randl_generator;
    /// Bounded uniform uint32 (indexed based on bounding range)
    mutable std::map<unsigned int, Generator<unsigned int>> _randlb_generators;

    /// The number of counts the generators have advanced
    /// This needs to be kept around for generation of new randlb generators
    std::size_t _advance_count = 0;

    // for restart capability
    friend void dataStore<MooseRandomStateless>(std::ostream & stream,
                                                MooseRandomStateless & v,
                                                void * context);
    friend void
    dataLoad<MooseRandomStateless>(std::istream & stream, MooseRandomStateless & v, void * context);
  };

  /**
   * @brief Retrieve an existing generator for a given stream index.
   *
   * @param i Stream index
   * @return Reference to the associated `GeneratorHandler`
   * @throws MooseError if no generator has been seeded for this index
   */
  const GeneratorHandler & getGenerator(std::size_t i) const
  {
    auto it = _generators.find(i);
    if (it == _generators.end())
      mooseError("No random generator initialized for id: ", i);
    return *it->second;
  }

  /// Map of per-stream generators (indexed by stream ID)
  std::map<std::size_t, std::unique_ptr<GeneratorHandler>> _generators;

  // for restart capability
  friend void
  dataStore<MooseRandomStateless>(std::ostream & stream, MooseRandomStateless & v, void * context);
  friend void
  dataLoad<MooseRandomStateless>(std::istream & stream, MooseRandomStateless & v, void * context);
};

template <>
inline void
dataStore(std::ostream & stream, MooseRandomStateless & v, void * context)
{
  // Stream ID -> (seed, advance count)
  std::map<std::size_t, std::pair<unsigned int, std::size_t>> generator_data;
  for (const auto & [i, gen] : v._generators)
    generator_data.try_emplace(i, gen->_seed, gen->_advance_count);

  storeHelper(stream, generator_data, context);
}

template <>
inline void
dataLoad(std::istream & stream, MooseRandomStateless & v, void * context)
{
  // Stream ID -> (seed, advance count)
  std::map<std::size_t, std::pair<unsigned int, std::size_t>> generator_data;
  loadHelper(stream, generator_data, context);

  for (const auto & [i, pr] : generator_data)
  {
    const auto [seed, count] = pr;
    v.seed(i, seed);
    v._generators[i]->advance(count);
  }
}
