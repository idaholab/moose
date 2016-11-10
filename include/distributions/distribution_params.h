#ifndef DISTRIBUTION_PARAMS_H
#define DISTRIBUTION_PARAMS_H


#include "MooseObject.h"
#include "distribution.h"
#include "distribution_1D.h"

class Distribution;

template<>
InputParameters validParams<Distribution>();

class Distribution : public MooseObject, public virtual BasicDistribution
{
public:
  //> constructor for built-in distributions
  Distribution(const InputParameters & parameters);

  virtual ~Distribution();

};

template<>
InputParameters validParams<UniformDistribution>();

class UniformDistribution : public Distribution, public BasicUniformDistribution
{
public:
  UniformDistribution(const InputParameters & parameters);
  virtual ~UniformDistribution();
};

template<>
InputParameters validParams<NormalDistribution>();

class NormalDistribution : public Distribution, public BasicNormalDistribution
{
public:
  NormalDistribution(const InputParameters & parameters);
  virtual ~NormalDistribution();
};

template<>
InputParameters validParams<LogNormalDistribution>();

class LogNormalDistribution : public Distribution, public BasicLogNormalDistribution
{
public:
  LogNormalDistribution(const InputParameters & parameters);
  virtual ~LogNormalDistribution();
};

template<>
InputParameters validParams<LogisticDistribution>();

class LogisticDistribution : public Distribution, public BasicLogisticDistribution
{
public:
  LogisticDistribution(const InputParameters & parameters);
  virtual ~LogisticDistribution();
};

template<>
InputParameters validParams<TriangularDistribution>();

class TriangularDistribution : public Distribution, public BasicTriangularDistribution
{
public:
  TriangularDistribution(const InputParameters & parameters);
  virtual ~TriangularDistribution();
};

template<>
InputParameters validParams<ExponentialDistribution>();

class ExponentialDistribution : public Distribution, public BasicExponentialDistribution
{
public:
  ExponentialDistribution(const InputParameters & parameters);
  virtual ~ExponentialDistribution();
};

template<>
InputParameters validParams<WeibullDistribution>();

class WeibullDistribution : public Distribution, public BasicWeibullDistribution
{
public:
  WeibullDistribution(const InputParameters & parameters);
  virtual ~WeibullDistribution();
};

class GammaDistribution;

template<>
InputParameters validParams<GammaDistribution>();

class GammaDistribution : public Distribution, public BasicGammaDistribution
{
public:
  GammaDistribution(const InputParameters & parameters);
  virtual ~GammaDistribution();
};

class BetaDistribution;

template<>
InputParameters validParams<BetaDistribution>();

class BetaDistribution : public Distribution, public BasicBetaDistribution
{
public:
  BetaDistribution(const InputParameters & parameters);
  virtual ~BetaDistribution();
};

class PoissonDistribution;

template<>
InputParameters validParams<PoissonDistribution>();

class PoissonDistribution : public Distribution, public BasicPoissonDistribution
{
public:
  PoissonDistribution(const InputParameters & parameters);
  virtual ~PoissonDistribution();
};

class BinomialDistribution;

template<>
InputParameters validParams<BinomialDistribution>();

class BinomialDistribution : public Distribution, public BasicBinomialDistribution
{
public:
  BinomialDistribution(const InputParameters & parameters);
  virtual ~BinomialDistribution();
};

class BernoulliDistribution;

template<>
InputParameters validParams<BernoulliDistribution>();

class BernoulliDistribution : public Distribution, public BasicBernoulliDistribution
{
public:
  BernoulliDistribution(const InputParameters & parameters);
  virtual ~BernoulliDistribution();
};

class ConstantDistribution;

template<>
InputParameters validParams<ConstantDistribution>();

class ConstantDistribution : public Distribution, public BasicConstantDistribution
{
public:
  ConstantDistribution(const InputParameters & parameters);
  virtual ~ConstantDistribution();
};


#endif /* DISTRIBUTION_PARAMS_H */
