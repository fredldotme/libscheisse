#include "lib.h"

#include <iostream>

int main(int argc, char** argv)
{
    const std::string test = "Weil der Dreck nicht geht ist die Scheiße am Dampfen.";

    std::cout << "Eingabe:" << std::endl;
    std::cout << test << std::endl;

    std::cout << std::endl;

    std::cout << "Analyse:" << std::endl;
    const auto analysis = analyse(test);
    for (const auto& token_analysis : analysis) {
        std::cout << "\tWord: " << token_analysis.word << std::endl;
        std::cout << "\tType: " << token_analysis.type << std::endl;
        std::cout << std::endl;
    }

    std::cout << std::endl;

    std::cout << "Verscheißert:" << std::endl;
    std::cout << verscheissern(test) << std::endl;
    return 0;
}