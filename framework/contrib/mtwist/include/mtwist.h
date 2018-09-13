#ifndef MTWIST_H
#define MTWIST_H

/*
 * $Id: mtwist.h,v 1.24 2012-12-31 22:22:03-08 geoff Exp $
 *
 * Header file for C/C++ use of the Mersenne-Twist pseudo-RNG.  See
 * http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html for full
 * information.
 *
 * Author of this header file: Geoff Kuenning, March 18, 2001.
 *
 * IMPORTANT NOTE: this implementation assumes a modern compiler.  In
 * particular, it assumes that the "inline" keyword is available, and
 * that the "stdint.h" header file is present.
 *
 * The variables above are defined in an inverted sense because I
 * expect that most modern compilers will support these features.  By
 * inverting the sense, this common case will require no special
 * compiler flags.
 *
 * IMPORTANT NOTE: this software requires access to a 32-bit type.
 * The Mersenne Twist algorithms are not guaranteed to produce correct
 * results with a 64-bit type.
 *
 * The executable part of this software is based on LGPL-ed code by
 * Takuji Nishimura.  The header file is therefore also distributed
 * under the LGPL:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.  You should have
 * received a copy of the GNU Library General Public License along
 * with this library; if not, write to the Free Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * $Log: mtwist.h,v $
 * Revision 1.24  2012-12-31 22:22:03-08  geoff
 * Fix the out-of-bounds bug in mt_llrand and mt_ldrand that were
 * overlooked because I assumed they used mts_*.
 *
 * Revision 1.23  2013-01-01 01:18:52-08  geoff
 * Fix a lot of compiler warnings.
 *
 * Revision 1.22  2012-12-30 16:24:49-08  geoff
 * Declare the new versions of the /dev/random and urandom seeding
 * functions, which now return the seed chosen.
 *
 * Revision 1.21  2012-09-23 23:15:40-07  geoff
 * Fix an array index violation found by valgrind and reported by David
 * Chapman; under some circumstances statevec[-1] could be accessed and
 * used in random-number generation.  The bug only affects the *_llrand
 * and *_ldrand functions.
 *
 * Revision 1.20  2010-12-10 03:28:18-08  geoff
 * Add support for GENERATE_CODE_IN_HEADER.  Fix the URL for the original
 * Web page.
 *
 * Revision 1.19  2010-06-24 20:53:59+12  geoff
 * Switch to using types from stdint.h.  Get rid of all compilation
 * options.
 *
 * Revision 1.18  2010-06-24 00:29:38-07  geoff
 * Do a better job of auto-determining MT_MACHINE_BITS.
 *
 * Revision 1.17  2007-10-26 00:21:06-07  geoff
 * Introduce, document, and use the new mt_u32bit_t type so that the code
 * will compile and run on 64-bit platforms (although it does not
 * currently use the 64-bit Mersenne Twist algorithm).
 *
 * Revision 1.16  2005/11/11 08:21:39  geoff
 * If possible, try to infer MT_MACHINE_BITS from limits.h.
 *
 * Revision 1.15  2003/09/11 23:56:20  geoff
 * Allow stdio references in C++ files; it turns out that ANSI has
 * blessed it.  Declare the various functions as external even if they're
 * inlined or being compiled directly (in mtwist.c).  Get rid of a #ifdef
 * that can't ever be true.
 *
 * Revision 1.14  2003/09/11 05:50:53  geoff
 * Don't allow stdio references from C++, since they're not guaranteed to
 * work on all compilers.  Disable inlining using the MT_INLINE keyword
 * rather than #defining inline, since doing the latter can affect other
 * files and functions than our own.
 *
 * Revision 1.13  2003/07/01 23:29:29  geoff
 * Refer to streams from the standard library using the correct namespace.
 *
 * Revision 1.12  2002/10/30 07:39:54  geoff
 * Declare the new seeding functions.
 *
 * Revision 1.11  2001/06/19 00:41:16  geoff
 * For consistency with other C++ types, don't put out a newline after
 * the saved data.
 *
 * Revision 1.10  2001/06/18 10:09:24  geoff
 * Fix some places where I forgot to set one of the result values.  Make
 * the C++ state vector protected so the random-distributions package can
 * pass it to the C functions.
 *
 * Revision 1.9  2001/06/18 05:40:12  geoff
 * Prefix the compile options with MT_.
 *
 * Revision 1.8  2001/06/14 10:26:59  geoff
 * Invert the sense of the #define flags so that the default is the
 * normal case (if gcc is normal!).  Also default MT_MACHINE_BITS to 32.
 *
 * Revision 1.7  2001/06/14 10:10:38  geoff
 * Move the critical-path PRNG code into the header file so that it can
 * be inlined.  Add saving/loading of state.  Add functions to seed based
 * on /dev/random or the time.  Add the function-call operator in the C++
 * code.
 *
 * Revision 1.6  2001/06/11 10:00:04  geoff
 * Add declarations of the refresh and /dev/random seeding functions.
 * Change getstate to return a complete state pointer, since knowing the
 * position in the state vector is critical to restoring the state.
 *
 * Revision 1.5  2001/04/23 08:36:03  geoff
 * Remember to zero the state pointer when constructing, since otherwise
 * proper initialization won't happen.
 *
 * Revision 1.4  2001/04/14 01:33:32  geoff
 * Clarify the license
 *
 * Revision 1.3  2001/04/14 01:04:54  geoff
 * Add a C++ class, mt_prng, that makes usage more convenient for C++
 * programmers.
 *
 * Revision 1.2  2001/04/09 08:45:00  geoff
 * Fix the name in the #ifndef wrapper, and clean up some outdated comments.
 *
 * Revision 1.1  2001/04/07 09:43:41  geoff
 * Initial revision
 *
 */

