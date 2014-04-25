/*
 * DistributionContainer.C
 *
 *  Created on: Jul 6, 2012
 *      Author: alfoa
 */
#include "DistributionContainer.h"
//#include "Factory.h"
//#include "distribution_1D.h"
#include "distribution_min.h"
#include "distribution_base_ND.h"
#include <iostream>
#include <math.h>
#include <cmath>
#include <cstdlib>
#include <stdlib.h>
#include <vector>
#include <map>
//#include <MooseRandom.h>
#include <boost/random/mersenne_twister.hpp>

using namespace std;

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

class DistributionContainer;

class RandomClass {
  boost::random::mt19937 rng;
  const double range;
public:
  RandomClass() : range(rng.max() - rng.min()) {};
  void seed(unsigned int seed) {
    rng.seed(seed);
  }
  double random() {
    return (rng()-rng.min())/range;
  }
};

DistributionContainer::DistributionContainer()
{
  _random = new RandomClass();
  _at_least_a_dist_triggered = false;
  _last_dist_triggered = "";
}
DistributionContainer::~DistributionContainer()
{
  delete _random;
}


void
DistributionContainer::addDistributionInContainer(const std::string & /*type*/, const std::string & name, BasicDistribution * dist){
   // create the distribution type
  //distribution * dist = dynamic_cast<distribution *>(_factory.create(type, name, params));
   if (_dist_by_name.find(name) == _dist_by_name.end())
    _dist_by_name[name] = dist;
   else
     throwError("Distribution with name " << name << " already exists");

   _dist_by_trigger_status[name] = false;
   _at_least_a_dist_triggered = false;

   //_dist_by_type[type].push_back(dist);

}

void
DistributionContainer::addDistributionInContainerND(const std::string & /*type*/, const std::string & name, BasicDistributionND * dist){
   // create the distribution type
  //distribution * dist = dynamic_cast<distribution *>(_factory.create(type, name, params));
   if (_dist_by_name.find(name) == _dist_by_name.end())
     _dist_nd_by_name[name] = dist;
   else
     throwError("Distribution with name " << name << " already exists");

   _dist_by_trigger_status[name] = false;
   _at_least_a_dist_triggered = false;

   //_dist_by_type[type].push_back(dist);

}

std::string
DistributionContainer::getType(const char *  DistAlias){
  return getType(std::string(DistAlias));
}

std::string
DistributionContainer::getType(const std::string DistAlias){

    if(_dist_by_name.find(DistAlias) != _dist_by_name.end())
    {
       BasicDistribution * dist = _dist_by_name.find(DistAlias)->second;
       std::string type = getDistributionType(*dist);
       if(type == "DistributionError")
       {
         throwError("getType: Type for distribution " << DistAlias << " not found");
       }
       return type;
    }
    else if (_dist_nd_by_name.find(DistAlias) != _dist_nd_by_name.end())
    {
      BasicDistributionND * dist = _dist_nd_by_name.find(DistAlias)->second;
      std::string type = getDistributionType(*dist);
      if(type == "DistributionError")
      {
        throwError("getType :Type for distribution " << DistAlias << " not found");
      }
      return type;
    }
    else{
       throwError("getType: Distribution " << DistAlias << " not found in distribution container");
       return "DistributionError";
    }
}

void
DistributionContainer::seedRandom(unsigned int seed){
  //std::cout << "seedRandom " << seed << std::endl;
  //srand( seed );
  //_random.seed(seed);
  //MooseRandom::seed(seed);
  _random->seed(seed);

}
double
DistributionContainer::random(){
  //return (static_cast<double>(rand())/static_cast<double>(RAND_MAX));
  //return _random.rand();
  //return MooseRandom::rand();
  return _random->random();
}

bool
DistributionContainer::checkCdf(const std::string DistAlias, double value){
  bool result;
  if (Cdf(std::string(DistAlias),value) >= getVariable(DistAlias,"ProbabilityThreshold")){
    result=true;
    _dist_by_trigger_status[DistAlias] = true;
    _last_dist_triggered = DistAlias;
    _at_least_a_dist_triggered = true;
  }
  else{
    result=false;
    _dist_by_trigger_status[DistAlias] = false;
  }
  return result;
}

