#ifndef RANDISTRS_H
#define RANDISTRS_H

/*
 * $Id: randistrs.h,v 1.8 2013-01-05 01:18:52-08 geoff Exp $
 *
 * Header file for C/C++ use of a generalized package that generates
 * random numbers in various distributions, using the Mersenne-Twist
 * pseudo-RNG.  See mtwist.h and mtwist.c for documentation on the PRNG.
 *
 * Author of this header file: Geoff Kuenning, April 7, 2001.
 *
 * All of the functions provided by this package have three variants.
 * The rd_xxx versions use the default state vector provided by the MT
 * package.  The rds_xxx versions use a state vector provided by the
 * caller.  In general, the rds_xxx versions are preferred for serious
 * applications, since they allow random numbers used for different
 * purposes to be drawn from independent, uncorrelated streams.
 * Finally, the C++ interface provides a class "mt_distribution",
 * derived from mt_prng, with no-prefix ("xxx") versions of each
 * function.
 *
 * The summary below will describe only the rds_xxx functions.  The
 * rd_xxx functions have identical specifications, except that the
 * "state" argument is omitted.  In all cases, the "state" argument
 * has type mt_state, and must have been initialized either by calling
 * one of the Mersenne Twist seeding functions, or by being set to all
 * zeros.
 *
 * The "l" version of each function calls the 64-bit version of the
 * PRNG instead of the 32-bit version.  In general, you shouldn't use
 * those functions unless your application is *very* sensitive to tiny
 * variations in the probability distribution.  This is especially
 * true of the uniform and empirical distributions.
 *
 * Random-distribution functions:
 *
 * rds_iuniform(mt_state* state, long lower, long upper)
 *		(Integer) uniform on the half-open interval [lower, upper).
 * rds_liuniform(mt_state* state, long long lower, long long upper)
 *		(Integer) uniform on the half-open interval [lower, upper).
 *		Don't use unless you need numbers bigger than a long!
 * rds_uniform(mt_state* state, double lower, double upper)
 *		(Floating) uniform on the half-open interval [lower, upper).
 * rds_luniform(mt_state* state, double lower, double upper)
 *		(Floating) uniform on the half-open interval [lower, upper).
 *		Higher precision but slower than rds_uniform.
 * rds_exponential(mt_state* state, double mean)
 *		Exponential with the given mean.
 * rds_lexponential(mt_state* state, double mean)
 *		Exponential with the given mean.
 *		Higher precision but slower than rds_exponential.
 * rds_erlang(mt_state* state, int p, double mean)
 *		p-Erlang with the given mean.
 * rds_lerlang(mt_state* state, int p, double mean)
 *		p-Erlang with the given mean.
 *		Higher precision but slower than rds_erlang.
 * rds_weibull(mt_state* state, double shape, double scale)
 *		Weibull with the given shape and scale parameters.
 * rds_lweibull(mt_state* state, double shape, double scale)
 *		Weibull with the given shape and scale parameters.
 *		Higher precision but slower than rds_weibull.
 * rds_normal(mt_state* state, double mean, double sigma)
 *		Normal with the  given mean and standard deviation.
 * rds_lnormal(mt_state* state, double mean, double sigma)
 *		Normal with the  given mean and standard deviation.
 *		Higher precision but slower than rds_normal.
 * rds_lognormal(mt_state* state, double shape, double scale)
 *		Lognormal with the given shape and scale parameters.
 * rds_llognormal(mt_state* state, double shape, double scale)
 *		Lognormal with the given shape and scale parameters.
 *		Higher precision but slower than rds_lognormal.
 * rds_triangular(mt_state* state, double lower, double upper, double mode)
 *		Triangular on the closed interval (lower, upper) with
 *		the given mode.
 * rds_ltriangular(mt_state* state, double lower, double upper, double mode)
 *		Triangular on the closed interval (lower, upper) with
 *		the given mode.
 *		Higher precision but slower than rds_triangular.
 * rds_int_empirical(mt_state* state, rd_empirical_control* control)
 *		Unsigned integer (actually a size_t) in the range [0, n)
 *		with empirically determined probabilities.  The
 *		"control" argument is the return value from a previous
 *		call to rd_emprical_setup; see documentation on that
 *		function below for more information.
 * rds_double_empirical(mt_state* state, rd_empirical_control* control)
 *		Double empirically selected from a list of values
 *		given to rd_empirical_setup (q.v.).
 * rds_continuous_empirical(mt_state* state, rd_empirical_control* control)
 *		Continuous empirical distribution.  See rd_empirical_setup.
 * rd_iuniform(long lower, long upper)
 * rd_liuniform(long long lower, long long upper)
 *		As above, using the default MT-PRNG.
 * rd_uniform(double lower, double upper)
 * rd_luniform(double lower, double upper)
 *		As above, using the default MT-PRNG.
 * rd_exponential(double mean)
 * rd_lexponential(double mean)
 *		As above, using the default MT-PRNG.
 * rd_erlang(int p, double mean)
 * rd_lerlang(int p, double mean)
 *		As above, using the default MT-PRNG.
 * rd_weibull(double shape, double scale)
 * rd_lweibull(double shape, double scale)
 *		As above, using the default MT-PRNG.
 * rd_normal(double mean, double sigma)
 * rd_lnormal(double mean, double sigma)
 *		As above, using the default MT-PRNG.
 * rd_lognormal(double shape, double scale)
 * rd_llognormal(double shape, double scale)
 *		As above, using the default MT-PRNG.
 * rd_triangular(double lower, double upper, double mode)
 * rd_ltriangular(double lower, double upper, double mode)
 *		As above, using the default MT-PRNG.
 * rd_empirical_setup(int n_probs, double* probs, double* values)
 *		Set up the control table for an empirical
 *		distribution.  Once set up, the returned control table
 *		can be used with multiple independent generators, and
 *		can be used with any of the three empirical
 *		distribution functions; usage can even be intermixed.
 *		In all cases, n_probs is the size of the probs array,
 *		which gives relative weights for different empirically
 *		observed values.  The weights do not need to sum to 1;
 *		if they do not, they will be normalized.  (In the
 *		following descriptions, normalized weights are assumed
 *		for simplicity.)
 *		    For calls to int_empirical, the values array is
 *		ignored.  In this case, the return value is in the
 *		range [0, n), where 0 is returned with probability
 *		probs[0], 1 with probability probs[1], etc.
 *		    For calls to double_empirical, the value
 *		calculated by int_empirical is used as an index into
 *		the values array, so that values[0] is returned with
 *		probability probs[0], values[1] with probability
 *		probs[1], etc.
 *		    For calls to continuous_empirical, the values
 *		array must contain n_probs+1 entries.  It is best for
 *		the values array to be sorted into ascending order;
 *		however, this condition is not enforced.  The return
 *		value is uniformly distributed between values[0] and
 *		values[1] with probability probs[0], between values[1]
 *		and values[2] with probability probs[1], etc.  The
 *		effect will be to generate a piecewise linear
 *		approximation to the empirically observed CDF.
 *		    If "values" is NULL, the setup function will
 *		automatically generate an array of uniformly spaced
 *		values in the range [0.0,1.0].  However, if a values
 *		array is provided, n_probs+1 entries must be supplied
 *		EVEN IF only double_empirical will be called.  This is
 *		because the setup function will be copying n_probs+1
 *		values, and there is a (small) possibility of a
 *		segfault if fewer are provided.
 * rd_empirical_free(rd_empirical_control*  control)
 *		Free a structure allocated by rd_empirical_setup.
 * rd_int_empirical(rd_empirical_control* control)
 * rd_double_empirical(rd_empirical_control* control)
 * rd_continuous_empirical(rd_empirical_control* control)
 *		As above, using the default MT-PRNG.
 *
 * $Log: randistrs.h,v $
 * Revision 1.8  2013-01-05 01:18:52-08  geoff
 * Fix a lot of compiler warnings.  Allow rd_empirical_setup to take
 * const arguments.
 *
 * Revision 1.7  2010-12-10 03:28:19-08  geoff
 * Support the new empirical_distribution interface.
 *
 * Revision 1.6  2010-06-24 20:53:59+12  geoff
 * Switch to using types from stdint.h.
 *
 * Revision 1.5  2008-07-25 16:34:01-07  geoff
 * Fix notation for intervals in commentary.
 *
 * Revision 1.4  2001/06/20 09:07:58  geoff
 * Fix a place where long long wasn't conditionalized.
 *
 * Revision 1.3  2001/06/19 00:41:17  geoff
 * Add the "l" versions of all functions.
 *
 * Revision 1.2  2001/06/18 10:09:24  geoff
 * Add the iuniform functions.  Improve the header comments.  Add a C++
 * interface.  Clean up some stylistic inconsistencies.
 *
 * Revision 1.1  2001/04/09 08:39:54  geoff
 * Initial revision
 *
 */

