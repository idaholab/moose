#ifndef lint
#ifdef __GNUC__
#define ATTRIBUTE(attrs) __attribute__(attrs)
#else
#define ATTRIBUTE(attrs)
#endif
static char Rcs_Id[] ATTRIBUTE((used)) =
    "$Id: mtwist.c,v 1.28 2014-01-23 21:11:42-08 geoff Exp $";
#endif

/*
 * C library functions for generating pseudorandom numbers using the
 * Mersenne Twist algorithm.  See M. Matsumoto and T. Nishimura,
 * "Mersenne Twister: A 623-Dimensionally Equidistributed Uniform
 * Pseudo-Random Number Generator", ACM Transactions on Modeling and
 * Computer Simulation, Vol. 8, No. 1, January 1998, pp 3--30.
 *
 * The Web page on the Mersenne Twist algorithm is at:
 *
 * http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 *
 * These functions were written by Geoff Kuenning, Claremont, CA.
 *
 * IMPORTANT NOTE: this implementation assumes a modern compiler.  In
 * particular, it assumes that the "inline" keyword is available, and
 * that the "inttypes.h" header file is present.
 *
 * IMPORTANT NOTE: this software requires access to a 32-bit type.
 * The Mersenne Twist algorithms are not guaranteed to produce correct
 * results with a 64-bit type.
 *
 * This software is based on LGPL-ed code by Takuji Nishimura.  It has
 * also been heavily influenced by code written by Shawn Cokus, and
 * somewhat influenced by code written by Richard J. Wagner.  It is
 * therefore also distributed under the LGPL:
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
 * $Log: mtwist.c,v $
 * Revision 1.28  2014-01-23 21:11:42-08  geoff
 * Fix an obsolete gettimeofday call
 *
 * Revision 1.27  2013-06-12 23:22:03-07  geoff
 * Validity-check state pointer when saving.
 *
 * Revision 1.26  2013-01-01 01:18:52-08  geoff
 * Fix a lot of comiler warnings.  Try to fall back to /dev/random if
 * /dev/urandom doesn't exist.
 *
 * Revision 1.25  2012-12-30 16:24:49-08  geoff
 * Use gcc attributes to suppress warnings on Rcs_Id.  Also get rid of
 * most of the table of contents, to suppress a different gcc warning.
 * Fix mts_seed and mts_goodseed to return the 32-bit seeds they chose.
 * Fix all three /dev/Xrandom seeding functions to use only the entropy
 * they need, rather than letting stdio eat far more than necessary.
 * (Thanks to Markus Armbruster for all three ideas.)
 *
 * Revision 1.24  2011-02-18 00:49:12-08  geoff
 * Fix a (harmless) typecasting error in mts_bestseed.  Thanks to Waclaw
 * Kusnierczyk for pointing it out.
 *
 * Revision 1.23  2010-12-11 00:28:18+13  geoff
 * Add support for GENERATE_CODE_IN_HEADER.  Fix the URL for the original
 * Web page.
 *
 * Revision 1.22  2010-06-24 20:53:59+12  geoff
 * Switch to using types and formats from inttypes.h.  Get rid of all
 * compilation options.
 *
 * Revision 1.21  2010-06-24 00:29:38-07  geoff
 * Correctly save and restore the state vector even if longs are wider
 * than 32 bits.
 *
 * Revision 1.20  2007-10-26 00:21:06-07  geoff
 * Use the new mt_u32bit_t type.
 *
 * Revision 1.19  2003/09/11 05:55:19  geoff
 * Get rid of some minor compiler warnings.
 *
 * Revision 1.18  2003/09/11 05:50:53  geoff
 * Don't #define inline to nothing, since that breaks standard include
 * files.  Instead, use MT_INLINE as a synonym.
 *
 * Revision 1.17  2002/10/31 22:07:10  geoff
 * Make WIN32 detection work with GCC as well as MS C
 *
 * Revision 1.16  2002/10/31 22:04:59  geoff
 * Fix a typo in the WIN32 option
 *
 * Revision 1.15  2002/10/31 06:01:43  geoff
 * Incorporate Joseph Brill's Windows-portability changes
 *
 * Revision 1.14  2002/10/30 07:39:53  geoff
 * Reintroduce the old seeding functions (so that old code will still
 * produce the same results), and give the new versions new names.
 *
 * Revision 1.13  2002/10/30 01:08:26  geoff
 * Switch to M&T's new initialization method
 *
 * Revision 1.12  2001/06/18 05:40:12  geoff
 * Prefix the compile options with MT_.
 *
 * Revision 1.11  2001/06/14 10:26:59  geoff
 * Invert the sense of the #define flags so that the default is the
 * normal case (if gcc is normal!).  Also default MT_MACHINE_BITS to 32.
 *
 * Revision 1.10  2001/06/14 10:10:38  geoff
 * Move the RNG functions into the header file so they can be inlined.
 * Add saving/loading of state.  Add a function that marks the PRNG as
 * initialized while also calculating critical constants.  Run the
 * refresh routine whenever seed32 is called.  Add functions to seed
 * based on /dev/random or the time.
 *
 * Revision 1.9  2001/06/11 10:00:04  geoff
 * Major changes to improve flexibility and performance, and to prepare
 * for inlining.  This code is about as fast as it can get without
 * inlining the various PRNG functions.  Add seed/goodseed/bestseed for
 * seeding from random start values.  Add the refresh routine a la Cokus,
 * but optimize it by unrolling loops.  Change getstate to return a
 * complete state pointer, since knowing the position in the state vector
 * is critical to restoring state.  Add more macros to improve
 * readability.  Rename certain macros in preparation for inlining.  Get
 * rid of leftover optimizer-bug stuff.  Stop using mtwist_guts.h;
 * instead use direct code (via macros) and the refresh function.
 *
 * Revision 1.8  2001/04/23 08:36:03  geoff
 * Move the #defined code into a header file to ease stepping with a debugger.
 *
 * Revision 1.7  2001/04/23 08:00:13  geoff
 * Add code to work around optimizer bug
 *
 * Revision 1.6  2001/04/14 01:33:32  geoff
 * Clarify the license
 *
 * Revision 1.5  2001/04/09 08:45:00  geoff
 * Rename default_state to mt_default_state, and make it global so that
 * the random-distribution code can use it.
 *
 * Revision 1.4  2001/04/07 23:24:11  geoff
 * My guess in the commentary for the last delta was right: it's faster
 * on a x86 to convert the two halves of the PRN to double, multiplying
 * them by the appropriate value to scale them, and then add them as
 * doubles.  I suspect the reason is that there is no instruction to
 * convert a 64-bit value directly to a double, so the work of building
 * the long long (which isn't easy anyway, without assembly access) is
 * worse than wasted.  So add support for MT_MACHINE_BITS, and only go
 * the via-long-long route on a true 64-bit machine.
 *
 * Revision 1.3  2001/04/07 23:09:38  geoff
 * Get rid of MT_INLINE.  Convert all of the code to use preprocessor
 * macros for the guts of the PRNG code.  Take advantage of the
 * conversion to get rid of unnecessary calls initialization tests.  Also
 * clean up the generation of long-double pseudorandom numbers on
 * machines that have the long long type (by converting first to a long
 * long, then to a double, saving one floating-point operation).  The
 * latter change might be a mistake on 32-bit machines.  The code is now
 * much faster as a result of macro-izing.
 *
 * Revision 1.2  2001/04/07 22:21:41  geoff
 * Make the long-double code a hair faster by always having a 64-bit
 * conversion constant.  Add commentary to the PRNG loop.
 *
 * Revision 1.1  2001/04/07 09:43:41  geoff
 * Initial revision
 *
 */

