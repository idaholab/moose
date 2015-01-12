#ifndef lint
#ifdef __GNUC__
#define ATTRIBUTE(attrs) __attribute__(attrs)
#else
#define ATTRIBUTE(attrs)
#endif
static char Rcs_Id[] ATTRIBUTE((used)) =
    "$Id: randistrs.c,v 1.12 2013-01-05 01:18:52-08 geoff Exp $";
#endif

/*
 * C library functions for generating various random distributions
 * using the Mersenne Twist PRNG.  See the header file for full
 * documentation.
 *
 * These functions were written by Geoff Kuenning, Claremont, CA.
 *
 * Unless otherwise specified, these algorithms are taken from Averill
 * M. Law and W. David Kelton, "Simulation Modeling and Analysis",
 * McGraw-Hill, 1991.
 *
 * IMPORTANT NOTE: By default, this code is reentrant.  If you are
 * certain you don't need reentrancy, you can get a bit more speed by
 * defining MT_CACHING.
 *
 * Copyright 2001, 2002, 2010, Geoffrey H. Kuenning, Claremont, CA.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All modifications to the source code must be clearly marked as
 *    such.  Binary redistributions based on modified source code
 *    must be clearly marked as modified versions in the documentation
 *    and/or other materials provided with the distribution.
 * 4. The name of Geoff Kuenning may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY GEOFF KUENNING AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL GEOFF KUENNING OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Log: randistrs.c,v $
 * Revision 1.12  2013-01-05 01:18:52-08  geoff
 * Fix a lot of compiler warnings.  Allow rd_empirical_setup to take
 * const arguments.
 *
 * Revision 1.11  2012-12-30 16:24:49-08  geoff
 * Use gcc attributes to suppress warnings on Rcs_Id.
 *
 * Revision 1.10  2010-12-10 03:28:19-08  geoff
 * Rewrite the empirical-distribution interface to run in O(1) time and
 * to provide a continuous approximation to empirical distributions.
 *
 * Revision 1.9  2010-06-24 20:53:59+12  geoff
 * Switch to using types from stdint.h.  Make reentrancy the default.
 *
 * Revision 1.8  2008-07-25 16:34:01-07  geoff
 * Fix notation for intervals in commentary.
 *
 * Revision 1.7  2005/05/17 21:40:10  geoff
 * Fix a bug that caused rds_iuniform to generate off-by-one values if the
 * lower bound was negative.
 *
 * Revision 1.6  2002/10/30 00:50:44  geoff
 * Add a (BSD-style) license.  Fix all places where logs are taken so
 * that there is no risk of unintentionally taking the log of zero.  This
 * is a very low-probability occurrence, but it's better to have robust
 * code.
 *
 * Revision 1.5  2001/06/20 09:07:57  geoff
 * Fix a place where long long wasn't conditionalized.
 *
 * Revision 1.4  2001/06/19 00:41:17  geoff
 * Add the "l" versions of all functions.  Add the MT_NO_CACHING option.
 *
 * Revision 1.3  2001/06/18 10:09:24  geoff
 * Add the iuniform functions to generate unbiased uniformly distributed
 * integers.
 *
 * Revision 1.2  2001/04/10 09:11:38  geoff
 * Make sure the Erlang distribution has a p of 1 or more.  Fix a serious
 * bug in the Erlang calculation (the value returned was completely
 * wrong).
 *
 * Revision 1.1  2001/04/09 08:39:54  geoff
 * Initial revision
 *
 */

#undef MT_GENERATE_CODE_IN_HEADER
#define MT_GENERATE_CODE_IN_HEADER 0
#include "mtwist.h"
#include "randistrs.h"
#include <math.h>
#include <stdlib.h>

