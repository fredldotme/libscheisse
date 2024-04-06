#ifndef LIBSCHEISSE_H
#define LIBSCHEISSE_H

#include <string>
#include <vector>

enum Tense {
    Tense_Unknown = 0,
    Praesens,
    Praeteritum,
    Perfekt,
    Plusquamperfekt,
    Futur1,
    Futur2
};

enum Mode {
    Mode_Unknown = 0,
    Indikativ,
    Imperativ,
    Konjunktiv1,
    Konjunktiv2
};

enum Case {
    Case_Unknown = 0,
    Nominativ,
    Genitiv,
    Dativ,
    Akkusativ
};

enum Type {
    Type_Unknown = 0,
    Nomen,
    Verb,
    Adjektiv,
    Adverb,
    Artikel,
    Pronomen,
    Praeposition,
    Konjunktion,
    Subjunktion
};

enum Genus {
    Neutrum = (1 << 0),
    Male = (1 << 1),
    Female = (1 << 2),
    AllGeni = Neutrum | Male | Female
};

enum TokenType {
    TokenType_Unknown = 0,
    Unmeaning,
    SentenceBeginning,
    SentenceEnd,
    Word
};

enum ScheissFlags {
    BeforeArticles = (1 << 0),
    RepeatArticle = (1 << 1),
    BeforeNomen = (1 << 2),
    Default = BeforeArticles | BeforeNomen
};

struct TokenAnalysis {
    std::string word;
    TokenType token_type = TokenType_Unknown;
    Type type = Type_Unknown;
};

std::vector<TokenAnalysis> analyse(const std::vector<std::string>& input);
std::vector<TokenAnalysis> analyse(const std::string& input);

std::string verscheissern(const std::vector<std::string>& input, const ScheissFlags flags = Default);
std::string verscheissern(const std::string& input, const ScheissFlags flags = Default);

#endif