#ifdef _WIN32
#undef WIN32
#define WIN32
#endif /* _WIN32 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <sys/timeb.h>
#else /* WIN32 */
#include <sys/time.h>
#endif /* WIN32 */

/*
 * Before we include the Mersenne Twist header file, we must do a bit
 * of magic setup.  The code for actual random-number generation
 * resides in that file rather than here.  We need to arrange for the
 * code to be compiled into this .o file, either because inlines
 * aren't supported or because somebody might want to take a pointer
 * to a function.  We do so with a couple of careful #defines.
 */
#define MT_INLINE			/* Disable the inline keyword */
#define MT_EXTERN			/* Generate real code for functions */
#undef MT_GENERATE_CODE_IN_HEADER
#define MT_GENERATE_CODE_IN_HEADER 1	/* Generate code when #including */

#include "mtwist.h"

/*
 * Table of (non-global) contents:
 */
static uint32_t		mts_devseed(mt_state* state, char* seed_dev);
					/* Choose seed from a device */

/*
 * The following values are fundamental parameters of the algorithm.
 * With the exception of the two masks, all of them were found
 * experimentally using methods described in Matsumoto and Nishimura's
 * paper.  They are exceedingly magic; don't change them.
 */

/* MT_STATE_SIZE is defined in the header file. */
#define RECURRENCE_OFFSET 397		/* Offset into state space for the */
					/* ..recurrence relation.  The */
					/* ..recurrence mashes together two */
					/* ..values that are separated by */
					/* ..this offset in the state */
					/* ..space. */
