/*
 * distribution.h
 *
 *  Created on: Nov 1, 2012
 *      Author: alfoa
 */

#ifndef DISTRIBUTION_H_
#define DISTRIBUTION_H_

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
//#include "Interpolation_Functions.h"
#include "distribution_min.h"

const int _defaultSeed = 1256955321;

enum truncation {MULTIPLICATIVE=1, SUM=2};

class distribution;

class DistributionContainer;

class BasicDistribution
{
public:
   BasicDistribution();
   virtual ~BasicDistribution();
   double  getVariable(const std::string variableName);                       ///< getVariable from mapping
   std::vector<double>  getVariableVector(const std::string  variableName);
   void updateVariable(const std::string & variableName, double & newValue);    ///< update variable into the mapping

  virtual double getRandom(double x);  //Same as inverseCdf, except it can be forced.

   virtual double  Pdf(double x) = 0;                                           ///< Pdf function at coordinate x
   virtual double  Cdf(double x) = 0;                                   ///< Cdf function at coordinate x
   virtual double  InverseCdf(double x) = 0;             ///< x

   virtual double untrPdf(double x) = 0;
   virtual double untrCdf(double x) = 0;
   virtual double untrCdfComplement(double x)  = 0;
   virtual double untrInverseCdf(double x) = 0;
   virtual double untrMean() = 0;
   virtual double untrMedian() = 0;
   virtual double untrMode() = 0;
   virtual double untrHazard(double x) = 0;

   std::string & getType();                                                     ///< Get distribution type
   unsigned int getSeed();                                                      ///< Get seed
   std::vector<std::string> getVariableNames();

   //virtual double windowProcessing(distribution & dist, double & RNG);

protected:
   enum force_random {NO_FORCING, FORCED_VALUE, FORCED_PROBABILITY};

   virtual force_random forcingMethod();
   virtual double forcedConstant();
   virtual void setForcingMethod(force_random forcingMethod);
   virtual void setForcedConstant(double forcedConstant);
   force_random _forcingMethod;
   double _forcedConstant;

   bool hasParameter(std::string);
   std::string _type;                              ///< Distribution type
   std::map <std::string,double> _dis_parameters;  ///< Distribution parameters
   std::map <std::string,std::vector<double> > _dis_vectorParameters;
   //Interpolation_Functions _interpolation;         ///< Interpolation class
   unsigned int _seed;                             ///< seed
   std::vector<double> _PBwindow;
   std::vector<double> _Vwindow;

   bool _checkStatus;
};



#endif /* DISTRIBUTION_H_ */