bool DistributionContainer::checkCdf(const std::string DistAlias, std::vector<double> value){
  bool result;
  if (Cdf(std::string(DistAlias),value) >= getVariable(DistAlias,"ProbabilityThreshold")){
    result=true;
    _dist_by_trigger_status[DistAlias] = true;
    _last_dist_triggered = DistAlias;
    _at_least_a_dist_triggered = true;
  }
  else{
    result=false;
    _dist_by_trigger_status[DistAlias] = false;
  }
  return result;
}

bool
DistributionContainer::getTriggerStatus(const std::string DistAlias){
  bool st;
  if(_dist_by_trigger_status.find(DistAlias) != _dist_by_trigger_status.end()){
    st = _dist_by_trigger_status.find(DistAlias) -> second;
  }
  else{
    throwError("getTriggerStatus:Distribution " + DistAlias + " not found in Triggering event.");
  }
  return st;
}
bool
DistributionContainer::getTriggerStatus(const char * DistAlias){
  return getTriggerStatus(std::string(DistAlias));
}
// to be implemented
bool DistributionContainer::checkCdf(const char * DistAlias, double value){
  return checkCdf(std::string(DistAlias),value);
}
bool DistributionContainer::checkCdf(const char * DistAlias, std::vector<double> value){
  return checkCdf(std::string(DistAlias),value);
}
// end to be implemented

double
DistributionContainer::getVariable(const char *DistAlias, const char * paramName){
  return getVariable(std::string(DistAlias),std::string(paramName));
}

double
DistributionContainer::getVariable(const std::string DistAlias, const std::string paramName){
    if(_dist_by_name.find(DistAlias) != _dist_by_name.end())
    {
       BasicDistribution * dist = _dist_by_name.find(DistAlias)->second;
       return getDistributionVariable(*dist,paramName);
    }
    else if (_dist_nd_by_name.find(DistAlias) != _dist_nd_by_name.end())
    {
      BasicDistributionND * dist = _dist_nd_by_name.find(DistAlias)->second;
      return getDistributionVariable(*dist,paramName);
    }
    throwError("getVariable: Distribution " << DistAlias << " not found in distribution container");
    return -1;
}

void
DistributionContainer::updateVariable(const char *DistAlias,const char * paramName,double newValue){
  updateVariable(std::string(DistAlias),std::string(paramName),newValue);
}

void
DistributionContainer::updateVariable(const std::string DistAlias,const std::string paramName,double newValue){
    if(_dist_by_name.find(DistAlias) != _dist_by_name.end())
    {
       BasicDistribution * dist = _dist_by_name.find(DistAlias)->second;
       DistributionUpdateVariable(*dist,paramName,newValue);
    }
    else if (_dist_nd_by_name.find(DistAlias) != _dist_nd_by_name.end())
    {
       BasicDistributionND * dist = _dist_nd_by_name.find(DistAlias)->second;
       DistributionUpdateVariable(*dist,paramName,newValue);
    }
    else
    {
       throwError("updateVariable: Distribution " + DistAlias + " was not found in distribution container.");

    }
}

std::vector<std::string>
DistributionContainer::getDistributionNames(){
  std::vector<std::string> distsNames;
  for(std::map<std::string, BasicDistribution *>::iterator it = _dist_by_name.begin(); it!= _dist_by_name.end();it++)
  {
    distsNames.push_back(it->first);
  }
  for(std::map<std::string, BasicDistributionND *>::iterator it = _dist_nd_by_name.begin(); it!= _dist_nd_by_name.end();it++)
  {
    distsNames.push_back(it->first);
  }
  return distsNames;
}

std::vector<std::string>
DistributionContainer::getRavenDistributionVariableNames(const std::string DistAlias){
  if(_dist_by_name.find(DistAlias) != _dist_by_name.end())
  {
     BasicDistribution * dist = _dist_by_name.find(DistAlias)->second;
     return getDistributionVariableNames(*dist);
  }
  else if(_dist_nd_by_name.find(DistAlias) != _dist_nd_by_name.end())
  {
    std::vector<std::string> a;
   return a;
//     BasicDistributionND * dist = _dist_nd_by_name.find(DistAlias)->second;
//     return getDistributionVariableNames(*dist);
  }
  else
  {
  throwError("getRavenDistributionVariableNames: Distribution " + DistAlias + " was not found in distribution container.");
  }
}

double
DistributionContainer::Pdf(const char * DistAlias, double x){
   return Pdf(std::string(DistAlias),x);
}

