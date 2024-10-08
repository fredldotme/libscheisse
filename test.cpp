#include "lib.h"

#include <iostream>

using namespace scheisse;

int main(int argc, char** argv)
{
    time_t current_time;
    srand((unsigned int)time(&current_time));

    const std::string test = "Weil der durch den Thorsten verursachte Dreck nicht geht ist die Scheiße am Dampfen. "
                             "Das ist aber i.d.r egal wenn der Hopfen fehlt. "
                             "Des Vodkas reinster Vergleichspunkt ist die der Kartoffel nachgesagte Verarbeitung. "
                             "Und die dem Anton gehörende Trompete macht einen unguten Eindruck. "
                             "Das traurige Entchen macht einen Bogen um das Thema. "
                             "Die am Bart klebende Sticker-Figur sieht aus, als würd sie gleich den Rotz vom Peter lecken. "
                             "Das Pisslaken stinkt nach einem Hitschaden mit einem Linkshaken. "
                             "Hier hat sich meine Katze gedacht, es wäre eine gute Idee die Vase runterzuwerfen. "
                             "Die nicht von der Hand zu weisende Problematik stinkt.";

    std::cout << "Eingabe:" << std::endl;
    std::cout << test << std::endl;

    std::cout << std::endl;

#if 1
    std::cout << "Analyse:" << std::endl;
    const auto analysis = analyse(test);
    for (const auto& token_analysis : analysis) {
        std::cout << "\tWord: " << token_analysis.word << std::endl;
        std::cout << "\tToken type: " << token_analysis.token_type << std::endl;
        std::cout << "\tType: " << token_analysis.type << std::endl;
        std::cout << "\tCase: " << token_analysis.casus << std::endl;
        std::cout << "\tGenus: " << token_analysis.genus << std::endl;
        if (token_analysis.dictating_token)
            std::cout << "\tDictating token: " << token_analysis.dictating_token->word << std::endl;
        std::cout << std::endl;
    }
#endif

    std::cout << std::endl;

    std::cout << "Verscheißert:" << std::endl;
    std::cout << verscheissern(test, static_cast<ScheissFlags>(ScheissFlags::Default | ScheissFlags::Debug)) << std::endl;
    return 0;
}