/*
 * Threshold below which it is OK for uniform integer distributions to make
 * use of the double-precision code as a crutch.  For ranges below
 * this value, a double-precision random value is generated and then
 * mapped to the given range.  For a lower bound of zero, this is
 * equivalent to mapping a 32-bit integer into the range by using the
 * following formula:
 *
 *	final = upper * mt_lrand() / (1 << 32);
 *
 * That formula can't be computed using integer arithmetic, since the
 * multiplication must precede the division and would cause overflow.
 * Double-precision calculations solve that problem.  However the
 * formula will also produce biased results unless the range ("upper")
 * is exactly a power of 2.  To see this, suppose mt_lrand produced
 * values from 0 to 7 (i.e., 8 values), and we asked for numbers in
 * the range [0, 7).  The 8 values uniformly generated by mt_lrand
 * would be mapped into the 7 output values.  Clearly, one output
 * value (in this case, 4) would occur twice as often as the others
 *
 * The amount of bias introduced by this approximation depends on the
 * relative sizes of the requested range and the range of values
 * produced by mt_lrand.  If the ranges are almost equal, some values
 * will occur almost twice as often as they should.  At the other
 * extreme, consider a requested range of 3 values (0 to 2,
 * inclusive).  If the PRNG cycles through all 2^32 possible values,
 * two of the output values will be generated 1431655765 times and the
 * third will appear 1431655766 times.  Clearly, the bias here is
 * within the expected limits of randomness.
 *
 * The exact amount of bias depends on the relative size of the range
 * compared to the width of the PRNG output.  In general, for an
 * output range of r, no value will appear more than r/(2^32) extra
 * times using the simple integer algorithm.
 *
 * The threshold given below will produce a bias of under 0.01%.  For
 * values above this threshold, a slower but 100% accurate algorithm
 * will be used.
 */
#ifndef RD_MAX_BIAS
#define RD_MAX_BIAS		0.0001
#endif /* RD_MAX_BIAS */
#ifndef RD_UNIFORM_THRESHOLD
#define RD_UNIFORM_THRESHOLD	((int)((double)(1u << 31) * 2.0 * RD_MAX_BIAS))
#endif /* RD_UNIFORM_THRESHOLD */

/*
 * Generate a uniform integer distribution on the open interval
 * [lower, upper).  See comments above about RD_UNIFORM_THRESHOLD.  If
 * we are above the threshold, this function is relatively expensive
 * because we may have to repeatedly draw random numbers to get a
 * one that works.
 */
int32_t rds_iuniform(
    mt_state *		state,		/* State of the MT PRNG to use */
    int32_t		lower,		/* Lower limit of distribution */
    int32_t		upper)		/* Upper limit of distribution */
    {
    uint32_t		range = upper - lower;
					/* Range of requested distribution */

    if (range <= RD_UNIFORM_THRESHOLD)
	return lower + (int32_t)(mts_ldrand(state) * range);
    else
	{
	/*
	 * Using the simple formula would produce too much bias.
	 * Instead, draw numbers until we get one within the range.
	 * To save time, we first calculate a mask so that we only
	 * look at the number of bits we actually need.  Since finding
	 * the mask is expensive, we optionally do a bit of caching
	 * here (note that the caching makes the code non-reentrant;
	 * set MT_CACHING to turn on this misfeature).
	 *
	 * Incidentally, the astute reader will note that we use the
	 * low-order bits of the PRNG output.  If the PRNG were linear
	 * congruential, using the low-order bits wouuld be a major
	 * no-no.  However, the Mersenne Twist PRNG doesn't have that
	 * drawback.
	 */
#ifdef MT_CACHING
	static uint32_t	lastrange = 0;	/* Range used last time */
	static uint32_t	rangemask = 0;	/* Mask for range */
#else /* MT_CACHING */
	uint32_t	rangemask = 0;	/* Mask for range */
#endif /* MT_CACHING */
	register uint32_t
			ranval;		/* Random value from mts_lrand */

#ifdef MT_CACHING
	if (range != lastrange)
#endif /* MT_CACHING */
	    {
	    /*
	     * Range is different from last time, recalculate mask.
	     *
	     * A few iterations could be trimmed off of the loop if we
	     * started rangemask at the next power of 2 above
	     * RD_UNIFORM_THRESHOLD.  However, I don't currently know
	     * a formula for generating that value (though there is
	     * probably one in HAKMEM).
	     */
#ifdef MT_CACHING
	    lastrange = range;
#endif /* MT_CACHING */
	    for (rangemask = 1;
	      rangemask < range  &&  rangemask != 0;
	      rangemask <<= 1)
		;

	    /*
	     * If rangemask became zero, the range is over 2^31.  In
	     * that case, subtracting 1 from rangemask will produce a
	     * full-word mask, which is what we need.
	     */
	    rangemask -= 1;
	    }

	/*
	 * Draw random numbers until we get one in the requested range.
	 */
	do
	    {
	    ranval = mts_lrand(state) & rangemask;
	    }
	    while (ranval >= range);
	return lower + ranval;
	}
    }

#ifdef INT64_MAX
/*
 * Generate a uniform integer distribution on the half-open interval
 * [lower, upper).
 */