#include <stdio.h>
#ifdef __cplusplus
#include <iostream>
#endif /* __cplusplus */

#define __STDC_LIMIT_MACROS
#include <stdint.h>

/*
 * The following value is a fundamental parameter of the algorithm.
 * It was found experimentally using methods described in Matsumoto
 * and Nishimura's paper.  It is exceedingly magic; don't change it.
 */
#define MT_STATE_SIZE	624		/* Size of the MT state vector */

/*
 * Internal state for an MT RNG.  The user can keep multiple mt_state
 * structures around as a way of generating multiple streams of random
 * numbers.
 *
 * In Matsumoto and Nishimura's original paper, the state vector was
 * processed in a forward direction.  I have reversed the state vector
 * in this implementation.  The reason for the reversal is that it
 * allows the critical path to use a test against zero instead of a
 * test against 624 to detect the need to refresh the state.  on most
 * machines, testing against zero is slightly faster.  It also means
 * that a state that has been set to all zeros will be correctly
 * detected as needing initialization; this means that setting a state
 * vector to zero (either with memset or by statically allocating it)
 * will cause the RNG to operate properly.
 */
typedef struct
    {
    uint32_t		statevec[MT_STATE_SIZE];
					/* Vector holding current state */
    int			stateptr;	/* Next state entry to be used */
    int			initialized;	/* NZ if state was initialized */
    }
			mt_state;

