#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include "mooseFork/moose/framework/include/utils/AppFuzzer.h"

using namespace std;

// Test function
bool testFindClosestAppName(const string& input, const vector<string>& validNames, const string& expectedOutput) {
    string result = findClosestAppName(input, validNames);
    if (result == expectedOutput) {
        return true;
    } else {
        cout << "Test failed: Expected '" << expectedOutput << "', but got '" << result << "' for input '" << input << "'" << endl;
        return false;
    }
}

int main() {
    vector<string> validNames = {"Calculator", "Calendar", "Camera"};

    // Tests
    bool test1 = testFindClosestAppName("Calendar", validNames, "Calendar");
    bool test2 = testFindClosestAppName("calendar", validNames, "Calendar");
    bool test3 = testFindClosestAppName("Photos", validNames, "NothingClose");

    if (test1 && test2 && test3) {
        cout << "All tests passed!" << endl;
    } else {
        cout << "Some tests failed." << endl;
    }

    return 0;
}