int64_t rds_liuniform(
    mt_state *		state,		/* State of the MT PRNG to use */
    int64_t		lower,		/* Lower limit of distribution */
    int64_t		upper)		/* Upper limit of distribution */
    {
    uint64_t		range = upper - lower;
					/* Range of requested distribution */

    /*
     * Draw numbers until we get one within the range.  To save time,
     * we first calculate a mask so that we only look at the number of
     * bits we actually need.  Since finding the mask is expensive, we
     * optionally do a bit of caching here.  See rds_iuniform for more
     * information.
     */
#ifdef MT_CACHING
    static uint32_t	lastrange = 0;	/* Range used last time */
    static uint32_t	rangemask = 0;	/* Mask for range */
#else /* MT_CACHING */
    uint32_t		rangemask = 0;	/* Mask for range */
#endif /* MT_CACHING */
    register uint32_t	ranval;		/* Random value from mts_lrand */

#ifdef MT_CACHING
    if (range != lastrange)
#endif /* MT_CACHING */
	{
	/*
	 * Range is different from last time, recalculate mask.
	 */
#ifdef MT_CACHING
	lastrange = range;
#endif /* MT_CACHING */
	for (rangemask = 1;
	  rangemask < range  &&  rangemask != 0;
	  rangemask <<= 1)
	    ;

	/*
	 * If rangemask became zero, the range is over 2^31.  In
	 * that case, subtracting 1 from rangemask will produce a
	 * full-word mask, which is what we need.
	 */
	rangemask -= 1;
	}

    /*
     * Draw random numbers until we get one in the requested range.
     */
    do
	{
	ranval = mts_llrand(state) & rangemask;
	}
	while (ranval >= range);
    return lower + ranval;
    }
#endif /* INT64_MAX */

/*
 * Generate a uniform distribution on the half-open interval [lower, upper).
 */
double rds_uniform(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		lower,		/* Lower limit of distribution */
    double		upper)		/* Upper limit of distribution */
    {
    return lower + mts_drand(state) * (upper - lower);
    }

/*
 * Generate a uniform distribution on the half-open interval [lower, upper).
 */
double rds_luniform(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		lower,		/* Lower limit of distribution */
    double		upper)		/* Upper limit of distribution */
    {
    return lower + mts_ldrand(state) * (upper - lower);
    }

/*
 * Generate an exponential distribution with the given mean.
 */
double rds_exponential(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		mean)		/* Mean of generated distribution */
    {
    double		random_value;	/* Random sample on [0,1) */

    do
	random_value = mts_drand(state);
    while (random_value == 0.0);
    return -mean * log(random_value);
    }

/*
 * Generate an exponential distribution with the given mean.
 */
double rds_lexponential(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		mean)		/* Mean of generated distribution */
    {
    double		random_value;	/* Random sample on [0,1) */

    do
	random_value = mts_ldrand(state);
    while (random_value == 0.0);
    return -mean * log(random_value);
    }

/*
 * Generate a p-Erlang distribution with the given mean.
 */
double rds_erlang(
    mt_state *		state,		/* State of the MT PRNG to use */
    int			p,		/* Order of distribution to generate */
    double		mean)		/* Mean of generated distribution */
    {
    int			order;		/* Order generated so far */
    double		random_value;	/* Value generated so far */

    do
	{
	if (p <= 1)
	    p = 1;
	random_value = mts_drand(state);
	for (order = 1;  order < p;  order++)
	    random_value *= mts_drand(state);
	}
    while (random_value == 0.0);
    return -mean * log(random_value) / p;
    }

/*
 * Generate a p-Erlang distribution with the given mean.
 */
double rds_lerlang(
    mt_state *		state,		/* State of the MT PRNG to use */
    int			p,		/* Order of distribution to generate */
    double		mean)		/* Mean of generated distribution */
    {
    int			order;		/* Order generated so far */
    double		random_value;	/* Value generated so far */

    do
	{
	if (p <= 1)
	    p = 1;
	random_value = mts_ldrand(state);
	for (order = 1;  order < p;  order++)
	    random_value *= mts_ldrand(state);
	}
    while (random_value == 0.0);
    return -mean * log(random_value) / p;
    }

/*
 * Generate a Weibull distribution with the given shape and scale parameters.
 */
double rds_weibull(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		shape,		/* Shape of the distribution */
    double		scale)		/* Scale of the distribution */
    {
    double		random_value;	/* Random sample on [0,1) */

    do
	random_value = mts_drand(state);
    while (random_value == 0.0);
    return scale * exp(log(-log(random_value)) / shape);
    }
					/* Weibull distribution */
