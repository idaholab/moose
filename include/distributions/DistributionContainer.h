/*
 * DistributionContainer.h
 *
 *  Created on: Jul 6, 2012
 *      Author: alfoa
 */

#ifndef DISTRIBUTIONCONTAINER_H_
#define DISTRIBUTIONCONTAINER_H_

//#include "distribution_1D.h"
#include "distribution_type.h"
#include <iostream>
#include <vector>
#include <map>
#include "distribution.h"
//#include "distribution_base_ND.h"
//#include "Interpolation_Functions.h"
//#include <MooseRandom.h>
//#include <boost/random/mersenne_twister.hpp>


using namespace std;

class RandomClass;

class DistributionContainer{
     public:
     static DistributionContainer & Instance();
     /*
      * Function to construct on the fly this class through the action system
      * @
      * @
      */
     void addDistributionInContainer(const std::string & type, const std::string & name, BasicDistribution * dist);
     void addDistributionInContainerND(const std::string & type, const std::string & name, BasicDistributionND * dist);

     void seedRandom(unsigned int seed);

     bool isEmpty(){return _dist_by_name.empty() and _dist_nd_by_name.empty();};
     /*
      * Function to get the enum of the distribution called DistAlias
      * @ DistAlias, alias of the distribution from which retrieving the parameter
      */
     std::string getType (const char * DistAlias);
     std::string  getType (const std::string DistAlias);

     double getVariable(const char * DistAlias,const char * paramName);
     double getVariable(const std::string DistAlias,const std::string paramName);

     void updateVariable(const char * DistAlias,const char * paramName,double newValue);
     void updateVariable(const std::string DistAlias,const std::string paramName,double newValue);

     std::vector<std::string> getRavenDistributionVariableNames(const std::string DistAlias);
     std::vector<std::string> getDistributionNames();

     double Pdf(const char * DistAlias, double x);
     double Pdf(const std::string DistAlias, double x);     // return pdf value of the distribution _type as function of the position x within [_xMin , xMax]
     double Pdf(const char * DistAlias, std::vector<double> x);
     double Pdf(const std::string DistAlias, std::vector<double> x);     // return pdf value of the distribution _type as function of the position x within [_xMin , xMax]

     /*
      * Function to get the cdf value of the distribution called "DistAlias"
      * as function of the position x within [_xMin , xMax]
      * @ DistAlias, alias of the distribution from which retrieving the parameter
      * @ x, position
      */
     double Cdf(const char * DistAlias, double x);
     double Cdf(const std::string DistAlias, double x);     // return cdf value of the distribution _type as function of the position x within [_xMin , xMax]
     double Cdf(const char * DistAlias, std::vector<double> x);
     double Cdf(const std::string DistAlias, std::vector<double> x);     // return cdf value of the distribution _type as function of the position x within [_xMin , xMax]


     /*
      * Function to get a random number distributed according to the distribution with a random number calculated.
      * @DistAlias alias of the distribution to use.
      */
     double getDistributionRandom(const char * DistAlias);

     /*
      * Function to get a random number distributed according to the distribution with a random number calculated.
      * @DistAlias alias of the distribution to use.
      */
     double getDistributionRandom(const std::string DistAlias);

     /*
      * Function to get a random number distributed accordingly to the distribution
      * given a random number [0,1]
      * @ DistAlias, alias of the distribution from which retrieving the parameter
      */
     double inverseCdf(const std::string DistAlias, double RNG);

     /*
      * Function to get a random number distributed accordingly to the distribution
      * given a random number [0,1]
      * @ DistAlias, alias of the distribution from which retrieving the parameter
      */
     double inverseCdf(const char * DistAlias, double RNG);

     std::vector<double> inverseCdf(const char * DistAlias, double min, double max);
     std::vector<double> inverseCdf(const std::string DistAlias, double min, double max);

     double random(); // return a random number

     bool checkCdf(const std::string DistAlias, double value);
     bool checkCdf(const char * DistAlias, double value);
     bool checkCdf(const std::string DistAlias, std::vector<double> value);
     bool checkCdf(const char * DistAlias, std::vector<double> value);

     bool getTriggerStatus(const std::string DistAlias);

     bool getTriggerStatus(const char * DistAlias);

     // unfortunately there is no way (right now) to link a triggered distribution
     // to the variables that have been changed in consequence of the trigger
     // for now we assume to get the last one.
     std::string lastDistributionTriggered();
     bool atLeastADistTriggered();

     protected:
     std::map < std::string, int > _vector_pos_map;
     //std::vector < BasicDistribution * > _distribution_cont;
     /// mapping from distribution name and distribution
     std::map<std::string, BasicDistribution *> _dist_by_name;
     /// mapping from distributionND name and distribution
     std::map<std::string, BasicDistributionND *> _dist_nd_by_name;
     /// "Buckets" of distribution based on their types
     //  std::map<std::string, std::vector<BasicDistribution *> > _dist_by_type;
     std::map<std::string, bool> _dist_by_trigger_status;
     std::string _last_dist_triggered;
     bool _at_least_a_dist_triggered;

     RandomClass * _random; 
     //MooseRandom _random;

//     /*
//      * Function to get the position in the internal mapping
//      * @ DistAlias, alias of the distribution from which retrieving the parameter
//      */
//     int getPosition(std::string DistAlias);

     /*
      * Constructor(empty)
      */
     DistributionContainer();
     /*
      * Destructor
      */
     virtual ~DistributionContainer();
     static DistributionContainer * _instance; // = NULL 
};

std::string * str_to_string_p(char *s);
const char * string_p_to_str(const std::string * s);
void free_string_p(std::string * s);

#endif /* DISTRIBUTIONCONTAINER_H_ */
