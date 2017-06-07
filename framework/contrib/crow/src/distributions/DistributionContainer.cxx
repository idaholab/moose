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
/*
 * DistributionContainer.C
 *
 *  Created on: Jul 6, 2012
 *      Author: alfoa
 */
#include "DistributionContainer.h"
//#include "distribution_base_ND.h"
#include "distributionNDBase.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <map>
//#include <MooseRandom.h>

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

class DistributionContainer;


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
DistributionContainer::addDistributionInContainer(const std::string & /*type*/, const std::string & name, MooseSharedPointer<BasicDistribution> dist){
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
DistributionContainer::addDistributionInContainerND(const std::string & /*type*/, const std::string & name, MooseSharedPointer<BasicDistributionND> dist){
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
DistributionContainer::getType(const char *  dist_alias){
  return getType(std::string(dist_alias));
}

std::string
DistributionContainer::getType(const std::string dist_alias){

    if(_dist_by_name.find(dist_alias) != _dist_by_name.end())
    {
      MooseSharedPointer<BasicDistribution> dist = _dist_by_name.find(dist_alias)->second;
      std::string type = dist->getType();
      if(type == "DistributionError")
      {
        throwError("getType: Type for distribution " << dist_alias << " not found");
      }
      return type;
    }
    else if (_dist_nd_by_name.find(dist_alias) != _dist_nd_by_name.end())
    {
      MooseSharedPointer<BasicDistributionND> dist = _dist_nd_by_name.find(dist_alias)->second;
      std::string type = dist->getType();
      if(type == "DistributionError")
      {
        throwError("getType :Type for distribution " << dist_alias << " not found");
      }
      return type;
    }
    else{
       throwError("getType: Distribution " << dist_alias << " not found in distribution container");
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
DistributionContainer::checkCdf(const std::string dist_alias, double value){
  bool result;
  if (cdf(std::string(dist_alias),value) >= getVariable(dist_alias,"ProbabilityThreshold")){
    result=true;
    _dist_by_trigger_status[dist_alias] = true;
    _last_dist_triggered = dist_alias;
    _at_least_a_dist_triggered = true;
  }
  else{
    result=false;
    _dist_by_trigger_status[dist_alias] = false;
  }
  return result;
}

bool DistributionContainer::checkCdf(const std::string dist_alias, std::vector<double> value){
  bool result;
  if (cdf(std::string(dist_alias),value) >= getVariable(dist_alias,"ProbabilityThreshold")){
    result=true;
    _dist_by_trigger_status[dist_alias] = true;
    _last_dist_triggered = dist_alias;
    _at_least_a_dist_triggered = true;
  }
  else{
    result=false;
    _dist_by_trigger_status[dist_alias] = false;
  }
  return result;
}

bool
DistributionContainer::getTriggerStatus(const std::string dist_alias){
  bool st;
  if(_dist_by_trigger_status.find(dist_alias) != _dist_by_trigger_status.end()){
    st = _dist_by_trigger_status.find(dist_alias) -> second;
  }
  else{
    throwError("getTriggerStatus:Distribution " + dist_alias + " not found in Triggering event.");
  }
  return st;
}
bool
DistributionContainer::getTriggerStatus(const char * dist_alias){
  return getTriggerStatus(std::string(dist_alias));
}
// to be implemented
bool DistributionContainer::checkCdf(const char * dist_alias, double value){
  return checkCdf(std::string(dist_alias),value);
}
bool DistributionContainer::checkCdf(const char * dist_alias, std::vector<double> value){
  return checkCdf(std::string(dist_alias),value);
}
// end to be implemented

double
DistributionContainer::getVariable(const char *dist_alias, const char * param_name){
  return getVariable(std::string(dist_alias),std::string(param_name));
}

double
DistributionContainer::getVariable(const std::string dist_alias, const std::string param_name){
    if(_dist_by_name.find(dist_alias) != _dist_by_name.end())
    {
      MooseSharedPointer<BasicDistribution> dist = _dist_by_name.find(dist_alias)->second;
      return dist->getVariable(param_name);
    }
    else if (_dist_nd_by_name.find(dist_alias) != _dist_nd_by_name.end())
    {
      MooseSharedPointer<BasicDistributionND> dist = _dist_nd_by_name.find(dist_alias)->second;
      return dist->getVariable(param_name);
    }
    throwError("getVariable: Distribution " << dist_alias << " not found in distribution container");
    return -1;
}

void
DistributionContainer::updateVariable(const char *dist_alias,const char * param_name,double new_value){
  updateVariable(std::string(dist_alias),std::string(param_name),new_value);
}

void
DistributionContainer::updateVariable(const std::string dist_alias,const std::string param_name,double new_value){
    if(_dist_by_name.find(dist_alias) != _dist_by_name.end())
    {
      MooseSharedPointer<BasicDistribution> dist = _dist_by_name.find(dist_alias)->second;
      dist->updateVariable(param_name, new_value);
    }
    else if (_dist_nd_by_name.find(dist_alias) != _dist_nd_by_name.end())
    {
      MooseSharedPointer<BasicDistributionND> dist = _dist_nd_by_name.find(dist_alias)->second;
      dist->updateVariable(param_name, new_value);
    }
    else
    {
       throwError("updateVariable: Distribution " + dist_alias + " was not found in distribution container.");

    }
}

std::vector<std::string>
DistributionContainer::getDistributionNames(){
  std::vector<std::string> distsNames;
  for(std::map<std::string, MooseSharedPointer<BasicDistribution> >::iterator it = _dist_by_name.begin(); it!= _dist_by_name.end();it++)
  {
    distsNames.push_back(it->first);
  }
  for(std::map<std::string, MooseSharedPointer<BasicDistributionND> >::iterator it = _dist_nd_by_name.begin(); it!= _dist_nd_by_name.end();it++)
  {
    distsNames.push_back(it->first);
  }
  return distsNames;
}

std::vector<std::string>
DistributionContainer::getDistributionVariableNames(const std::string dist_alias){
  if(_dist_by_name.find(dist_alias) != _dist_by_name.end())
  {
    MooseSharedPointer<BasicDistribution> dist = _dist_by_name.find(dist_alias)->second;
    return dist->getVariableNames();
  }
  else if(_dist_nd_by_name.find(dist_alias) != _dist_nd_by_name.end())
  {
    std::vector<std::string> a;
   return a;
//     BasicDistributionND * dist = _dist_nd_by_name.find(dist_alias)->second;
//     return getDistributionVariableNames(*dist);
  }
  else
  {
  throwError("getDistributionVariableNames: Distribution " + dist_alias + " was not found in distribution container.");
  }
}

double
DistributionContainer::pdf(const char * dist_alias, double x){
   return pdf(std::string(dist_alias),x);
}

double
DistributionContainer::pdf(const std::string dist_alias, double x){

    if(_dist_by_name.find(dist_alias) != _dist_by_name.end()){
      MooseSharedPointer<BasicDistribution> dist = _dist_by_name.find(dist_alias)->second;
      return dist->pdf(x);
    }
    throwError("pdf: Distribution " + dist_alias + " was not found in distribution container.");
    return -1.0;
}

double
DistributionContainer::pdf(const char * dist_alias, std::vector<double> x)
{
   return pdf(std::string(dist_alias),x);
}

double
DistributionContainer::pdf(const std::string dist_alias, std::vector<double> x){

    if(_dist_nd_by_name.find(dist_alias) != _dist_nd_by_name.end()){
      MooseSharedPointer<BasicDistributionND> dist = _dist_nd_by_name.find(dist_alias)->second;
      return dist->pdf(x);
    }
    throwError("pdf: Distribution ND" + dist_alias + " was not found in distribution container.");
    return -1.0;
}

double
DistributionContainer::cdf(const char * dist_alias, double x){
   return cdf(std::string(dist_alias),x);
}

double
DistributionContainer::cdf(const std::string dist_alias, double x){

   if(_dist_by_name.find(dist_alias) != _dist_by_name.end()){
     MooseSharedPointer<BasicDistribution> dist = _dist_by_name.find(dist_alias)->second;
     return dist->cdf(x);
    }
    throwError("cdf: Distribution " + dist_alias + " was not found in distribution container.");
    return -1.0;
}

double
DistributionContainer::cdf(const char * dist_alias, std::vector<double> x){
  return cdf(std::string(dist_alias),x);
}

double
DistributionContainer::cdf(const std::string dist_alias, std::vector<double> x){

   if(_dist_nd_by_name.find(dist_alias) != _dist_nd_by_name.end()){
     MooseSharedPointer<BasicDistributionND> dist = _dist_nd_by_name.find(dist_alias)->second;
     return dist->cdf(x);
    }
    throwError("cdf: Distribution ND" + dist_alias + " was not found in distribution container.");
    return -1.0;

}


double
DistributionContainer::inverseCdf(const char * dist_alias, double rng) {
  return inverseCdf(std::string(dist_alias),rng);
}

double
DistributionContainer::inverseCdf(const std::string dist_alias, double rng) {
    if(_dist_by_name.find(dist_alias) != _dist_by_name.end()){
      MooseSharedPointer<BasicDistribution> dist = _dist_by_name.find(dist_alias)->second;
      return dist->inverseCdf(rng);
     }
     throwError("inverseCdf: Distribution " + dist_alias + " was not found in distribution container.");
     return -1.0;

}

double
DistributionContainer::getDistributionRandom(const char * dist_alias)
{
  return getDistributionRandom(std::string(dist_alias));
}

double
DistributionContainer::getDistributionRandom(const std::string dist_alias){

    if(_dist_by_name.find(dist_alias) != _dist_by_name.end()){
      MooseSharedPointer<BasicDistribution> dist = _dist_by_name.find(dist_alias)->second;
        //return dist->inverseCdf(rng);
      return dist->getRandom(random());
     }
     throwError("getDistributionRandom: Distribution " + dist_alias + " was not found in distribution container.");
     return -1.0;

}

std::vector<double>
DistributionContainer::inverseCdf(const char * dist_alias, double f, double g){
   return inverseCdf(std::string(dist_alias),f,g);
}

std::vector<double>
DistributionContainer::inverseCdf(const std::string dist_alias, double f, double g){

   if(_dist_nd_by_name.find(dist_alias) != _dist_nd_by_name.end()){
     MooseSharedPointer<BasicDistributionND> dist = _dist_nd_by_name.find(dist_alias)->second;
     return dist->inverseCdf(f,g);
    }
    throwError("inverseCdf: Distribution ND" + dist_alias + " was not found in distribution container.");
    std::vector<double> value (2,-1.0);
    return value;

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

int DistributionContainer::returnDimensionality(const std::string dist_alias){
   if(_dist_by_name.find(dist_alias) != _dist_by_name.end()){
         MooseSharedPointer<BasicDistribution> dist = _dist_by_name.find(dist_alias)->second;
         return dist->returnDimensionality();
        }
        throwError("returnDimensionality: Distribution " + dist_alias + " was not found in distribution container.");
        return -1.0;
}

int DistributionContainer::returnDimensionality(const char * dist_alias){
        return returnDimensionality(std::string(dist_alias));
}

DistributionContainer & DistributionContainer::instance() {
  if(_instance == NULL){
    _instance = new DistributionContainer();
  }
  return *_instance;
}

DistributionContainer * DistributionContainer::_instance = NULL;

/* the strToStringP and freeStringP are for python use */

std::string * strToStringP(char *s) {
  return new std::string(s);
}

const char * stringPToSt(const std::string * s) {
  return s->c_str();
}

void freeStringP(std::string * s) {
  delete s;
}