/*
 * Generate a Weibull distribution with the given shape and scale parameters.
 */
double rds_lweibull(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		shape,		/* Shape of the distribution */
    double		scale)		/* Scale of the distribution */
    {
    double		random_value;	/* Random sample on [0,1) */

    do
	random_value = mts_ldrand(state);
    while (random_value == 0.0);
    return scale * exp(log(-log(random_value)) / shape);
    }
					/* Weibull distribution */
/*
 * Generate a normal distribution with the given mean and standard
 * deviation.  See Law and Kelton, p. 491.
 */
double rds_normal(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		mean,		/* Mean of generated distribution */
    double		sigma)		/* Standard deviation to generate */
    {
    double		mag;		/* Magnitude of (x,y) point */
    double		offset;		/* Unscaled offset from mean */
    double		xranval;	/* First random value on [-1,1) */
    double		yranval;	/* Second random value on [-1,1) */

    /*
     * Generating a normal distribution is a bit tricky.  We may need
     * to make several attempts before we get a valid result.  When we
     * are done, we will have two normally distributed values, one of
     * which we discard.
     */
    do
	{
	xranval = 2.0 * mts_drand(state) - 1.0;
	yranval = 2.0 * mts_drand(state) - 1.0;
	mag = xranval * xranval + yranval * yranval;
	}
    while (mag > 1.0  ||  mag == 0.0);

    offset = sqrt((-2.0 * log(mag)) / mag);
    return mean + sigma * xranval * offset;

    /*
     * The second random variate is given by:
     *
     *     mean + sigma * yranval * offset;
     *
     * If this were a C++ function, it could probably save that value
     * somewhere and return it in the next subsequent call.  But
     * that's too hard to make bulletproof (and reentrant) in C.
     */
    }

/*
 * Generate a normal distribution with the given mean and standard
 * deviation.  See Law and Kelton, p. 491.
 */
double rds_lnormal(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		mean,		/* Mean of generated distribution */
    double		sigma)		/* Standard deviation to generate */
    {
    double		mag;		/* Magnitude of (x,y) point */
    double		offset;		/* Unscaled offset from mean */
    double		xranval;	/* First random value on [-1,1) */
    double		yranval;	/* Second random value on [-1,1) */

    /*
     * Generating a normal distribution is a bit tricky.  We may need
     * to make several attempts before we get a valid result.  When we
     * are done, we will have two normally distributed values, one of
     * which we discard.
     */
    do
	{
	xranval = 2.0 * mts_ldrand(state) - 1.0;
	yranval = 2.0 * mts_ldrand(state) - 1.0;
	mag = xranval * xranval + yranval * yranval;
	}
    while (mag > 1.0  ||  mag == 0.0);

    offset = sqrt((-2.0 * log(mag)) / mag);
    return mean + sigma * xranval * offset;

    /*
     * The second random variate is given by:
     *
     *     mean + sigma * yranval * offset;
     *
     * If this were a C++ function, it could probably save that value
     * somewhere and return it in the next subsequent call.  But
     * that's too hard to make bulletproof (and reentrant) in C.
     */
    }

/*
 * Generate a lognormal distribution with the given shape and scale
 * parameters.
 */
double rds_lognormal(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		shape,		/* Shape of the distribution */
    double		scale)		/* Scale of the distribution */
    {
    return exp(rds_normal(state, scale, shape));
    }

/*
 * Generate a lognormal distribution with the given shape and scale
 * parameters.
 */
double rds_llognormal(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		shape,		/* Shape of the distribution */
    double		scale)		/* Scale of the distribution */
    {
    return exp(rds_lnormal(state, scale, shape));
    }

/*
 * Generate a triangular distibution between given limits, with a
 * given mode.
 */
double rds_triangular(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		lower,		/* Lower limit of distribution */
    double		upper,		/* Upper limit of distribution */
    double		mode)		/* Highest point of distribution */
    {
    double		ran_value;	/* Value generated by PRNG */
    double		scaled_mode;	/* Scaled version of mode */

    scaled_mode = (mode - lower) / (upper - lower);
    ran_value = mts_drand(state);
    if (ran_value <= scaled_mode)
	ran_value = sqrt(scaled_mode * ran_value);
    else
	ran_value = 1.0 - sqrt((1.0 - scaled_mode) * (1.0 - ran_value));
    return lower + (upper - lower) * ran_value;
    }

/*
 * Generate a triangular distibution between given limits, with a
 * given mode.
 */