#ifdef __cplusplus
extern "C"
    {
#endif

/*
 * Functions for manipulating any generator (given a state pointer).
 */
extern void		mts_mark_initialized(mt_state* state);
					/* Mark a PRNG state as initialized */
extern void		mts_seed32(mt_state* state, uint32_t seed);
					/* Set random seed for any generator */
extern void		mts_seed32new(mt_state* state, uint32_t seed);
					/* Set random seed for any generator */
extern void		mts_seedfull(mt_state* state,
			  uint32_t seeds[MT_STATE_SIZE]);
					/* Set complicated seed for any gen. */
extern uint32_t		mts_seed(mt_state* state);
					/* Choose seed from random input. */
					/* ..Prefers /dev/urandom; uses time */
					/* ..if /dev/urandom unavailable. */
					/* ..Only gives 32 bits of entropy. */
					/* ..Returns seed usable with seed32 */
extern uint32_t		mts_goodseed(mt_state* state);
					/* Choose seed from more random */
					/* ..input than mts_seed.  Prefers */
					/* ../dev/random; uses time if that */
					/* ..is unavailable.  Only gives 32 */
					/* ..bits of entropy. */
					/* ..Returns seed usable with seed32 */
extern void		mts_bestseed(mt_state* state);
					/* Choose seed from extremely random */
					/* ..input (can be *very* slow). */
					/* ..Prefers /dev/random and reads */
					/* ..the entire state from there. */
					/* ..If /dev/random is unavailable, */
					/* ..falls back to mt_goodseed().  */
					/* ..Not usually worth the cost.  */
extern void		mts_refresh(mt_state* state);
					/* Generate 624 more random values */
extern int		mts_savestate(FILE* statefile, mt_state* state);
					/* Save state to a file (ASCII). */
					/* ..Returns NZ if succeeded. */
extern int		mts_loadstate(FILE* statefile, mt_state* state);
					/* Load state from a file (ASCII). */
					/* ..Returns NZ if succeeded. */

/*
 * Functions for manipulating the default generator.
 */
extern void		mt_seed32(uint32_t seed);
					/* Set random seed for default gen. */
extern void		mt_seed32new(uint32_t seed);
					/* Set random seed for default gen. */
extern void		mt_seedfull(uint32_t seeds[MT_STATE_SIZE]);
					/* Set complicated seed for default */
extern uint32_t		mt_seed(void);	/* Choose seed from random input. */
					/* ..Prefers /dev/urandom; uses time */
					/* ..if /dev/urandom unavailable. */
					/* ..Only gives 32 bits of entropy. */
extern uint32_t		mt_goodseed(void);
					/* Choose seed from more random */
					/* ..input than mts_seed.  Prefers */
					/* ../dev/random; uses time if that */
					/* ..is unavailable.  Only gives 32 */
					/* ..bits of entropy. */
extern void		mt_bestseed(void);
					/* Choose seed from extremely random */
					/* ..input (can be *very* slow). */
					/* ..Prefers /dev/random and reads */
					/* ..the entire state from there. */
					/* ..If /dev/random is unavailable, */
					/* ..falls back to mt_goodseed().  */
					/* ..Not usually worth the cost.  */
extern mt_state*	mt_getstate(void);
					/* Get current state of default */
					/* ..generator */
extern int		mt_savestate(FILE* statefile);
					/* Save state to a file (ASCII) */
					/* ..Returns NZ if succeeded. */
extern int		mt_loadstate(FILE* statefile);
					/* Load state from a file (ASCII) */
					/* ..Returns NZ if succeeded. */

#ifdef __cplusplus
    }
#endif

/*
 * Functions for generating random numbers.  The actual code of the
 * functions is given in this file so that it can be declared inline.
 * For compilers that don't have the inline feature, mtwist.c will
 * incorporate this file with some clever #defining so that the code
 * actually gets compiled.  In that case, however, "extern"
 * definitions will be needed here, so we give them.
 */
#ifdef __cplusplus
#endif /* __cplusplus */

extern uint32_t		mts_lrand(mt_state* state);
					/* Generate 32-bit value, any gen. */
#ifdef UINT64_MAX
extern uint64_t		mts_llrand(mt_state* state);
					/* Generate 64-bit value, any gen. */
#endif /* UINT64_MAX */
extern double		mts_drand(mt_state* state);
					/* Generate floating value, any gen. */
					/* Fast, with only 32-bit precision */
