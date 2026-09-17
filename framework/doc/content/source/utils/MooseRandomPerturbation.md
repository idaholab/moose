# MooseRandomPerturbation

## Overview

MooseRandomPerturbation generates a keyed pseudo-random permutation of the integers
[0, n) using a balanced Feistel network. Given the same seed and n, the mapping is fully
deterministic and bijective: every input in [0, n) maps to a unique output in [0, n), and
different seeds produce statistically independent permutations. This makes it useful for
shuffling a fixed-size index set (e.g., sample or row indices) reproducibly without
materializing the full permutation in memory.

!listing framework/include/utils/MooseRandomPerturbation.h
         start=Generates a keyed pseudo-random permutation
         end=class MooseRandomPerturbation
         include-end=False

An instance is constructed with a seed, the domain size `n`, and the number of Feistel
rounds to apply (more rounds improve mixing at the cost of throughput):

!listing framework/include/utils/MooseRandomPerturbation.h
         link=False
         line=MooseRandomPerturbation(uint64_t seed

The `permute` and `invert` methods then map an index to its permuted value and back, such
that `invert(permute(x)) == x` for every x in [0, n):

!listing framework/include/utils/MooseRandomPerturbation.h link=False line=uint32_t permute(uint32_t x) const;

!listing framework/include/utils/MooseRandomPerturbation.h link=False line=uint32_t invert(uint32_t y) const;

For example, [LatinHypercube](LatinHypercubeSampler.md optional=True) constructs one MooseRandomPerturbation per column
to shuffle that column's row assignment:

!listing stochastic_tools/src/samplers/LatinHypercubeSampler.C line=std::make_unique<MooseRandomPerturbation>

### Recovering from a checkpoint

Because the permutation is fully determined by the seed, n, and round count,
`dataStore`/`dataLoad` specializations serialize only those three values rather than any
per-index mapping, allowing a `MooseRandomPerturbation` (typically held via
`std::unique_ptr`) to be exactly reconstructed on recover:

!listing framework/include/utils/MooseRandomPerturbation.h
         start=dataStore(std::ostream & stream, MooseRandomPerturbation & v, void * context)
         end=v = std::make_unique<MooseRandomPerturbation>(seed, n, rounds);
         include-end=True