#define MATRIX_A	0x9908b0df	/* Constant vector A for the */
					/* ..recurrence relation.  The */
					/* ..mashed-together value is */
					/* ..multiplied by this vector to */
					/* ..get a new value that will be */
					/* ..stored into the state space. */

/*
 * Width of an unsigned int.  Don't change this even if your ints are 64 bits.
 */
#define BIT_WIDTH	32		/* Work with 32-bit words */

/*
 * Masks for extracting the bits to be mashed together.  The widths of these
 * masks are also fundamental parameters of the algorithm, determined
 * experimentally -- but of course the masks themselves are simply bit
 * selectors.
 */
#define UPPER_MASK	0x80000000	/* Most significant w-r bits */
#define LOWER_MASK	0x7fffffff	/* Least significant r bits */

/*
 * Macro to simplify code in the generation loop.  This function
 * combines the top bit of x with the bottom 31 bits of y.
 */
#define COMBINE_BITS(x, y) \
			(((x) & UPPER_MASK) | ((y) & LOWER_MASK))

/*
 * Another generation-simplification macro.  This one does the magic
 * scrambling function.
 */
#define MATRIX_MULTIPLY(original, new) \
			((original) ^ ((new) >> 1) \
			  ^ matrix_decider[(new) & 0x1])

/*
 * Parameters of Knuth's PRNG (Line 25, Table 1, p. 102 of "The Art of
 * Computer Programming, Vol. 2, 2nd ed, 1981).
 */
#define KNUTH_MULTIPLIER_OLD \
			69069

/*
 * Parameters of Knuth's PRNG (p. 106 of "The Art of Computer
 * Programming, Vol. 2, 3rd ed).
 */
#define KNUTH_MULTIPLIER_NEW \
			1812433253ul
#define KNUTH_SHIFT	30		// Even on a 64-bit machine!

/*
 * Default 32-bit random seed if mts_seed32 wasn't called
 */
#define DEFAULT_SEED32_OLD \
			4357
#define DEFAULT_SEED32_NEW \
			5489ul

/*
 * Where to get random numbers
 */
#define DEVRANDOM	"/dev/random"
#define DEVURANDOM	"/dev/urandom"

/*
 * Many applications need only a single PRNG, so it's a nuisance to have to
 * specify a state.  For those applications, we will provide a default
 * state, and functions to use it.
 */
mt_state		mt_default_state;

/*
 * To generate double-precision random numbers, we need to divide the result
 * of mts_lrand or mts_llrand by 2^32 or 2^64, respectively.  The quickest
 * way to do that on most machines is to multiply by the inverses of those
 * numbers.  However, I don't trust the compiler to correctly convert the
 * corresponding decimal constant.  So we will compute the correct number at
 * run time as part of initialization, which will produce a nice exact
 * result.
 */
double			mt_32_to_double;
					/* Multiplier to convert long to dbl */
double			mt_64_to_double;
					/* Mult'r to cvt long long to dbl */

/*
 * In the recurrence relation, the new value is XORed with MATRIX_A only if
 * the lower bit is nonzero.  Since most modern machines don't like to
 * branch, it's vastly faster to handle this decision by indexing into an
 * array.  The chosen bit is used as an index into the following vector,
 * which produces either zero or MATRIX_A and thus the desired effect.
 */
static uint32_t	matrix_decider[2] =
			  {0x0, MATRIX_A};

/*
 * Mark a PRNG's state as having been initialized.  This is the only
 * way to set that field nonzero; that way we can be sure that the
 * constants are set properly before the PRNG is used.
 *
 * As a side effect, set up some constants that the PRNG assumes are
 * valid.  These are calculated at initialization time rather than
 * being written as decimal constants because I frankly don't trust
 * the compiler's ASCII conversion routines.
 */