extern double		mts_ldrand(mt_state* state);
					/* Generate floating value, any gen. */
					/* Slower, with 64-bit precision */

extern uint32_t		mt_lrand(void);	/* Generate 32-bit random value */
#ifdef UINT64_MAX
extern uint64_t		mt_llrand(void);
					/* Generate 64-bit random value */
#endif /* UINT64_MAX */
extern double		mt_drand(void);
					/* Generate floating value */
					/* Fast, with only 32-bit precision */
extern double		mt_ldrand(void);
					/* Generate floating value */
					/* Slower, with 64-bit precision */

/*
 * Tempering parameters.  These are perhaps the most magic of all the magic
 * values in the algorithm.  The values are again experimentally determined.
 * The values generated by the recurrence relation (constants above) are not
 * equidistributed in 623-space.  For some reason, the tempering process
 * produces that effect.  Don't ask me why.  Read the paper if you can
 * understand the math.  Or just trust these magic numbers.
 */
#define MT_TEMPERING_MASK_B 0x9d2c5680
#define MT_TEMPERING_MASK_C 0xefc60000
#define MT_TEMPERING_SHIFT_U(y) \
			(y >> 11)
#define MT_TEMPERING_SHIFT_S(y) \
			(y << 7)
#define MT_TEMPERING_SHIFT_T(y) \
			(y << 15)
#define MT_TEMPERING_SHIFT_L(y) \
			(y >> 18)

/*
 * Macros to do the tempering.  MT_PRE_TEMPER does all but the last step;
 * it's useful for situations where the final step can be incorporated
 * into a return statement.  MT_FINAL_TEMPER does that final step (not as
 * an assignment).  MT_TEMPER does the entire process.  Note that
 * MT_PRE_TEMPER and MT_TEMPER both modify their arguments.
 */
#define MT_PRE_TEMPER(value)						\
    do									\
	{								\
	value ^= MT_TEMPERING_SHIFT_U(value);				\
	value ^= MT_TEMPERING_SHIFT_S(value) & MT_TEMPERING_MASK_B;	\
	value ^= MT_TEMPERING_SHIFT_T(value) & MT_TEMPERING_MASK_C;	\
	}								\
	while (0)
#define MT_FINAL_TEMPER(value) \
			((value) ^ MT_TEMPERING_SHIFT_L(value))
#define MT_TEMPER(value)						\
    do									\
	{								\
	value ^= MT_TEMPERING_SHIFT_U(value);				\
	value ^= MT_TEMPERING_SHIFT_S(value) & MT_TEMPERING_MASK_B;	\
	value ^= MT_TEMPERING_SHIFT_T(value) & MT_TEMPERING_MASK_C;	\
	value ^= MT_TEMPERING_SHIFT_L(value);				\
	}								\
	while (0)

/*
 * The Mersenne Twist PRNG makes it default state available as an
 * external variable.  This feature is undocumented, but is useful to
 * use because it allows us to avoid implementing every randistr function
 * twice.  (In fact, the feature was added to enable randistrs.c to be
 * written.  It would be better to write in C++, where I could control
 * the access to the state.)
 */
extern mt_state		mt_default_state;
					/* State of the default generator */
extern double		mt_32_to_double;
					/* Multiplier to convert long to dbl */
extern double		mt_64_to_double;
					/* Mult'r to cvt long long to dbl */

/*
 * In gcc, inline functions must be declared extern or they'll produce
 * assembly code (and thus linking errors).  We have to work around
 * that difficulty with the MT_EXTERN define.
 */
#ifndef MT_EXTERN
#ifdef __cplusplus
#define MT_EXTERN			/* C++ doesn't need static */
#else /* __cplusplus */
#define MT_EXTERN	extern		/* C (at least gcc) needs extern */
#endif /* __cplusplus */
#endif /* MT_EXTERN */

