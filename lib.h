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
    Konjunktion
};

enum ScheissFlags {
    BeforeArticles = (1 << 0),
    RepeatArticle = (1 << 1),
    Default = BeforeArticles
};

std::string verscheissern(const std::vector<std::string>& input, const ScheissFlags flags = Default);
std::string verscheissern(const std::string& input, const ScheissFlags flags = Default);

#endif
