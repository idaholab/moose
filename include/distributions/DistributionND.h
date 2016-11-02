#ifndef DISTRIBUTION_ND_H
#define DISTRIBUTION_ND_H

#include <string>
#include <vector>
#include "MooseObject.h"
#include "distributionNDBase.h"
#include "distributionNDInverseWeight.h"
#include "distributionNDNormal.h"
#include "distributionNDCartesianSpline.h"
#include "distributionNDScatteredMS.h"
#include "ND_Interpolation_Functions.h"

class DistributionND;

template<>
InputParameters validParams<DistributionND>();

class DistributionND : public MooseObject , public virtual BasicDistributionND
{
public:
  //> constructor for built-in distributions
  DistributionND(const InputParameters & parameters);
  virtual ~DistributionND();

};

/*
 * CLASS MultiDimensionalInverseWeight DISTRIBUTION
 */
class MultiDimensionalInverseWeight;

template<>
InputParameters validParams<MultiDimensionalInverseWeight>();

class MultiDimensionalInverseWeight : public DistributionND, public BasicMultiDimensionalInverseWeight
{
public:
  MultiDimensionalInverseWeight(const InputParameters & parameters);
  virtual ~MultiDimensionalInverseWeight();
};

/*
 * CLASS MultivariateNormal DISTRIBUTION
 */
class MultivariateNormal;

template<>
InputParameters validParams<MultivariateNormal>();

class MultivariateNormal : public DistributionND, public BasicMultivariateNormal
{
public:
  MultivariateNormal(const InputParameters & parameters);
  virtual ~MultivariateNormal();
};

/*
 * CLASS MultiDimensionalScatteredMS DISTRIBUTION
 */

class MultiDimensionalScatteredMS;

template<>
InputParameters validParams<MultiDimensionalScatteredMS>();

class MultiDimensionalScatteredMS : public DistributionND, public BasicMultiDimensionalScatteredMS
{
public:
  MultiDimensionalScatteredMS(const InputParameters & parameters);
  virtual ~MultiDimensionalScatteredMS();
};

/*
 * CLASS MultiDimensionalCartesianSpline DISTRIBUTION
 */
class MultiDimensionalCartesianSpline;

template<>
InputParameters validParams<MultiDimensionalCartesianSpline>();


class MultiDimensionalCartesianSpline : public DistributionND, public BasicMultiDimensionalCartesianSpline
{
public:
  MultiDimensionalCartesianSpline(const InputParameters & parameters);
  virtual ~MultiDimensionalCartesianSpline();
};

///*
// * CLASS MultiDimensionalLinear DISTRIBUTION
// */
//class MultiDimensionalLinear;
//
//template<>
//InputParameters validParams<MultiDimensionalLinear>();
//
//
//class MultiDimensionalLinear : public DistributionND, public BasicMultiDimensionalLinear
//{
//public:
//  MultiDimensionalLinear(const InputParameters & parameters);
//  virtual ~MultiDimensionalLinear();
//};



#endif
