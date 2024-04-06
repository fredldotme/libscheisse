#include "lib.h"

#include <iostream>

int main(int argc, char** argv)
{
    const std::string test = "Weil der Dreck nicht geht ist die Scheiße am Dampfen";

    std::cout << "Eingabe:" << std::endl;
    std::cout << test << std::endl;

    std::cout << std::endl;

    std::cout << "Verscheißert:" << std::endl;
    std::cout << verscheissern(test) << std::endl;
    return 0;
}