double rds_ltriangular(
    mt_state *		state,		/* State of the MT PRNG to use */
    double		lower,		/* Lower limit of distribution */
    double		upper,		/* Upper limit of distribution */
    double		mode)		/* Highest point of distribution */
    {
    double		ran_value;	/* Value generated by PRNG */
    double		scaled_mode;	/* Scaled version of mode */

    scaled_mode = (mode - lower) / (upper - lower);
    ran_value = mts_ldrand(state);
    if (ran_value <= scaled_mode)
	ran_value = sqrt(scaled_mode * ran_value);
    else
	ran_value = 1.0 - sqrt((1.0 - scaled_mode) * (1.0 - ran_value));
    return lower + (upper - lower) * ran_value;
    }

/*
 * Generate a discrete integer empirical distribution given a set of
 * probability cutoffs.  See rd_empirical_setup for full information.
 */
size_t rds_int_empirical(
    mt_state *		state,		/* State of the MT PRNG to use */
    rd_empirical_control* control)	/* Control from rd_empirical_setup */
    {
    double		ran_value;	/* Value generated by PRNG */
    size_t		result;		/* Result we'll return */

    ran_value = mts_ldrand(state);
    ran_value *= control->n;		/* Scale value to required range */
    result = (size_t)ran_value;		/* Integer part MIGHT be result */
    if (ran_value < control->cutoff[result]) /* Correct probability? */
	return result;			/* Done! */
    else
	return control->remap[result];	/* Nope, remap to correct result */
    }

/*
 * Generate a discrete floating-point empirical distribution given a
 * set of probability cutoffs.  Use the result of rds_int_empirical to
 * choose a final value.
 */
double rds_double_empirical(
    mt_state *		state,		/* State of the MT PRNG to use */
    rd_empirical_control* control)	/* Control from rd_empirical_setup */
    {
    return control->values[rds_int_empirical(state, control)];
    }

/*
 * Generate a continuous floating-point empirical distribution given a
 * set of probability cutoffs.  Use the result of rds_int_empirical to
 * choose a pair of values, and then return a uniform distribution
 * between those two values.
 */
double rds_continuous_empirical(
    mt_state *		state,		/* State of the MT PRNG to use */
    rd_empirical_control* control)	/* Control from rd_empirical_setup */
    {
    size_t		index;		/* Index into values table */

    index = rds_int_empirical(state, control);
    return control->values[index]
      + mts_ldrand(state)
	* (control->values[index + 1] - control->values[index]);
    }

/*
 * Generate a uniform integer distribution on the half-open interval
 * [lower, upper).  See comments on rds_iuniform.
 */
int32_t rd_iuniform(
    int32_t		lower,		/* Lower limit of distribution */
    int32_t		upper)		/* Upper limit of distribution */
    {
    return rds_iuniform(&mt_default_state, lower, upper);
    }

#ifdef INT64_MAX
/*
 * Generate a uniform integer distribution on the open interval
 * [lower, upper).  See comments on rds_iuniform.
 */
int64_t rd_liuniform(
    int64_t		lower,		/* Lower limit of distribution */
    int64_t		upper)		/* Upper limit of distribution */
    {
    return rds_liuniform(&mt_default_state, lower, upper);
    }
#endif /* INT64_MAX */

/*
 * Generate a uniform distribution on the open interval [lower, upper).
 */
double rd_uniform(
    double		lower,		/* Lower limit of distribution */
    double		upper)		/* Upper limit of distribution */
    {
    return rds_uniform (&mt_default_state, lower, upper);
    }

/*
 * Generate a uniform distribution on the open interval [lower, upper).
 */
double rd_luniform(
    double		lower,		/* Lower limit of distribution */
    double		upper)		/* Upper limit of distribution */
    {
    return rds_luniform (&mt_default_state, lower, upper);
    }

/*
 * Generate an exponential distribution with the given mean.
 */
double rd_exponential(
    double		mean)		/* Mean of generated distribution */
    {
    return rds_exponential (&mt_default_state, mean);
    }

/*
 * Generate an exponential distribution with the given mean.
 */
double rd_lexponential(
    double		mean)		/* Mean of generated distribution */
    {
    return rds_lexponential (&mt_default_state, mean);
    }

/*
 * Generate a p-Erlang distribution with the given mean.
 */
double rd_erlang(
    int			p,		/* Order of distribution to generate */
    double		mean)		/* Mean of generated distribution */
    {
    return rds_erlang (&mt_default_state, p, mean);
    }

/*
 * Generate a p-Erlang distribution with the given mean.
 */
