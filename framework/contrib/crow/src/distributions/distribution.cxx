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
#include "distribution.h"
#include <limits>
#include <iostream>

#define throwError(msg) { std::cerr << "\n\n" << msg << "\n\n"; throw std::runtime_error("Error"); }

BasicDistribution::BasicDistribution() {
  _forcing_method = NO_FORCING;
  _forced_constant = 0.0;
  //_dist_parameters["truncation"] = 1.0;
  //_dist_parameters.insert(std::pair<std::string,double>("truncation",1));
}

BasicDistribution::~BasicDistribution() {}

double
BasicDistribution::getVariable(std::string variable_name){
   double res;
   if(_dist_parameters.find(variable_name) != _dist_parameters.end()){
          res = _dist_parameters.find(variable_name) ->second;
   }
   else{
     throwError("Parameter " << variable_name << " was not found in distribution type " << _type <<".");
   }
   return res;
}

std::vector<double>
BasicDistribution::getVariableVector(std::string  variable_name){
        std::vector<double> res;
   if(_dist_vector_parameters.find(variable_name) != _dist_vector_parameters.end()){
         res = _dist_vector_parameters.find(variable_name) ->second;
   }
   else{
     throwError("Parameter " << variable_name << " was not found in distribution type " << _type <<".");
   }
   return res;
}

void
BasicDistribution::updateVariable(const std::string & variable_name, double & new_value){
   if(_dist_parameters.find(variable_name) != _dist_parameters.end()){
     // we are sure the variable_name is already present in the mapping =>
     // we can update it in the following way
     _dist_parameters[variable_name] = new_value;
   }
   else{
     throwError("Parameter " << variable_name << " was not found in distribution type " << _type << ".");
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
  for (std::map<std::string,double>::iterator it = _dist_parameters.begin(); it!= _dist_parameters.end();it++){
    paramtersNames.push_back(it->first);
  }
  return paramtersNames;
}

double
BasicDistribution::getRandom(double x)
{
  if(forcingMethod() == NO_FORCING) {
    return windowProcessing(x);
    //return inverseCdf(x);
  } else if(forcingMethod() == FORCED_VALUE) {
    return forcedConstant();
  } else if(forcingMethod() == FORCED_PROBABILITY) {
    return inverseCdf(forcedConstant());
  } else {
    throwError("Invalid forcing method " << forcingMethod());
  }
}

BasicDistribution::EForceRandom
BasicDistribution::forcingMethod()
{
  return _forcing_method;
}

double
BasicDistribution::forcedConstant()
{
  return _forced_constant;
}

void
BasicDistribution::setForcingMethod(EForceRandom forcing_method)
{
  _forcing_method = forcing_method;
}

void
BasicDistribution::setForcedConstant(double forced_constant)
{
  _forced_constant = forced_constant;
}

bool
BasicDistribution::hasParameter(std::string s)
{
  return _dist_parameters.find(s) != _dist_parameters.end();
}

double BasicDistribution::windowProcessing(double rng){
        double value;

   if(getVariable(std::string("PB_window_Low")) != 0.0 || getVariable(std::string("PB_window_Up")) != 1.0){ // interval Pb window
                double pbLOW = getVariable(std::string("PB_window_Low"));
                double pbUP  = getVariable(std::string("PB_window_Up"));
                double pb = pbLOW + (pbUP-pbLOW) * rng;
                value = inverseCdf(pb);
                //std::cerr << " pbLOW " << pbLOW << " pbUP " << pbUP << " pb " << pb << " value " << value << std::endl;
        }else if(getVariable(std::string("V_window_Low")) != -std::numeric_limits<double>::max() && getVariable(std::string("V_window_Up")) != std::numeric_limits<double>::max( )){ // interval V window
                double pbLOW = cdf(getVariable(std::string("V_window_Low")));
                double pbUP  = cdf(getVariable(std::string("V_window_Up")));
                double pb = pbLOW + (pbUP-pbLOW) * rng;
                value = inverseCdf(pb);
                //std::cerr << " valLOW " << valLOW << " valUP " << valUP << " value " << value << std::endl;
        }
        else // DEFAULT
                value = inverseCdf(rng);

        return value;
}
