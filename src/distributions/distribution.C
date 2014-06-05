/*
 * distribution.C
 *
 *  Created on: Nov 1, 2012
 *      Author: alfoa
 */
#include "distribution.h"
#include <limits>
#include <iostream>

using namespace std;

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

BasicDistribution::BasicDistribution() {
  _forcingMethod = NO_FORCING;
  _forcedConstant = 0.0;
  //_dis_parameters["truncation"] = 1.0;
  //_dis_parameters.insert(std::pair<std::string,double>("truncation",1));
}

BasicDistribution::~BasicDistribution() {}

double
BasicDistribution::getVariable(std::string variableName){
   double res;
   if(_dis_parameters.find(variableName) != _dis_parameters.end()){
          res = _dis_parameters.find(variableName) ->second;
   }
   else{
     throwError("Parameter " << variableName << " was not found in distribution type " << _type <<".");
   }
   return res;
}

std::vector<double>
BasicDistribution::getVariableVector(std::string  variableName){
        std::vector<double> res;
   if(_dis_vectorParameters.find(variableName) != _dis_vectorParameters.end()){
         res = _dis_vectorParameters.find(variableName) ->second;
   }
   else{
     throwError("Parameter " << variableName << " was not found in distribution type " << _type <<".");
   }
   return res;
}

void
BasicDistribution::updateVariable(const std::string & variableName, double & newValue){
   if(_dis_parameters.find(variableName) != _dis_parameters.end()){
     // we are sure the variableName is already present in the mapping =>
     // we can update it in the following way
     _dis_parameters[variableName] = newValue;
   }
   else{
     throwError("Parameter " << variableName << " was not found in distribution type " << _type << ".");
   }
}

std::string &
BasicDistribution::getType(){
   return _type;
}

unsigned int BasicDistribution::getSeed() {
  return _seed;
}

std::vector<std::string>
BasicDistribution::getVariableNames(){
  std::vector<std::string> paramtersNames;
  for (std::map<std::string,double>::iterator it = _dis_parameters.begin(); it!= _dis_parameters.end();it++){
    paramtersNames.push_back(it->first);
  }
  return paramtersNames;
}

double
BasicDistribution::getRandom(double x)
{
  if(forcingMethod() == NO_FORCING) {
    return windowProcessing(x);
    //return InverseCdf(x);
  } else if(forcingMethod() == FORCED_VALUE) {
    return forcedConstant();
  } else if(forcingMethod() == FORCED_PROBABILITY) {
    return InverseCdf(forcedConstant());
  } else {
    throwError("Invalid forcing method " << forcingMethod());
  }
}

BasicDistribution::force_random
BasicDistribution::forcingMethod()
{
  return _forcingMethod;
}

double
BasicDistribution::forcedConstant()
{
  return _forcedConstant;
}

void
BasicDistribution::setForcingMethod(force_random forcingMethod)
{
  _forcingMethod = forcingMethod;
}

void
BasicDistribution::setForcedConstant(double forcedConstant)
{
  _forcedConstant = forcedConstant;
}

bool
BasicDistribution::hasParameter(std::string s)
{
  return _dis_parameters.find(s) != _dis_parameters.end();
}

double BasicDistribution::windowProcessing(double RNG){
        double value;

   if(getVariable(std::string("PB_window_Low")) != 0.0 || getVariable(std::string("PB_window_Up")) != 1.0){	// interval Pb window
                double pbLOW = getVariable(std::string("PB_window_Low"));
                double pbUP  = getVariable(std::string("PB_window_Up"));
                double pb = pbLOW + (pbUP-pbLOW) * RNG;
                value = InverseCdf(pb);
                //std::cerr << " pbLOW " << pbLOW << " pbUP " << pbUP << " pb " << pb << " value " << value << std::endl;
        }else if(getVariable(std::string("V_window_Low")) != -std::numeric_limits<double>::max() && getVariable(std::string("V_window_Up")) != std::numeric_limits<double>::max( )){	// interval V window
                double pbLOW = Cdf(getVariable(std::string("V_window_Low")));
                double pbUP  = Cdf(getVariable(std::string("V_window_Up")));
                double pb = pbLOW + (pbUP-pbLOW) * RNG;
                value = InverseCdf(pb);
                //std::cerr << " valLOW " << valLOW << " valUP " << valUP << " value " << value << std::endl;
        }
        else	// DEFAULT
                value = InverseCdf(RNG);

        return value;
}