double rd_lerlang(
    int			p,		/* Order of distribution to generate */
    double		mean)		/* Mean of generated distribution */
    {
    return rds_lerlang (&mt_default_state, p, mean);
    }

/*
 * Generate a Weibull distribution with the given shape and scale parameters.
 */
double rd_weibull(
    double		shape,		/* Shape of the distribution */
    double		scale)		/* Scale of the distribution */
    {
    return rds_weibull (&mt_default_state, shape, scale);
    }

/*
 * Generate a Weibull distribution with the given shape and scale parameters.
 */
double rd_lweibull(
    double		shape,		/* Shape of the distribution */
    double		scale)		/* Scale of the distribution */
    {
    return rds_lweibull (&mt_default_state, shape, scale);
    }

/*
 * Generate a normal distribution with the given mean and standard
 * deviation.  See Law and Kelton, p. 491.
 */
double rd_normal(
    double		mean,		/* Mean of generated distribution */
    double		sigma)		/* Standard deviation to generate */
    {
    return rds_normal (&mt_default_state, mean, sigma);
    }

/*
 * Generate a normal distribution with the given mean and standard
 * deviation.  See Law and Kelton, p. 491.
 */
double rd_lnormal(
    double		mean,		/* Mean of generated distribution */
    double		sigma)		/* Standard deviation to generate */
    {
    return rds_lnormal (&mt_default_state, mean, sigma);
    }

/*
 * Generate a lognormal distribution with the given shape and scale
 * parameters.
 */
double rd_lognormal(
    double		shape,		/* Shape of the distribution */
    double		scale)		/* Scale of the distribution */
    {
    return rds_lognormal (&mt_default_state, shape, scale);
    }

/*
 * Generate a lognormal distribution with the given shape and scale
 * parameters.
 */
double rd_llognormal(
    double		shape,		/* Shape of the distribution */
    double		scale)		/* Scale of the distribution */
    {
    return rds_llognormal (&mt_default_state, shape, scale);
    }

/*
 * Generate a triangular distibution between given limits, with a
 * given mode.
 */
double rd_triangular(
    double		lower,		/* Lower limit of distribution */
    double		upper,		/* Upper limit of distribution */
    double		mode)
    {
    return rds_triangular (&mt_default_state, lower, upper, mode);
    }

/*
 * Generate a triangular distibution between given limits, with a
 * given mode.
 */
double rd_ltriangular(
    double		lower,		/* Lower limit of distribution */
    double		upper,		/* Upper limit of distribution */
    double		mode)
    {
    return rds_ltriangular (&mt_default_state, lower, upper, mode);
    }