#include "mtwist.h"
#ifdef __cplusplus
#include <stdexcept>
#include <vector>
#endif

/*
 * Internal structure used to support O(1) generation of empirical
 * distributions.
 */
typedef struct
    {
    size_t		n;		/* Number of probabilities given */
    double*		cutoff;		/* Table of probability cutoffs */
					/* ..this is NOT probabilities; see */
					/* ..comments in the code */
    size_t*		remap;		/* Table of where to remap to */
    double*		values;		/* Float values to return */
    }
			rd_empirical_control;

#ifdef __cplusplus
extern "C"
    {
#endif

/*
 * Functions that use a provided state.
 */
extern int32_t		rds_iuniform(mt_state* state, int32_t lower,
			  int32_t upper);
					/* (Integer) uniform distribution */
#ifdef INT64_MAX
extern int64_t		rds_liuniform(mt_state* state, int64_t lower,
			  int64_t upper);
					/* (Integer) uniform distribution */
#endif /* INT64_MAX */
extern double		rds_uniform(mt_state* state,
			  double lower, double upper);
					/* (Floating) uniform distribution */
extern double		rds_luniform(mt_state* state,
			  double lower, double upper);
					/* (Floating) uniform distribution */
extern double		rds_exponential(mt_state* state, double mean);
					/* Exponential distribution */
extern double		rds_lexponential(mt_state* state, double mean);
					/* Exponential distribution */
extern double		rds_erlang(mt_state* state, int p, double mean);
					/* p-Erlang distribution */
extern double		rds_lerlang(mt_state* state, int p, double mean);
					/* p-Erlang distribution */
extern double		rds_weibull(mt_state* state,
			  double shape, double scale);
					/* Weibull distribution */
extern double		rds_lweibull(mt_state* state,
			  double shape, double scale);
					/* Weibull distribution */
extern double		rds_normal(mt_state* state,
			  double mean, double sigma);
					/* Normal distribution */
extern double		rds_lnormal(mt_state* state,
			  double mean, double sigma);
					/* Normal distribution */
extern double		rds_lognormal(mt_state* state,
			  double shape, double scale);
					/* Lognormal distribution */
extern double		rds_llognormal(mt_state* state,
			  double shape, double scale);
					/* Lognormal distribution */
extern double		rds_triangular(mt_state* state,
			  double lower, double upper, double mode);
					/* Triangular distribution */
extern double		rds_ltriangular(mt_state* state,
			  double lower, double upper, double mode);
					/* Triangular distribution */
extern size_t		rds_int_empirical(mt_state* state,
			  rd_empirical_control* control);
					/* Discrete integer empirical distr. */
extern double		rds_double_empirical(mt_state* state,
			  rd_empirical_control* control);
					/* Discrete float empirical distr. */
extern double		rds_continuous_empirical(mt_state* state,
			  rd_empirical_control* control);
					/* Continuous empirical distribution */

/*
 * Functions that use the default state of the PRNG.
 */
extern int32_t		rd_iuniform(int32_t lower, int32_t upper);
					/* (Integer) uniform distribution */
#ifdef INT64_MAX
extern int64_t		rd_liuniform(int64_t lower, int64_t upper);
					/* (Integer) uniform distribution */
#endif /* INT64_MAX */
extern double		rd_uniform(double lower, double upper);
					/* (Floating) uniform distribution */
extern double		rd_luniform(double lower, double upper);
					/* (Floating) uniform distribution */
extern double		rd_exponential(double mean);
					/* Exponential distribution */
extern double		rd_lexponential(double mean);
					/* Exponential distribution */
extern double		rd_erlang(int p, double mean);
					/* p-Erlang distribution */
extern double		rd_lerlang(int p, double mean);
					/* p-Erlang distribution */
extern double		rd_weibull(double shape, double scale);
					/* Weibull distribution */
extern double		rd_lweibull(double shape, double scale);
					/* Weibull distribution */
extern double		rd_normal(double mean, double sigma);
					/* Normal distribution */
extern double		rd_lnormal(double mean, double sigma);
					/* Normal distribution */
extern double		rd_lognormal(double shape, double scale);
					/* Lognormal distribution */
extern double		rd_llognormal(double shape, double scale);
					/* Lognormal distribution */
extern double		rd_triangular(double lower, double upper, double mode);
					/* Triangular distribution */
extern double		rd_ltriangular(double lower, double upper,
			  double mode);	/* Triangular distribution */
extern rd_empirical_control*
			rd_empirical_setup(size_t n_probs,
			  const double* probs, const double* values);
					/* Set up empirical distribution */
extern void		rd_empirical_free(rd_empirical_control* control);
					/* Free empirical control structure */
extern size_t		rd_int_empirical(rd_empirical_control* control);
					/* Discrete integer empirical distr. */
extern double		rd_double_empirical(rd_empirical_control* control);
					/* Discrete float empirical distr. */
extern double		rd_continuous_empirical(rd_empirical_control* control);
					/* Continuous empirical distribution */

#ifdef __cplusplus
    }
