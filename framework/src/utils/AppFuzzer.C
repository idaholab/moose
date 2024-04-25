#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <cctype>

using namespace std;

string findClosestAppName(const string & input, const vector<string> & validNames){
    string input_lower = input;
    transform(input.begin(),
        input.end(),
        input_lower.begin(),
        [](unsigned char c) { return tolower(c); });

    for (const auto & name : validNames){
        string lower_name = name;
        transform(name.begin(),
            name.end(),
            lower_name.begin(), 
            [](unsigned char c){ 
                return tolower(c); }
        );
        if (input_lower == lower_name){
            return name;
        }
    }
    return "NothingClose"; 
}