/*
 * Make it possible for mtwist.c to disable the inline keyword.  We
 * use our own keyword so that we don't interfere with inlining in
 * C/C++ header files, above.
 */
#ifndef MT_INLINE
#define MT_INLINE	inline		/* Compiler has inlining */
#endif /* MT_INLINE */

/*
 * Try to guess whether the compiler is one (like gcc) that requires
 * inline code to be available in the header file, or a smarter one
 * that gets inlines directly from object files.  But if we've been
 * given the information, trust it.
 */
#ifndef MT_GENERATE_CODE_IN_HEADER
#ifdef __GNUC__
#define MT_GENERATE_CODE_IN_HEADER 1
#endif /* __GNUC__ */
#if defined(__INTEL_COMPILER)  ||  defined(_MSC_VER)
#define MT_GENERATE_CODE_IN_HEADER 1
#endif /* __INTEL_COMPILER || _MSC_VER */
#endif /* MT_GENERATE_CODE_IN_HEADER */

#if MT_GENERATE_CODE_IN_HEADER
/*
 * Generate a random number in the range 0 to 2^32-1, inclusive, working
 * from a given state vector.
 *
 * The generator is optimized for speed.  The primary optimization is that
 * the pseudorandom numbers are generated in batches of MT_STATE_SIZE.  This
 * saves the cost of a modulus operation in the critical path.
 */
MT_EXTERN MT_INLINE uint32_t mts_lrand(
    mt_state*	state)		/* State for the PRNG */
    {
    uint32_t random_value; /* Pseudorandom value generated */

    if (state->stateptr <= 0)
	mts_refresh(state);

    random_value = state->statevec[--state->stateptr];
    MT_PRE_TEMPER(random_value);
    return MT_FINAL_TEMPER(random_value);
    }

#ifdef UINT64_MAX
/*
 * Generate a random number in the range 0 to 2^64-1, inclusive, working
 * from a given state vector.
 *
 * According to Matsumoto and Nishimura, such a number can be generated by
 * simply concatenating two 32-bit pseudorandom numbers.  Who am I to argue?
 *
 * Note that there is a slight inefficiency here: if the 624-entry state is
 * recycled on the second call to mts_lrand, there will be an unnecessary
 * check to see if the state has been initialized.  The cost of that check
 * seems small (since it happens only once every 624 random numbers, and
 * never if only 64-bit numbers are being generated), so I didn't bother to
 * optimize it out.  Doing so would be messy, since it would require two
 * nearly-identical internal implementations of mts_lrand.
 */
MT_EXTERN MT_INLINE uint64_t mts_llrand(
    mt_state*	state)		/* State for the PRNG */
    {
    uint32_t random_value_1; /* 1st pseudorandom value generated */
    uint32_t random_value_2; /* 2nd pseudorandom value generated */

    /*
     * For maximum speed, we'll handle the two overflow cases
     * together.  That will save us one test in the common case, at
     * the expense of an extra one in the overflow case.
     */
    if (--state->stateptr <= 0)
	{
	if (state->stateptr < 0)
	    {
	    mts_refresh(state);
	    random_value_1 = state->statevec[--state->stateptr];
	    }
	else
	    {
	    random_value_1 = state->statevec[state->stateptr];
	    mts_refresh(state);
	    }
	}
    else
	random_value_1 = state->statevec[state->stateptr];

    MT_TEMPER(random_value_1);

    random_value_2 = state->statevec[--state->stateptr];
    MT_PRE_TEMPER(random_value_2);

    return ((uint64_t) random_value_1 << 32)
      | (uint64_t) MT_FINAL_TEMPER(random_value_2);
    }
#endif /* UINT64_MAX */

/*
 * Generate a double-precision random number between 0 (inclusive) and 1.0
 * (exclusive).  This function is optimized for speed, but it only generates
 * 32 bits of precision.  Use mts_ldrand to get 64 bits of precision.
 */
