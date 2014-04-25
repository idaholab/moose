#ifndef DISTRIBUTION_MIN_H_
#define DISTRIBUTION_MIN_H_

#include "distribution_type.h"

class BasicDistribution;
class BasicDistributionND;

double getDistributionVariable(BasicDistribution & dist, const std::string & variableName);
void DistributionUpdateVariable(BasicDistribution & dist, const std::string & variableName, double newValue);
double DistributionPdf(BasicDistribution & dist,double x);
double DistributionCdf(BasicDistribution & dist,double x);
double DistributionInverseCdf(BasicDistribution & dist, double x);
std::string getDistributionType(BasicDistribution & dist);
std::vector<std::string> getDistributionVariableNames(BasicDistribution & dist);


//double DistributionPdf(BasicDistributionND & dist,std::vector<double> & x);
//double DistributionCdf(BasicDistributionND & dist,std::vector<double> & x);
//std::string getDistributionType(BasicDistributionND & dist);
//double getDistributionVariable(BasicDistributionND & dist, std::string & variableName);
//void DistributionUpdateVariable(BasicDistributionND & dist, std::string & variableName, double & newValue);


#endif /* DISTRIBUTION_MIN_H_ */