/*
 * Set up to calculate an empirical distribution in O(1) time.  The
 * method used is adapted from Alastair J. Walker, "An efficient
 * method for generating discrete random variables with general
 * distributions", ACM Transactions on Mathematical Software 3,
 * 253-256 (1977).  Walker's algorithm required O(N^2) setup time;
 * this code uses the O(N) setup approach devised by James Theiler of
 * LANL, as documented in commentary ini the Gnu Scientific Library.
 * We also use a modification suggested by Donald E. Knuth, The Art of
 * Computer Programming, Volume 2 (Seminumerical algorithms), 3rd
 * edition, Addison-Wesley (1997), p120.
 *
 * The essence of Walker's approach is to observe that each empirical
 * probabilitiy is either above or below the uniform probability by
 * some amount.  Suppose the probability pi of the i-th element is
 * smaller than the uniform probability (1/n).  Then if we choose a
 * uniformly distributed random integer, i will appear too often; to
 * be precise, it will appear 1/n - pi too frequently.  Walker's idea
 * is that there must be some other element, j, that has a probability
 * pj that is above uniform.  So if we "push" the 1/n - pi "extra"
 * probability of element i onto element j, we will decrease the
 * probability of i appearing and increase the probability of j.  We
 * can do this by selecting a "cutoff" value which is to be compared
 * to a random number x on [0,1); if x exceeds the cutoff, we remap to
 * element j.  The cutoff is selected such that this happens exactly
 * (1/n - pi) / (1/n) = 1 - n*pi of the time, since that's the amount
 * of extra probability that needs to be pushed onto j.
 *
 * For example, suppose there are only two probabilities, 0.25 and
 * 0.75.  Element 0 will be selected 0.5 of the time, so we must remap
 * half of those selections to j.  The cutoff is chosen as 1 - 2*0.25
 * = 0.5.  Presto!
 *
 * In general, element j won't need precisely the amount of extra
 * stuff remapped from element i.  If it needs more, that's OK; there
 * will be some other element k that has a probability below uniform,
 * and we can also map its extra onto j.  If j needs *less* extra,
 * then we'll do a remap on it as well, pushing that extra onto yet
 * another element--but only if j was selected directly in the initial
 * uniform distribution.  (All of these adjustments are done by
 * modifying the calculated difference between j's probability and the
 * uniform distribution.)  This produces the rather odd result that j
 * both accepts and donates probability, but it all works out in the
 * end.
 *
 * The trick is then to calculate the cutoff and remap arrays.  The
 * formula for the cutoff values was given above.  At each step,
 * Walker scans the current probability array to find the elements
 * that are most "out of balance" on both the high and low ends; the
 * low one is then remapped to the high.  The loop is repeated until
 * all probabilities differ from uniform by less than predetermined
 * threshold.  This is an O(N^2) algorithm; it can also be troublesome
 * if the threshold is in appropriate for the data at hand.
 *
 * Theiler's improvement involves noting that if a probability is
 * below uniform ("small"), it will never become "large".  That means
 * we can keep two tables, one each of small and large values.  For
 * convenience, the tables are organized as stacks.  At each step, a
 * value is popped from each stack, and the small one is remapped to
 * the large one by calculating a cutoff.  The large value is then
 * placed back on the appropriate stack.  (For efficiency, the
 * implementation doesn't pop from the large stack unless necessary.)
 *
 * Finally, Knuth's improvements: Walker's original paper suggested
 * drawing two uniform random numbers when generating from the
 * empirical distribution: one to select an element, and a second to
 * compare to the cutoff.  Knuth points out that if the random numbers
 * have sufficient entropy (which is certainly true for the Mersenne
 * Twister), we can use the upper bits to choose a slot and the lower
 * ones to compare against the cutoff.  This is done by taking s = n*r
 * (where r is the double-precision random value), and then using
 * int(s) as the slot and frac(s) as the cutoff.  The final
 * improvement is that we can avoid calculating frac(s) if, when
 * setting the cutoff c, we store i + c instead of c, where i is the
 * slot number.
 */