MT_EXTERN MT_INLINE double mts_drand(
    mt_state*	state)		/* State for the PRNG */
    {
    uint32_t random_value; /* Pseudorandom value generated */

    if (state->stateptr <= 0)
	mts_refresh(state);

    random_value = state->statevec[--state->stateptr];
    MT_TEMPER(random_value);

    return random_value * mt_32_to_double;
    }

/*
 * Generate a double-precision random number between 0 (inclusive) and 1.0
 * (exclusive).  This function generates 64 bits of precision.  Use
 * mts_drand for more speed but less precision.
 */
MT_EXTERN MT_INLINE double mts_ldrand(
    mt_state*	state)		/* State for the PRNG */
    {
#ifdef UINT64_MAX
    uint64_t		final_value;	/* Final (integer) value */
#endif /* UINT64_MAX */
    uint32_t random_value_1; /* 1st pseudorandom value generated */
    uint32_t random_value_2; /* 2nd pseudorandom value generated */

    /*
     * For maximum speed, we'll handle the two overflow cases
     * together.  That will save us one test in the common case, at
     * the expense of an extra one in the overflow case.
     */
    if (--state->stateptr <= 0)
	{
	if (state->stateptr < 0)
	    {
	    mts_refresh(state);
	    random_value_1 = state->statevec[--state->stateptr];
	    }
	else
	    {
	    random_value_1 = state->statevec[state->stateptr];
	    mts_refresh(state);
	    }
	}
    else
	random_value_1 = state->statevec[state->stateptr];

    MT_TEMPER(random_value_1);

    random_value_2 = state->statevec[--state->stateptr];
    MT_TEMPER(random_value_2);

#ifdef UINT64_MAX
    final_value = ((uint64_t) random_value_1 << 32) | (uint64_t) random_value_2;
    return final_value * mt_64_to_double;
#else /* UINT64_MAX */
    return random_value_1 * mt_32_to_double + random_value_2 * mt_64_to_double;
#endif /* UINT64_MAX */
    }

/*
 * Generate a random number in the range 0 to 2^32-1, inclusive, working
 * from the default state vector.
 *
 * See mts_lrand for full commentary.
 */
MT_EXTERN MT_INLINE uint32_t mt_lrand(void)
    {
    uint32_t random_value; /* Pseudorandom value generated */

    if (mt_default_state.stateptr <= 0)
	mts_refresh(&mt_default_state);

    random_value = mt_default_state.statevec[--mt_default_state.stateptr];
    MT_PRE_TEMPER(random_value);

    return MT_FINAL_TEMPER(random_value);
    }

#ifdef UINT64_MAX
/*
 * Generate a random number in the range 0 to 2^64-1, inclusive, working
 * from the default state vector.
 *
 * See mts_llrand for full commentary.
 */
MT_EXTERN MT_INLINE uint64_t mt_llrand(void)
    {
    uint32_t random_value_1; /* 1st pseudorandom value generated */
    uint32_t random_value_2; /* 2nd pseudorandom value generated */

    /*
     * For maximum speed, we'll handle the two overflow cases
     * together.  That will save us one test in the common case, at
     * the expense of an extra one in the overflow case.
     */
    if (--mt_default_state.stateptr <= 0)
	{
	if (mt_default_state.stateptr < 0)
	    {
	    mts_refresh(&mt_default_state);
	    random_value_1 =
	      mt_default_state.statevec[--mt_default_state.stateptr];
	    }
	else
	    {
	    random_value_1 =
	      mt_default_state.statevec[mt_default_state.stateptr];
	    mts_refresh(&mt_default_state);
	    }
	}
    else
	random_value_1 = mt_default_state.statevec[mt_default_state.stateptr];

    MT_TEMPER(random_value_1);

    random_value_2 = mt_default_state.statevec[--mt_default_state.stateptr];
    MT_PRE_TEMPER(random_value_2);

    return ((uint64_t) random_value_1 << 32)
      | (uint64_t) MT_FINAL_TEMPER(random_value_2);
    }