void mts_mark_initialized(
    mt_state*		state)		/* State vector to mark initialized */
    {
    int			i;		/* Power of 2 being calculated */

    /*
     * Figure out the proper multiplier for long-to-double conversion.  We
     * don't worry too much about efficiency, since the assumption is that
     * initialization is vastly rarer than generation of random numbers.
     */
    mt_32_to_double = 1.0;
    for (i = 0;  i < BIT_WIDTH;  i++)
	mt_32_to_double /= 2.0;
    mt_64_to_double = mt_32_to_double;
    for (i = 0;  i < BIT_WIDTH;  i++)
	mt_64_to_double /= 2.0;

    state->initialized = 1;
    }

/*
 * Initialize a Mersenne Twist PRNG from a 32-bit seed.
 *
 * According to Matsumoto and Nishimura's paper, the seed array needs to be
 * filled with nonzero values.  (My own interpretation is that there needs
 * to be at least one nonzero value).  They suggest using Knuth's PRNG from
 * Line 25, Table 1, p.102, "The Art of Computer Programming," Vol. 2 (2nd
 * ed.), 1981.  I find that rather odd, since that particular PRNG is
 * sensitive to having an initial seed of zero (there are many other PRNGs
 * out there that have an additive component, so that a seed of zero does
 * not generate a repeating-zero sequence).  However, one thing I learned
 * from reading Knuth is that you shouldn't second-guess mathematicians
 * about PRNGs.  Also, by following M & N's approach, we will be compatible
 * with other implementations.  So I'm going to stick with their version,
 * with the single addition that a zero seed will be changed to their
 * default seed.
 */
void mts_seed32(
    mt_state*		state,		/* State vector to initialize */
    uint32_t		seed)		/* 32-bit seed to start from */
    {
    int			i;		/* Loop index */

    if (seed == 0)
	seed = DEFAULT_SEED32_OLD;

    /*
     * Fill the state vector using Knuth's PRNG.  Be sure to mask down
     * to 32 bits in case we're running on a machine with 64-bit
     * ints.
     */
    state->statevec[MT_STATE_SIZE - 1] = seed & 0xffffffff;
    for (i = MT_STATE_SIZE - 2;  i >= 0;  i--)
        state->statevec[i] =
          (KNUTH_MULTIPLIER_OLD * state->statevec[i + 1]) & 0xffffffff;

    state->stateptr = MT_STATE_SIZE;
    mts_mark_initialized(state);

    /*
     * Matsumoto and Nishimura's implementation refreshes the PRNG
     * immediately after running the Knuth algorithm.  This is
     * probably a good thing, since Knuth's PRNG doesn't generate very
     * good numbers.
     */
    mts_refresh(state);
    }

/*
 * Initialize a Mersenne Twist PRNG from a 32-bit seed, using
 * Matsumoto and Nishimura's newer reference implementation (Jan. 9,
 * 2002).
 */
void mts_seed32new(
    mt_state*		state,		/* State vector to initialize */
    uint32_t		seed)		/* 32-bit seed to start from */
    {
    int			i;		/* Loop index */
    uint32_t		nextval;	/* Next value being calculated */

    /*
     * Fill the state vector using Knuth's PRNG.  Be sure to mask down
     * to 32 bits in case we're running on a machine with 64-bit
     * ints.
     */
    state->statevec[MT_STATE_SIZE - 1] = seed & 0xffffffffUL;
    for (i = MT_STATE_SIZE - 2;  i >= 0;  i--)
	{
	nextval = state->statevec[i + 1] >> KNUTH_SHIFT;
	nextval ^= state->statevec[i + 1];
	nextval *= KNUTH_MULTIPLIER_NEW;
	nextval += (MT_STATE_SIZE - 1) - i;
	state->statevec[i] = nextval & 0xffffffffUL;
	}

    state->stateptr = MT_STATE_SIZE;
    mts_mark_initialized(state);

    /*
     * Matsumoto and Nishimura's implementation refreshes the PRNG
     * immediately after running the Knuth algorithm.  This is
     * probably a good thing, since Knuth's PRNG doesn't generate very
     * good numbers.
     */
    mts_refresh(state);
    }