rd_empirical_control* rd_empirical_setup(
    size_t		n_probs,	/* Number of probabilities provide */
    const double*	probs,		/* Probability (weight) table */
    const double*	values)		/* Value for floating distributions */
    {
    rd_empirical_control* control;	/* Control structure we'll build */
    size_t		i;		/* General loop index */
    size_t		j;		/* Element from stack_high */
    size_t		n_high;		/* Current size of stack_high */
    size_t		n_low;		/* Current size of stack_low */
    size_t*		stack_high;	/* Stack of values above uniform */
    size_t*		stack_low;	/* Stack of values below uniform */
    double		prob_total;	/* Total of all weights */

    control = (rd_empirical_control*)malloc(sizeof *control);
    if (control == NULL)
	return NULL;
    control->n = n_probs;
    control->cutoff = (double*)malloc(n_probs * sizeof (double));
    control->remap = (size_t*)malloc(n_probs * sizeof (size_t));
    control->values = (double*)malloc((n_probs + 1) * sizeof (double));
    if (control->cutoff == NULL  ||  control->remap == NULL
      ||  control->values == NULL)
	{
	rd_empirical_free(control);
	return NULL;
	}
    if (values != NULL)
	{
	/*
	 * We could use memcpy here, but doing so is kind of
	 * ugly...and a smart compiler will do it for us.
	 *
	 * Note that we're snagging one extra value, regardless of
	 * whether it'll actually be needed.  This can cause segfaults
	 * if the caller isn't careful.
	 */
	for (i = 0;  i <= n_probs;  i++)
	    control->values[i] = values[i];
	}
    else
	{
	/*
	 * Generate values in the range [0,1).
	 */
	for (i = 0;  i <= n_probs;  i++)
	    control->values[i] = (double)i / n_probs;
	}
    stack_high = (size_t*)malloc(n_probs * sizeof (size_t));
    if (stack_high == NULL)
	{
	rd_empirical_free(control);
	return NULL;
	}
    stack_low = (size_t*)malloc(n_probs * sizeof (size_t));
    if (stack_low == NULL)
	{
	free(stack_high);
	rd_empirical_free(control);
	return NULL;
	}
    n_high = n_low = 0;

    /*
     * We're done with memory allocation, and we've snagged the values
     * array.  Now it's time to generate the probability cutoffs and
     * the remap array, which form the heart of the algorithm.  First,
     * we initialize the cutoffs array to the difference between the
     * desired probability and a uniform distribution.  Elements that
     * are less probable than uniform go on stack_low; the rest go on
     * stack_high.
     */
    for (i = 0, prob_total = 0.0;  i < n_probs;  i++)
	prob_total += probs[i];
    for (i = 0;  i < n_probs;  i++)
	{
	control->remap[i] = i;
	control->cutoff[i] = probs[i] / prob_total - 1.0 / n_probs;
	if (control->cutoff[i] >= 0.0)
	    stack_high[n_high++] = i;
	else
	    stack_low[n_low++] = i;
	}
    /*
     * Now we adjust the cutoffs.  For each item on stack_low,
     * generate a probabilistic remapping from it to the top element
     * on stack_high.  Then adjust the top element of stack_high to
     * reflect that fact, if necessary moving it to stack_low.
     */
    while (n_low > 0)
	{
	i = stack_low[--n_low];		/* i is the guy we'll adjust */
	j = stack_high[n_high - 1];
	/*
	 * The cutoff for i is negative, and represents the difference
	 * between the uniform distribution and how often this element
	 * should occur.  For example, if n_probs is 4, a uniform
	 * distribution would generate each value 1/4 of the time.
	 * Suppose element i instead has a probability of 0.20.  Then
	 * cutoffs[i] is -0.05.  If a random choice picked us, we must
	 * remap to some higher-probability event 0.05/0.25 = 0.05 /
	 * (1/4) = 0.05 * n_probs = 20% of the time.  This is done by
	 * setting the cutoff to 1.0 + (-0.05) * n_probs = 1.0 - 0.20
	 * = 0.8.
	 *
	 * We also use a trick due to Knuth, which involves adding an
	 * extra integer "i" to the cutoff.  This saves us one step in
	 * the random-number generation because we won't have to
	 * separate out the fractional part of the result of
	 * rds_ldrand (see rds_int_empirical).
	 *
	 * Because we are "transferring" part of the probability of i
	 * to the top of stack_high, we must also adjust its
	 * probability cutoff to reflect that fact.  In the example
	 * above, we are transferring 0.05 of the probability of i
	 * onto stack_high, so we must subtract that amount from
	 * stack_high.  Since the cutoff is negative, "subtract" means
	 * "add" here.
	 */
	control->cutoff[j] += control->cutoff[i];
	control->cutoff[i] = i + 1.0 + control->cutoff[i] * n_probs;
	control->remap[i] = j;
	/*
	 * If the stack_high cutoff became negative, move it to stack_low.
	 */
	if (control->cutoff[j] < 0.0)
	    {
	    stack_low[n_low++] = j;
	    --n_high;
	    }
	}
    /*
     * We're done; the cutoffs are all prepared.  Note that there may
     * still be elements on stack_high; that's not a problem because
     * they're all (effectively) zero.  Go through them and set their
     * cutoffs such that they'll never be remapped.
     */
    for (i = 0;  i < n_high;  i++)
	{
	j = stack_high[i];
	control->cutoff[j] = j + 1.0;
	}
    free(stack_high);
    free(stack_low);
    return control;
    }

/*
 * Free an empirical-distribution control structure.
 */
void rd_empirical_free(
    rd_empirical_control* control)	/* Structure to free */
    {
    if (control == NULL)
	return;
    if (control->cutoff != NULL)
	free(control->cutoff);
    if (control->remap != NULL)
	free(control->remap);
    if (control->values != NULL)
	free(control->values);
    free(control);
    }

/*
 * Generate a discrete integer empirical distribution given a set of
 * probability cutoffs.  See rd_empirical_setup for full information.
 */
size_t rd_int_empirical(
    rd_empirical_control* control)	/* Control from rd_empirical_setup */
    {
    return rds_int_empirical(&mt_default_state, control);
    }

/*
 * Generate a discrete floating-point empirical distribution given a
 * set of probability cutoffs.  See rds_double_empirical.
 */
double rd_double_empirical(
    rd_empirical_control* control)	/* Control from rd_empirical_setup */
    {
    return rds_double_empirical(&mt_default_state, control);
    }

/*
 * Generate a continuous floating-point empirical distribution given a
 * set of probability cutoffs.  See rds_continuous_empirical.
 */
double rd_continuous_empirical(
    rd_empirical_control* control)	/* Control from rd_empirical_setup */
    {
    return rds_continuous_empirical(&mt_default_state, control);
    }