#endif

#ifdef __cplusplus
/*
 * C++ interface to the random-distribution generators.  This class is
 * little more than a wrapper for the C functions, but it fits a bit
 * more nicely with the mt_prng class.
 */
class mt_distribution : public mt_prng
    {
    public:
	/*
	 * Constructors and destructors.  All constructors and
	 * destructors are the same as for mt_prng.
	 */
			mt_distribution(
					// Default constructor
			    bool pickSeed = false)
					// True to get seed from /dev/urandom
					// ..or time
			    : mt_prng(pickSeed)
			    {
			    }
			mt_distribution(uint32_t newseed)
					// Construct with 32-bit seeding
			    : mt_prng(newseed)
			    {
			    }
			mt_distribution(uint32_t seeds[MT_STATE_SIZE])
					// Construct with full seeding
			    : mt_prng(seeds)
			    {
			    }
			~mt_distribution() { }

	/*
	 * Functions for generating distributions.  These simply
	 * invoke the C functions above.
	 */
	int32_t		iuniform(int32_t lower, int32_t upper)
					/* Uniform distribution */
			    {
			    return rds_iuniform(&state, lower, upper);
			    }
#ifdef INT64_MAX
	int64_t	liuniform(int64_t lower, int64_t upper)
					/* Uniform distribution */
			    {
			    return rds_liuniform(&state, lower, upper);
			    }
#endif /* INT64_MAX */
	double		uniform(double lower, double upper)
					/* Uniform distribution */
			    {
			    return rds_uniform(&state, lower, upper);
			    }
	double		luniform(double lower, double upper)
					/* Uniform distribution */
			    {
			    return rds_luniform(&state, lower, upper);
			    }
	double		exponential(double mean)
					/* Exponential distribution */
			    {
			    return rds_exponential(&state, mean);
			    }
	double		lexponential(double mean)
					/* Exponential distribution */
			    {
			    return rds_lexponential(&state, mean);
			    }
	double		erlang(int p, double mean)
					/* p-Erlang distribution */
			    {
			    return rds_erlang(&state, p, mean);
			    }
	double		lerlang(int p, double mean)
					/* p-Erlang distribution */
			    {
			    return rds_lerlang(&state, p, mean);
			    }
	double		weibull(double shape, double scale)
					/* Weibull distribution */
			    {
			    return rds_weibull(&state, shape, scale);
			    }
	double		lweibull(double shape, double scale)
					/* Weibull distribution */
			    {
			    return rds_lweibull(&state, shape, scale);
			    }
	double		normal(double mean, double sigma)
					/* Normal distribution */
			    {
			    return rds_normal(&state, mean, sigma);
			    }
	double		lnormal(double mean, double sigma)
					/* Normal distribution */
			    {
			    return rds_lnormal(&state, mean, sigma);
			    }
	double		lognormal(double shape, double scale)
					/* Lognormal distribution */
			    {
			    return rds_lognormal(&state, shape, scale);
			    }
	double		llognormal(double shape, double scale)
					/* Lognormal distribution */
			    {
			    return rds_llognormal(&state, shape, scale);
			    }
	double		triangular(double lower, double upper, double mode)
					/* Triangular distribution */
			    {
			    return rds_triangular(&state, lower, upper, mode);
			    }
	double		ltriangular(double lower, double upper, double mode)
					/* Triangular distribution */
			    {
			    return rds_ltriangular(&state, lower, upper, mode);
			    }
    };