/*
 * Initialize a Mersenne Twist RNG from a 624-int seed.
 *
 * The 32-bit seeding routine given by Matsumoto and Nishimura has the
 * drawback that there are only 2^32 different PRNG sequences that can be
 * generated by calling that function.  This function solves that problem by
 * allowing a full 624*32-bit state to be given.  (Note that 31 bits of the
 * given state are ignored; see the paper for details.)
 *
 * Since an all-zero state would cause the PRNG to cycle, we detect
 * that case and abort the program (silently, since there is no
 * portable way to produce a message in both C and C++ environments).
 * An alternative would be to artificially force the state to some
 * known nonzero value.  However, I feel that if the user is providing
 * a full state, it's a bug to provide all zeros and we we shouldn't
 * conceal the bug by generating apparently correct output.
 */
void mts_seedfull(
    mt_state*		state,		/* State vector to initialize */
    uint32_t		seeds[MT_STATE_SIZE])
					/* Seed array to start from */
    {
    int			had_nz = 0;	/* NZ if at least one NZ seen */
    int			i;		/* Loop index */

    for (i = 0;  i < MT_STATE_SIZE;  i++)
        {
        if (seeds[i] != 0)
	    had_nz = 1;
        state->statevec[MT_STATE_SIZE - i - 1] = seeds[i];
	}

    if (!had_nz)
	{
	/*
	 * It would be nice to abort with a message.  Unfortunately, fprintf
	 * isn't compatible with all implementations of C++.  In the
	 * interest of C++ compatibility, therefore, we will simply abort
	 * silently.  It will unfortunately be up to a programmer to run
	 * under a debugger (or examine the core dump) to discover the cause
	 * of the abort.
	 */
	abort();
	}

    state->stateptr = MT_STATE_SIZE;
    mts_mark_initialized(state);
    }

/*
 * Choose a seed based on some moderately random input.  Prefers
 * /dev/urandom as a source of random numbers, but uses the lower bits
 * of the current time if /dev/urandom is not available.  In any case,
 * only provides 32 bits of entropy.
 */
uint32_t mts_seed(
    mt_state*		state)		/* State vector to seed */
    {
    return mts_devseed(state, DEVURANDOM);
    }

/*
 * Choose a seed based on some fairly random input.  Prefers
 * /dev/random as a source of random numbers, but uses the lower bits
 * of the current time if /dev/random is not available.  In any case,
 * only provides 32 bits of entropy.
 */
uint32_t mts_goodseed(
    mt_state*		state)		/* State vector to seed */
    {
    return mts_devseed(state, DEVRANDOM);
    }

/*
 * Choose a seed based on a random-number device given by the caller.
 * If that device can't be opened, use the lower 32 bits from the
 * current time.
 */
static uint32_t mts_devseed(
    mt_state*		state,		/* State vector to seed */
    char*		seed_dev)	/* Device to seed from */
    {
    int			bytesread;	/* Byte count read from device */
    int			nextbyte;	/* Index of next byte to read */
    FILE*		ranfile;	/* Access to device */
    union
	{
	char		ranbuffer[sizeof (uint32_t)];
					/* Space for reading random int */
	uint32_t	randomvalue;	/* Random value for initialization */
	}
			randomunion;	/* Union for reading random int */
#ifdef WIN32
    struct _timeb	tb;		/* Time of day (Windows mode) */
#else /* WIN32 */
    struct timeval	tv;		/* Time of day */
#endif /* WIN32 */

    ranfile = fopen(seed_dev, "rb");
    /*
     * Some machines have /dev/random but not /dev/urandom.  On those
     * machines, /dev/random is nonblocking, so we'll try it before we
     * fall back to using the time.
     */
    if (ranfile == NULL)
	ranfile = fopen(DEVRANDOM, "rb");
    if (ranfile != NULL)
	{
	setbuf(ranfile, NULL);
	for (nextbyte = 0;
	  nextbyte < (int)sizeof randomunion.ranbuffer;
	  nextbyte += bytesread)
	    {
	    bytesread = fread(&randomunion.ranbuffer[nextbyte], (size_t)1,
	      sizeof randomunion.ranbuffer - nextbyte, ranfile);
	    if (bytesread == 0)
		break;
	    }
	fclose(ranfile);
	if (nextbyte == sizeof randomunion.ranbuffer)
	    {
	    mts_seed32new(state, randomunion.randomvalue);
	    return randomunion.randomvalue;
	    }
	}

    /*
     * The device isn't available.  Use the time.  We will
     * assume that the time of day is accurate to microsecond
     * resolution, which is true on most modern machines.
     */
#ifdef WIN32
    (void) _ftime (&tb);
#else /* WIN32 */
    (void) gettimeofday (&tv, NULL);
#endif /* WIN32 */

    /*
     * We just let the excess part of the seconds field overflow
     */
#ifdef WIN32
    randomunion.randomvalue = tb.time * 1000 + tb.millitm;
#else /* WIN32 */
    randomunion.randomvalue = tv.tv_sec * 1000000 + tv.tv_usec;
#endif /* WIN32 */
    mts_seed32new(state, randomunion.randomvalue);
    return randomunion.randomvalue;
    }

