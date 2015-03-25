#ifndef DISTRIBUTION_ND_H
#define DISTRIBUTION_ND_H

#include <string>
#include <vector>
#include "MooseObject.h"
#include "distribution_base_ND.h"
#include "ND_Interpolation_Functions.h"

class distributionND;

template<>
InputParameters validParams<distributionND>();

class distributionND : public MooseObject , public virtual BasicDistributionND
{
public:
  //> constructor for built-in distributions
  distributionND(const std::string & name, InputParameters parameters);
  virtual ~distributionND();

};

/*
 * CLASS MultiDimensionalInverseWeight DISTRIBUTION
 */
class MultiDimensionalInverseWeight;

template<>
InputParameters validParams<MultiDimensionalInverseWeight>();

class MultiDimensionalInverseWeight : public distributionND, public BasicMultiDimensionalInverseWeight
{
public:
  MultiDimensionalInverseWeight(const std::string & name, InputParameters parameters);
  virtual ~MultiDimensionalInverseWeight();
};

/*
 * CLASS MultivariateNormal DISTRIBUTION
 */
class MultivariateNormal;

template<>
InputParameters validParams<MultivariateNormal>();

class MultivariateNormal : public distributionND, public BasicMultivariateNormal
{
public:
  MultivariateNormal(const std::string & name, InputParameters parameters);
  virtual ~MultivariateNormal();
};

/*
 * CLASS MultiDimensionalScatteredMS DISTRIBUTION
 */

class MultiDimensionalScatteredMS;

template<>
InputParameters validParams<MultiDimensionalScatteredMS>();

class MultiDimensionalScatteredMS : public distributionND, public BasicMultiDimensionalScatteredMS
{
public:
  MultiDimensionalScatteredMS(const std::string & name, InputParameters parameters);
  virtual ~MultiDimensionalScatteredMS();
};

/*
 * CLASS MultiDimensionalCartesianSpline DISTRIBUTION
 */
class MultiDimensionalCartesianSpline;

template<>
InputParameters validParams<MultiDimensionalCartesianSpline>();


class MultiDimensionalCartesianSpline : public distributionND, public BasicMultiDimensionalCartesianSpline
{
public:
  MultiDimensionalCartesianSpline(const std::string & name, InputParameters parameters);
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
//class MultiDimensionalLinear : public distributionND, public BasicMultiDimensionalLinear
//{
//public:
//  MultiDimensionalLinear(const std::string & name, InputParameters parameters);
//  virtual ~MultiDimensionalLinear();
//};



#endif
