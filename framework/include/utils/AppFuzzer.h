// AppFuzzer.h
#ifndef APPFUZZER_H
#define APPFUZZER_H

#include <string>
#include <vector>

std::string findClosestAppName(const std::string &input, const std::vector<std::string> &validNames);

#endif // APPFUZZER_H