/*
 * Choose a seed based on the best random input available.  Prefers
 * /dev/random as a source of random numbers, and reads the entire
 * 624-int state from that device.  Because of this approach, the
 * function can take a long time (in real time) to complete, since
 * /dev/random may have to wait quite a while before it can provide
 * that much randomness.  If /dev/random is unavailable, falls back to
 * calling mts_goodseed.
 */
void mts_bestseed(
    mt_state*		state)		/* State vector to seed */
    {
    int			bytesread;	/* Byte count read from device */
    int			nextbyte;	/* Index of next byte to read */
    FILE*		ranfile;	/* Access to device */

    ranfile = fopen("/dev/random", "rb");
    if (ranfile == NULL)
	{
	mts_goodseed(state);
	return;
	}

    for (nextbyte = 0;
      nextbyte < (int)sizeof state->statevec;
      nextbyte += bytesread)
	{
	bytesread = fread((char *)&state->statevec[0] + nextbyte, (size_t)1,
	  sizeof state->statevec - nextbyte, ranfile);
	if (bytesread == 0)
	    {
	    /*
	     * Something went wrong.  Fall back to time-based seeding.
	     */
	    fclose(ranfile);
	    mts_goodseed(state);
	    return;
	    }
	}
    fclose(ranfile);
    }

/*
 * Generate 624 more random values.  This function is called when the
 * state vector has been exhausted.  It generates another batch of
 * pseudo-random values.  The performance of this function is critical
 * to the performance of the Mersenne Twist PRNG, so it has been
 * highly optimized.
 */
