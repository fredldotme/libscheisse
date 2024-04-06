#include "lib.h"

#include <iostream>

int main(int argc, char** argv)
{
    std::vector<std::string> test = {
        "Weil", "der", "Dreck", "nicht", "geht", "ist", "die", "Scheiße", "am", "Dampfen"
    };

    std::cout << "Eingabe:" << std::endl;
    for (const auto& word : test)
        std::cout << word << " ";

    std::cout << std::endl << std::endl;

    std::cout << "Verscheißert:" << std::endl;
    std::cout << verscheissern(test) << std::endl;
    return 0;
}