#endif /* UINT64_MAX */

/*
 * Generate a double-precision random number between 0 (inclusive) and 1.0
 * (exclusive).  This function is optimized for speed, but it only generates
 * 32 bits of precision.  Use mt_ldrand to get 64 bits of precision.
 */
MT_EXTERN MT_INLINE double mt_drand(void)
    {
    uint32_t random_value; /* Pseudorandom value generated */

    if (mt_default_state.stateptr <= 0)
	mts_refresh(&mt_default_state);

    random_value = mt_default_state.statevec[--mt_default_state.stateptr];
    MT_TEMPER(random_value);

    return random_value * mt_32_to_double;
    }

/*
 * Generate a double-precision random number between 0 (inclusive) and 1.0
 * (exclusive).  This function generates 64 bits of precision.  Use
 * mts_drand for more speed but less precision.
 */
MT_EXTERN MT_INLINE double mt_ldrand(void)
    {
#ifdef UINT64_MAX
    uint64_t		final_value;	/* Final (integer) value */
#endif /* UINT64_MAX */
    uint32_t random_value_1; /* 1st pseudorandom value generated */
    uint32_t random_value_2; /* 2nd pseudorandom value generated */

    /*
     * For maximum speed, we'll handle the two overflow cases
     * together.  That will save us one test in the common case, at
     * the expense of an extra one in the overflow case.
     */
    if (--mt_default_state.stateptr <= 0)
	{
	if (mt_default_state.stateptr < 0)
	    {
	    mts_refresh(&mt_default_state);
	    random_value_1 =
	      mt_default_state.statevec[--mt_default_state.stateptr];
	    }
	else
	    {
	    random_value_1 =
	      mt_default_state.statevec[mt_default_state.stateptr];
	    mts_refresh(&mt_default_state);
	    }
	}
    else
	random_value_1 = mt_default_state.statevec[mt_default_state.stateptr];

    MT_TEMPER(random_value_1);

    random_value_2 = mt_default_state.statevec[--mt_default_state.stateptr];
    MT_TEMPER(random_value_2);

#ifdef UINT64_MAX
    final_value = ((uint64_t) random_value_1 << 32) | (uint64_t) random_value_2;
    return final_value * mt_64_to_double;
#else /* UINT64_MAX */
    return random_value_1 * mt_32_to_double + random_value_2 * mt_64_to_double;
#endif /* UINT64_MAX */
    }
#endif /* MT_GENERATE_CODE_IN_HEADER */

#ifdef __cplusplus
/*
 * C++ interface to the Mersenne Twist PRNG.  This class simply
 * provides a more C++-ish way to access the PRNG.  Only state-based
 * functions are provided.  All functions are inlined, both for speed
 * and so that the same implementation code can be used in C and C++.
 */