void mts_refresh(
    register mt_state*	state)		/* State for the PRNG */
    {
    register int	i;		/* Index into the state */
    register uint32_t*
			state_ptr;	/* Next place to get from state */
    register uint32_t
			value1;		/* Scratch val picked up from state */
    register uint32_t
			value2;		/* Scratch val picked up from state */

    /*
     * Start by making sure a random seed has been set.  If not, set
     * one.
     */
    if (!state->initialized)
	{
	mts_seed32(state, DEFAULT_SEED32_OLD);
	return;				/* Seed32 calls us recursively */
	}

    /*
     * Now generate the new pseudorandom values by applying the
     * recurrence relation.  We use two loops and a final
     * 2-statement sequence so that we can handle the wraparound
     * explicitly, rather than having to use the relatively slow
     * modulus operator.
     *
     * In essence, the recurrence relation concatenates bits
     * chosen from the current random value (last time around)
     * with the immediately preceding one.  Then it
     * matrix-multiplies the concatenated bits with a value
     * RECURRENCE_OFFSET away and a constant matrix.  The matrix
     * multiplication reduces to a shift and two XORs.
     *
     * Some comments on the optimizations are in order:
     *
     * Strictly speaking, none of the optimizations should be
     * necessary.  All could conceivably be done by a really good
     * compiler.  However, the compilers available to me aren't quite
     * smart enough, so hand optimization needs to be done.
     *
     * Shawn Cokus was the first to achieve a major speedup.  In the
     * original code, the first value given to COMBINE_BITS (in my
     * characterization) was re-fetched from the state array, rather
     * than being carried in a scratch variable.  Cokus noticed that
     * the first argument to COMBINE_BITS could be saved in a register
     * in the previous loop iteration, getting rid of the need for an
     * expensive memory reference.
     *
     * Cokus also switched to using pointers to access the state
     * array and broke the original loop into two so that he could
     * avoid using the expensive modulus operator.  Cokus used three
     * pointers; Richard J. Wagner noticed that the offsets between
     * the three were constant, so that they could be collapsed into a
     * single pointer and constant-offset accesses.  This is clearly
     * faster on x86 architectures, and is the same cost on RISC
     * machines.  A secondary benefit is that Cokus' version was
     * register-starved on the x86, while Wagner's version was not.
     *
     * I made several smaller improvements to these observations.
     * First, I reversed the contents of the state vector.  In the
     * current version of the code, this change doesn't directly
     * affect the performance of the refresh loop, but it has the nice
     * side benefit that an all-zero state structure represents an
     * uninitialized generator.  It also slightly speeds up the
     * random-number routines, since they can compare the state
     * pointer against zero instead of against a constant (this makes
     * the biggest difference on RISC machines).
     *
     * Second, I returned to Matsumoto and Nishimura's original
     * technique of using a lookup table to decide whether to xor the
     * constant vector A (MATRIX_A in this code) with the newly
     * computed value.  Cokus and Wagner had used the ?: operator,
     * which requires a test and branch.  Modern machines don't like
     * branches, so the table lookup is faster.
     *
     * Third, in the Cokus and Wagner versions the loop ends with a
     * statement similar to "value1 = value2", which is necessary to
     * carry the fetched value into the next loop iteration.  I
     * recognized that if the loop were unrolled so that it generates
     * two values per iteration, a bit of variable renaming would get
     * rid of that assignment.  A nice side effect is that the
     * overhead of loop control becomes only half as large.
     *
     * It is possible to improve the code's performance somewhat
     * further.  In particular, since the second loop's loop count
     * factors into 2*2*3*3*11, it could be unrolled yet further.
     * That's easy to do, too: just change the "/ 2" into a division
     * by whatever factor you choose, and then use cut-and-paste to
     * duplicate the code in the body.  To remove a few more cycles,
     * fix the code to decrement state_ptr by the unrolling factor, and
     * adjust the various offsets appropriately.  However, the payoff
     * will be small.  At the moment, the x86 version of the loop is
     * 25 instructions, of which 3 are involved in loop control
     * (including the decrementing of state_ptr).  Further unrolling by
     * a factor of 2 would thus produce only about a 6% speedup.
     *
     * The logical extension of the unrolling
     * approach would be to remove the loops and create 624
     * appropriate copies of the body.  However, I think that doing
     * the latter is a bit excessive!
     *
     * I suspect that a superior optimization would be to simplify the
     * mathematical operations involved in the recurrence relation.
     * However, I have no idea whether such a simplification is
     * feasible.
     */
    state_ptr = &state->statevec[MT_STATE_SIZE - 1];
    value1 = *state_ptr;
    for (i = (MT_STATE_SIZE - RECURRENCE_OFFSET) / 2;  --i >= 0;  )
	{
	state_ptr -= 2;
	value2 = state_ptr[1];
	value1 = COMBINE_BITS(value1, value2);
	state_ptr[2] =
	  MATRIX_MULTIPLY(state_ptr[-RECURRENCE_OFFSET + 2], value1);
	value1 = state_ptr[0];
	value2 = COMBINE_BITS(value2, value1);
	state_ptr[1] =
	  MATRIX_MULTIPLY(state_ptr[-RECURRENCE_OFFSET + 1], value2);
	}
    value2 = *--state_ptr;
    value1 = COMBINE_BITS(value1, value2);
    state_ptr[1] =
      MATRIX_MULTIPLY(state_ptr[-RECURRENCE_OFFSET + 1], value1);

    for (i = (RECURRENCE_OFFSET - 1) / 2;  --i >= 0;  )
	{
	state_ptr -= 2;
	value1 = state_ptr[1];
	value2 = COMBINE_BITS(value2, value1);
	state_ptr[2] =
	  MATRIX_MULTIPLY(state_ptr[MT_STATE_SIZE - RECURRENCE_OFFSET + 2],
	    value2);
	value2 = state_ptr[0];
	value1 = COMBINE_BITS(value1, value2);
	state_ptr[1] =
	  MATRIX_MULTIPLY(state_ptr[MT_STATE_SIZE - RECURRENCE_OFFSET + 1],
	    value1);
	}

    /*
     * The final entry in the table requires the "previous" value
     * to be gotten from the other end of the state vector, so it
     * must be handled specially.
     */
    value1 = COMBINE_BITS(value2, state->statevec[MT_STATE_SIZE - 1]);
    *state_ptr =
      MATRIX_MULTIPLY(state_ptr[MT_STATE_SIZE - RECURRENCE_OFFSET], value1);

    /*
     * Now that refresh is complete, reset the state pointer to allow more
     * pseudorandom values to be fetched from the state array.
     */
    state->stateptr = MT_STATE_SIZE;
    }