double
DistributionContainer::Pdf(const std::string DistAlias, double x){

    if(_dist_by_name.find(DistAlias) != _dist_by_name.end()){
       BasicDistribution * dist = _dist_by_name.find(DistAlias)->second;
       return DistributionPdf(*dist,x);
    }
    throwError("Pdf: Distribution " + DistAlias + " was not found in distribution container.");
    return -1.0;
}

double
DistributionContainer::Pdf(const char * DistAlias, std::vector<double> x)
{
   return Pdf(std::string(DistAlias),x);
}

double
DistributionContainer::Pdf(const std::string DistAlias, std::vector<double> x){

    if(_dist_nd_by_name.find(DistAlias) != _dist_nd_by_name.end()){
      BasicDistributionND * dist = _dist_nd_by_name.find(DistAlias)->second;
       return DistributionPdf(*dist,x);
    }
    throwError("Pdf: Distribution ND" + DistAlias + " was not found in distribution container.");
    return -1.0;
}

double
DistributionContainer::Cdf(const char * DistAlias, double x){
   return Cdf(std::string(DistAlias),x);
}

double
DistributionContainer::Cdf(const std::string DistAlias, double x){

   if(_dist_by_name.find(DistAlias) != _dist_by_name.end()){
       BasicDistribution * dist = _dist_by_name.find(DistAlias)->second;
       return DistributionCdf(*dist,x);
    }
    throwError("Cdf: Distribution " + DistAlias + " was not found in distribution container.");
    return -1.0;
}

double
DistributionContainer::Cdf(const char * DistAlias, std::vector<double> x){
  return Cdf(std::string(DistAlias),x);
}

double
DistributionContainer::Cdf(const std::string DistAlias, std::vector<double> x){

   if(_dist_nd_by_name.find(DistAlias) != _dist_nd_by_name.end()){
       BasicDistributionND * dist = _dist_nd_by_name.find(DistAlias)->second;
       return DistributionCdf(*dist,x);
       //return DistributionCdf(dist,x);
    }
    throwError("Cdf: Distribution ND" + DistAlias + " was not found in distribution container.");
    return -1.0;

}


double
DistributionContainer::inverseCdf(const char * DistAlias, double RNG) {
  return inverseCdf(std::string(DistAlias),RNG);
}

double
DistributionContainer::inverseCdf(const std::string DistAlias, double RNG) {
    if(_dist_by_name.find(DistAlias) != _dist_by_name.end()){
        BasicDistribution * dist = _dist_by_name.find(DistAlias)->second;
        //return dist->InverseCdf(RNG);
        return DistributionInverseCdf(*dist,RNG);
     }
     throwError("inverseCdf: Distribution " + DistAlias + " was not found in distribution container.");
     return -1.0;

}

double
DistributionContainer::getDistributionRandom(const char * DistAlias)
{
  return getDistributionRandom(std::string(DistAlias));
}

double
DistributionContainer::getDistributionRandom(const std::string DistAlias){

    if(_dist_by_name.find(DistAlias) != _dist_by_name.end()){
        BasicDistribution * dist = _dist_by_name.find(DistAlias)->second;
        //return dist->InverseCdf(RNG);
        return dist->getRandom(random());
     }
     throwError("getDistributionRandom: Distribution " + DistAlias + " was not found in distribution container.");
     return -1.0;

}

std::vector<double>
DistributionContainer::inverseCdf(const char * DistAlias, double min, double max){
   return inverseCdf(std::string(DistAlias),min,max);
}

std::vector<double>
DistributionContainer::inverseCdf(const std::string /*DistAlias*/, double /*min*/, double /*max*/){
    throwError("inverseCdf not yet implemented for MultiDimensional Distributions");
    return std::vector<double>(2,-1.0);
}

std::string DistributionContainer::lastDistributionTriggered(){
  if(atLeastADistTriggered()){
    return _last_dist_triggered;
  }
  else{
    return std::string("");
  }
}

bool DistributionContainer::atLeastADistTriggered(){return _at_least_a_dist_triggered;}


DistributionContainer & DistributionContainer::Instance() {
  if(_instance == NULL){
    _instance = new DistributionContainer();
  }
  return *_instance;
}

DistributionContainer * DistributionContainer::_instance = NULL;

/* the str_to_string_p and free_string_p are for python use */

std::string * str_to_string_p(char *s) {
  return new std::string(s);
}

const char * string_p_to_str(const std::string * s) {
  return s->c_str();
}

void free_string_p(std::string * s) {
  delete s;
}