class mt_prng
    {
	friend class mt_empirical_distribution;
    public:
	/*
	 * Constructors and destructors.  The default constructor
	 * leaves initialization (seeding) for later unless pickSeed
	 * is true, in which case the seed is chosen based on either
	 * /dev/urandom (if available) or the system time.  The other
	 * constructors accept either a 32-bit seed, or a full
	 * 624-integer seed.
	 */
			mt_prng(	// Default constructor
			    bool pickSeed = false)
					// True to get seed from /dev/urandom
					// ..or time
			    {
			    state.stateptr = 0;
			    state.initialized = 0;
			    if (pickSeed)
				(void)mts_seed(&state);
			    }
			mt_prng(uint32_t newseed)
					// Construct with 32-bit seeding
			    {
			    state.stateptr = 0;
			    state.initialized = 0;
			    mts_seed32(&state, newseed);
			    }
			mt_prng(uint32_t seeds[MT_STATE_SIZE])
					// Construct with full seeding
			    {
			    state.stateptr = 0;
			    state.initialized = 0;
			    mts_seedfull(&state, seeds);
			    }
			~mt_prng() { }

	/*
	 * Copy and assignment are best left defaulted.
	 */

	/*
	 * PRNG seeding functions.
	 */
	void		seed32(uint32_t newseed)
					// Set 32-bit random seed
			    {
			    mts_seed32(&state, newseed);
			    }
	void		seed32new(uint32_t newseed)
					// Set 32-bit random seed
			    {
			    mts_seed32new(&state, newseed);
			    }
	void		seedfull(uint32_t seeds[MT_STATE_SIZE])
					// Set complicated random seed
			    {
			    mts_seedfull(&state, seeds);
			    }
	uint32_t	seed()		// Choose seed from random input
			    {
			    return mts_seed(&state);
			    }
	uint32_t	goodseed()	// Choose better seed from random input
			    {
			    return mts_goodseed(&state);
			    }
	void		bestseed()	// Choose best seed from random input
			    {
			    mts_bestseed(&state);
			    }
	friend std::ostream&
			operator<<(std::ostream& stream, const mt_prng& rng);
	friend std::istream&
			operator>>(std::istream& stream, mt_prng& rng);

	/*
	 * PRNG generation functions
	 */
	uint32_t	lrand()		// Generate 32-bit pseudo-random value
			    {
			    return mts_lrand(&state);
			    }
#ifdef UINT64_MAX
	uint64_t	llrand()	// Generate 64-bit pseudo-random value
			    {
			    return mts_llrand(&state);
			    }
#endif /* UINT64_MAX */
	double		drand()		// Generate fast 32-bit floating value
			    {
			    return mts_drand(&state);
			    }
	double		ldrand()	// Generate slow 64-bit floating value
			    {
			    return mts_ldrand(&state);
			    }

	/*
	 * Following Richard J. Wagner's example, we overload the
	 * function-call operator to return a 64-bit floating value.
	 * That allows the common use of the PRNG to be simplified as
	 * in the following example:
	 *
	 *	mt_prng ranno(true);
	 *	// ...
	 *	coinFlip = ranno() >= 0.5 ? heads : tails;
	 */
	double		operator()()
			    {
			    return mts_drand(&state);
			    }
    protected:
	/*
	 * Protected data
	 */
	mt_state	state;		// Current state of the PRNG
    };

#if MT_GENERATE_CODE_IN_HEADER
/*
 * Save state to a stream.  See mts_savestate.
 */
MT_INLINE std::ostream& operator<<(
    std::ostream&	stream,		// Stream to save to
    const mt_prng&	rng)		// PRNG to save
    {
    for (int i = MT_STATE_SIZE;  --i >= 0;  )
	{
	if (!(stream << rng.state.statevec[i] << ' '))
	    return stream;
	}

    return stream << rng.state.stateptr;
    }

/*
 * Restore state from a stream.  See mts_loadstate.
 */
MT_INLINE std::istream& operator>>(
    std::istream&	stream,		// Stream to laod from
    mt_prng&		rng)		// PRNG to load
    {
    rng.state.initialized = rng.state.stateptr = 0;
    for (int i = MT_STATE_SIZE;  --i >= 0;  )
	{
	if (!(stream >> rng.state.statevec[i]))
	    return stream;
	}

    if (!(stream >> rng.state.stateptr))
	{
	rng.state.stateptr = 0;
	return stream;
	}

    /*
     * If the state is invalid, all we can do is to make it uninitialized.
     */
    if (rng.state.stateptr < 0  ||  rng.state.stateptr > MT_STATE_SIZE)
	{
	rng.state.stateptr = 0;
	return stream;
	}

    mts_mark_initialized(&rng.state);

    return stream;
    }
#endif /* MT_GENERATE_CODE_IN_HEADER */
#endif /* __cplusplus */

#endif /* MTWIST_H */