/*
 * Save state to a file.  The save format is compatible with Richard
 * J. Wagner's format, although the details are different.  Returns NZ
 * if the save succeeded.  Produces one very long line containing 625
 * numbers.
 */
int mts_savestate(
    FILE*		statefile,	/* File to save to */
    mt_state*		state)		/* State to be saved */
    {
    int			i;		/* Next word to save */

    if (!state->initialized)
	mts_seed32(state, DEFAULT_SEED32_OLD);

    /*
     * Ensure the state pointer is valid.
     */
    if (state->stateptr < 0  ||  state->stateptr > MT_STATE_SIZE)
	{
	fprintf(stderr,
	  "Mtwist internal: Trying to write invalid state pointer %d\n",
	  state->stateptr);
	mts_refresh(state);
	}

    for (i = MT_STATE_SIZE;  --i >= 0;  )
	{
	if (fprintf(statefile, "%" PRIu32 " ", state->statevec[i]) < 0)
	    return 0;
	}

    if (fprintf(statefile, "%d\n", state->stateptr) < 0)
	return 0;

    return 1;
    }

/*
 * Load state from a file.  Returns NZ if the load succeeded.
 */
int mts_loadstate(
    FILE*		statefile,	/* File to load from */
    mt_state*		state)		/* State to be loaded */
    {
    int			i;		/* Next word to load */

    /*
     * Set the state to "uninitialized" in case the load fails.
     */
    state->initialized = state->stateptr = 0;

    for (i = MT_STATE_SIZE;  --i >= 0;  )
	{
	if (fscanf(statefile, "%" SCNu32, &state->statevec[i]) != 1)
	    return 0;
	}

    if (fscanf(statefile, "%d", &state->stateptr) != 1)
	return 0;

    /*
     * The only validity checking we can do is to insist that the
     * state pointer be valid.
     */
    if (state->stateptr < 0  ||  state->stateptr > MT_STATE_SIZE)
	{
	state->stateptr = 0;
	return 0;
	}

    mts_mark_initialized(state);

    return 1;
    }

/*
 * Initialize the default Mersenne Twist PRNG from a 32-bit seed.
 *
 * See mts_seed32 for full commentary.
 */
void mt_seed32(
    uint32_t		seed)		/* 32-bit seed to start from */
    {
    mts_seed32(&mt_default_state, seed);
    }

/*
 * Initialize the default Mersenne Twist PRNG from a 32-bit seed.
 *
 * See mts_seed32new for full commentary.
 */
void mt_seed32new(
    uint32_t		seed)		/* 32-bit seed to start from */
    {
    mts_seed32new(&mt_default_state, seed);
    }

/*
 * Initialize a Mersenne Twist RNG from a 624-int seed.
 *
 * See mts_seedfull for full commentary.
 */
void mt_seedfull(
    uint32_t		seeds[MT_STATE_SIZE])
    {
    mts_seedfull(&mt_default_state, seeds);
    }

/*
 * Initialize the PRNG from random input.  See mts_seed.
 */
uint32_t mt_seed(void)
    {
    return mts_seed(&mt_default_state);
    }

/*
 * Initialize the PRNG from random input.  See mts_goodseed.
 */
uint32_t mt_goodseed(void)
    {
    return mts_goodseed(&mt_default_state);
    }

/*
 * Initialize the PRNG from random input.  See mts_bestseed.
 */
void mt_bestseed(void)
    {
    mts_bestseed(&mt_default_state);
    }

/*
 * Return a pointer to the current state of the PRNG.  The purpose of
 * this function is to allow the state to be saved for later
 * restoration.  The state should not be modified; instead, it should
 * be reused later as a parameter to one of the mts_xxx functions.
 */
mt_state* mt_getstate(void)
    {
    return &mt_default_state;
    }

/*
 * Save state to a file.  The save format is compatible with Richard
 * J. Wagner's format, although the details are different.
 */
int mt_savestate(
    FILE*		statefile)	/* File to save to */
    {
    return mts_savestate(statefile, &mt_default_state);
    }

/*
 * Load state from a file.
 */
int mt_loadstate(
    FILE*		statefile)	/* File to load from */
    {
    return mts_loadstate(statefile, &mt_default_state);
    }
