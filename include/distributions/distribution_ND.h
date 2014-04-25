/*
 * distribution_ND.h
 *
 *  Created on: Feb 6, 2013
 *      Author: alfoa
 *
 */

#ifndef DISTRIBUTION_ND_H_
#define DISTRIBUTION_ND_H_

#include <string>
#include <vector>
#include "RavenObject.h"
#include "distribution_base_ND.h"
#include "ND_Interpolation_Functions.h"

class distributionND;

template<>
InputParameters validParams<distributionND>();

class distributionND : public RavenObject , public virtual BasicDistributionND
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

class MultiDimensionalInverseWeight : public distributionND, public BasicMultiDimensionalInverseWeight {
public:
  MultiDimensionalInverseWeight(const std::string & name, InputParameters parameters);
  virtual ~MultiDimensionalInverseWeight();
protected:
};

/*
 * CLASS MultivariateNormal DISTRIBUTION
 */
class MultivariateNormal;

template<>
InputParameters validParams<MultivariateNormal>();

class MultivariateNormal : public distributionND, public BasicMultivariateNormal {
public:
  MultivariateNormal(const std::string & name, InputParameters parameters);
  virtual ~MultivariateNormal();
protected:
};

/*
 * CLASS MultiDimensionalScatteredMS DISTRIBUTION
 */

class MultiDimensionalScatteredMS;

template<>
InputParameters validParams<MultiDimensionalScatteredMS>();

class MultiDimensionalScatteredMS : public distributionND, public BasicMultiDimensionalScatteredMS {
public:
  MultiDimensionalScatteredMS(const std::string & name, InputParameters parameters);
  virtual ~MultiDimensionalScatteredMS();
protected:
};

/*
 * CLASS MultiDimensionalCartesianSpline DISTRIBUTION
 */
class MultiDimensionalCartesianSpline;

template<>
InputParameters validParams<MultiDimensionalCartesianSpline>();


class MultiDimensionalCartesianSpline : public distributionND, public BasicMultiDimensionalCartesianSpline {
public:
  MultiDimensionalCartesianSpline(const std::string & name, InputParameters parameters);
  virtual ~MultiDimensionalCartesianSpline();
protected:
};




#endif
