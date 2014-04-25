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
    return InverseCdf(x);
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

double getDistributionVariable(BasicDistribution & dist,const std::string & variableName){
  return dist.getVariable(variableName);
}

void DistributionUpdateVariable(BasicDistribution & dist,const std::string & variableName, double newValue){
  dist.updateVariable(variableName, newValue);
}

double DistributionPdf(BasicDistribution & dist, double x){
  return dist.Pdf(x);
}

double DistributionCdf(BasicDistribution & dist, double x){
  return dist.Cdf(x);
}

double windowProcessing(BasicDistribution & dist, double RNG){
        double value;

//	if (dist.getVariableVector(std::string("PBwindow")).size()==1) // value Pb window
//		value=dist.InverseCdf(RNG);
//	else if(dist.getVariableVector(std::string("PBwindow")).size()==2){	// interval Pb window
//		double pbLOW = dist.getVariableVector(std::string("PBwindow"))[0];
//		double pbUP  = dist.getVariableVector(std::string("PBwindow"))[1];
//		double pb=pbLOW+(pbUP-pbLOW)*RNG;
//		value=dist.InverseCdf(pb);
//	}
//	else if(dist.getVariableVector(std::string("Vwindow")).size()==1)	// value V window
//		value=RNG;
//	else if(dist.getVariableVector(std::string("Vwindow")).size()==2){	// interval V window
//		double valLOW = dist.getVariableVector(std::string("Vwindow"))[0];
//		double valUP  = dist.getVariableVector(std::string("Vwindow"))[1];
//		value=valLOW+(valUP-valLOW)*RNG;
//	}
//	else	// DEFAULT
//		value = dist.InverseCdf(RNG);

   if(dist.getVariable(std::string("PB_window_Low")) != 0.0 || dist.getVariable(std::string("PB_window_Up")) != 1.0){	// interval Pb window
                double pbLOW = dist.getVariable(std::string("PB_window_Low"));
                double pbUP  = dist.getVariable(std::string("PB_window_Up"));
                double pb = pbLOW + (pbUP-pbLOW) * RNG;
                value = dist.InverseCdf(pb);
                //std::cerr << " pbLOW " << pbLOW << " pbUP " << pbUP << " pb " << pb << " value " << value << std::endl;
        }else if(dist.getVariable(std::string("V_window_Low")) != -std::numeric_limits<double>::max() && dist.getVariable(std::string("V_window_Up")) != std::numeric_limits<double>::max( )){	// interval V window
                double valLOW = dist.getVariable(std::string("V_window_Low"));
                double valUP  = dist.getVariable(std::string("V_window_Up"));
                value=valLOW+(valUP-valLOW)*RNG;
                //std::cerr << " valLOW " << valLOW << " valUP " << valUP << " value " << value << std::endl;
        }
        else	// DEFAULT
                value = dist.InverseCdf(RNG);

        return value;
}

double DistributionInverseCdf(BasicDistribution & dist, double x){
  //double standardRNG = dist.InverseCdf(x);
  double windowedRNG = windowProcessing(dist, x);

  return windowedRNG;
}

double untrDistributionPdf(BasicDistribution & dist, double & x){
  return dist.untrPdf(x);
}

double untrDistributionCdf(BasicDistribution & dist, double & x){
  return dist.untrCdf(x);
}

double untrDistributionInverseCdf(BasicDistribution & dist, double & x){
  return dist.untrInverseCdf(x);
}

std::string getDistributionType(BasicDistribution & dist) {
  return dist.getType();
}

std::vector<std::string>
getDistributionVariableNames(BasicDistribution & dist)
{
  return dist.getVariableNames();
}