/*
 * Class for handing empirical distributions.  This is necessary
 * because of the need to allocate and initialize extra parameters.
 *
 * BUG/WARNING: this code will only work on compilers where C
 * malloc/free can be freely intermixed with C++ new/delete.
 */
class mt_empirical_distribution
    {
    public:
		mt_empirical_distribution(const std::vector<double>& probs,
		  const std::vector<double>& values)
		    : c(NULL)
		    {
		    if (values.size() != probs.size() + 1)
			throw std::invalid_argument(
			  "values must be one longer than probs");
		    c = rd_empirical_setup(probs.size(),
			  &probs.front(), &values.front());
		    }
		mt_empirical_distribution(const std::vector<double>& probs)
		    : c(rd_empirical_setup(probs.size(), &probs.front(), NULL))
		    {
		    }
		~mt_empirical_distribution()
		    {
		    rd_empirical_free(c);
		    }

	size_t	int_empirical(mt_prng& rng)
				/* Discrete integer empirical distr. */
		    {
		    return rds_int_empirical(&rng.state, c);
		    }
	double	double_empirical(mt_prng& rng)
				/* Discrete double empirical distr. */
		    {
		    return rds_double_empirical(&rng.state, c);
		    }
	double	continuous_empirical(mt_prng& rng)
				/* Continuous empirical distribution */
		    {
		    return rds_continuous_empirical(&rng.state, c);
		    }
    private:
	/*
	 * Copying and assignment are not supported.  (Implementing
	 * them would either require reconstructing the
	 * original weights, which is ugly, or doing C-style
	 * allocation, which is equally ugly.)
	 */
		mt_empirical_distribution(
		  const mt_empirical_distribution& source);
		mt_empirical_distribution& operator=(
		  const mt_empirical_distribution& rhs);

	/*
	 * Private Data.
	 */
	rd_empirical_control*
		c;		/* C-style control structure */
    };

#endif /* __cplusplus */

#endif /* RANDISTRS_H */
