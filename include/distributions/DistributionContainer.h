/* Copyright 2017 Battelle Energy Alliance, LLC

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#ifndef DISTRIBUTIONCONTAINER_H
#define DISTRIBUTIONCONTAINER_H

#include <iostream>
#include <vector>
#include <map>
#include "distribution.h"
#include "randomClass.h"

class BasicDistribution;
class BasicDistributionND;


#include <memory>
#define MooseSharedPointer std::shared_ptr
#define MooseSharedNamespace std

class DistributionContainer
{
public:
  static DistributionContainer & instance();
  /**
   * Function to construct on the fly this class through the action system
   */
  void addDistributionInContainer(const std::string & type, const std::string & name, MooseSharedPointer<BasicDistribution> dist);
  void addDistributionInContainerND(const std::string & type, const std::string & name, MooseSharedPointer<BasicDistributionND> dist);

  void seedRandom(unsigned int seed);

  bool isEmpty()
  {
    return _dist_by_name.empty() && _dist_nd_by_name.empty();
  };

  /**
   * Function to get the enum of the distribution called dist_alias
   * @param dist_alias, alias of the distribution from which retrieving the parameter
   */
  std::string getType (const char * dist_alias);
  std::string  getType (const std::string dist_alias);

  double getVariable(const char * dist_alias,const char * param_name);
  double getVariable(const std::string dist_alias,const std::string param_name);

  void updateVariable(const char * dist_alias,const char * param_name,double new_value);
  void updateVariable(const std::string dist_alias,const std::string param_name,double new_value);

  std::vector<std::string> getDistributionVariableNames(const std::string dist_alias);
  std::vector<std::string> getDistributionNames();

  double pdf(const char * dist_alias, double x);
  double pdf(const std::string dist_alias, double x);     // return pdf value of the distribution _type as function of the position x within [_xMin , xMax]
  double pdf(const char * dist_alias, std::vector<double> x);
  double pdf(const std::string dist_alias, std::vector<double> x);     // return pdf value of the distribution _type as function of the position x within [_xMin , xMax]

  /*
   * Function to get the cdf value of the distribution called "dist_alias"
   * as function of the position x within [_xMin , xMax]
   * @param dist_alias, alias of the distribution from which retrieving the parameter
   * @param x, position
   */
  double cdf(const char * dist_alias, double x);
  double cdf(const std::string dist_alias, double x);     // return cdf value of the distribution _type as function of the position x within [_xMin , xMax]
  double cdf(const char * dist_alias, std::vector<double> x);
  double cdf(const std::string dist_alias, std::vector<double> x);     // return cdf value of the distribution _type as function of the position x within [_xMin , xMax]


  /**
   * Function to get a random number distributed according to the distribution with a random number calculated.
   * @param dist_alias alias of the distribution to use.
   */
  double getDistributionRandom(const char * dist_alias);

  /**
   * Function to get a random number distributed according to the distribution with a random number calculated.
   * @param dist_alias alias of the distribution to use.
   */
  double getDistributionRandom(const std::string dist_alias);

  /**
   * Function to get a random number distributed accordingly to the distribution
   * given a random number [0,1]
   * @param dist_alias, alias of the distribution from which retrieving the parameter
   */
  double inverseCdf(const std::string dist_alias, double rng);

  /*
   * Function to get a random number distributed accordingly to the distribution
   * given a random number [0,1]
   * @ dist_alias, alias of the distribution from which retrieving the parameter
   */
  double inverseCdf(const char * dist_alias, double rng);

  std::vector<double> inverseCdf(const char * dist_alias, double f, double g);
  std::vector<double> inverseCdf(const std::string dist_alias, double f, double g);

  double random(); // return a random number

  bool checkCdf(const std::string dist_alias, double value);
  bool checkCdf(const char * dist_alias, double value);
  bool checkCdf(const std::string dist_alias, std::vector<double> value);
  bool checkCdf(const char * dist_alias, std::vector<double> value);

  bool getTriggerStatus(const std::string dist_alias);

  bool getTriggerStatus(const char * dist_alias);

  // unfortunately there is no way (right now) to link a triggered distribution
  // to the variables that have been changed in consequence of the trigger
  // for now we assume to get the last one.
  std::string lastDistributionTriggered();
  bool atLeastADistTriggered();

  int returnDimensionality(const std::string dist_alias);
  int returnDimensionality(const char * dist_alias);

protected:
  std::map < std::string, int > _vector_pos_map;

  /// mapping from distribution name and distribution
  std::map<std::string, MooseSharedPointer<BasicDistribution> > _dist_by_name;
  /// mapping from distributionND name and distribution
  std::map<std::string, MooseSharedPointer<BasicDistributionND> > _dist_nd_by_name;

  std::map<std::string, bool> _dist_by_trigger_status;
  std::string _last_dist_triggered;
  bool _at_least_a_dist_triggered;

  RandomClass * _random;

  /**
   * Constructor(empty)
   */
  DistributionContainer();
  /**
   * Destructor
   */
  virtual ~DistributionContainer();
  static DistributionContainer * _instance; // = NULL
};

std::string * strToStringP(char *s);
const char * stringPToSt(const std::string * s);
void freeStringP(std::string * s);

#endif /* DISTRIBUTIONCONTAINER_H */
