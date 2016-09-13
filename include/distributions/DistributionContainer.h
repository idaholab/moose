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
  static DistributionContainer & Instance();
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

  double Pdf(const char * dist_alias, double x);
  double Pdf(const std::string dist_alias, double x);     // return pdf value of the distribution _type as function of the position x within [_xMin , xMax]
  double Pdf(const char * dist_alias, std::vector<double> x);
  double Pdf(const std::string dist_alias, std::vector<double> x);     // return pdf value of the distribution _type as function of the position x within [_xMin , xMax]

  /*
   * Function to get the cdf value of the distribution called "dist_alias"
   * as function of the position x within [_xMin , xMax]
   * @param dist_alias, alias of the distribution from which retrieving the parameter
   * @param x, position
   */
  double Cdf(const char * dist_alias, double x);
  double Cdf(const std::string dist_alias, double x);     // return cdf value of the distribution _type as function of the position x within [_xMin , xMax]
  double Cdf(const char * dist_alias, std::vector<double> x);
  double Cdf(const std::string dist_alias, std::vector<double> x);     // return cdf value of the distribution _type as function of the position x within [_xMin , xMax]


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

  std::vector<double> inverseCdf(const char * dist_alias, double F, double g);
  std::vector<double> inverseCdf(const std::string dist_alias, double F, double g);

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

std::string * str_to_string_p(char *s);
const char * string_p_to_str(const std::string * s);
void free_string_p(std::string * s);

#endif /* DISTRIBUTIONCONTAINER_H */
