// debug_app.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "external_functions.h"
#include <math.h>

int main()
{
    double offs[4]{ 2324001.770, 606500.200,0,3.0466/180.0*3.14159265 };
    run_landxml2iges("C:\\Users\\Georg\\Documents\\GitHub\\renga_surfaces_work_plugin\\test_data\\push_15_test.xml", offs);
    std::cout << "Hello World!\n";
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
