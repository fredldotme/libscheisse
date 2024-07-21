#include "lib.h"

#include <algorithm>
#include <array>

#ifndef INDEVELOPMENT
#define INDEVELOPMENT 0
#endif

// As in "In Spe", determines potentially resulting word
struct Spe {
    std::string word;
    bool regular; // False means Verscheisserung
};

struct Alternative {
    std::string word;
    Genus genus;
    Case casus;
};

// TODO: Verteilung der Auswahl alternativer Schreibweisen

// With articles
static const std::vector<Alternative> alternatives_known = {
    Alternative{"scheiß", AllGeni, AllCases},
    Alternative{"beschissene", AllGeni, Nominativ},
    Alternative{"beschissenen", AllGeni, Genitiv},
    Alternative{"beschissenen", AllGeni, Dativ},
    Alternative{"beschissenen", Male, Akkusativ},
    Alternative{"beschissene", Female, Akkusativ},
    Alternative{"beschissene", Neutrum, Akkusativ}
};

// Without articles
static const std::vector<Alternative> alternatives_unknown = {
    Alternative{"scheiß", AllGeni, AllCases},
    Alternative{"beschissener", Male, Nominativ},
    Alternative{"beschissene", Female, Nominativ},
    Alternative{"beschissenes", Neutrum, Nominativ},
    Alternative{"beschissenen", Male, Genitiv},
    Alternative{"beschissener", Female, Genitiv},
    Alternative{"beschissenen", Neutrum, Genitiv},
    Alternative{"beschissenem", Male, Dativ},
    Alternative{"beschissener", Female, Dativ},
    Alternative{"beschissenem", Neutrum, Dativ},
    Alternative{"beschissenen", Male, Akkusativ},
    Alternative{"beschissene", Female, Akkusativ},
    Alternative{"beschissenes", Neutrum, Akkusativ}
};

// Mixed flexion
static const std::vector<Alternative> alternatives_mixed_flex = {
    Alternative{"scheiß", AllGeni, AllCases},
    Alternative{"beschissener", Male, Nominativ},
    Alternative{"beschissene", Female, Nominativ},
    Alternative{"beschissenes", Neutrum, Nominativ},
    Alternative{"beschissenen", AllGeni, Genitiv},
    Alternative{"beschissenen", AllGeni, Dativ},
    Alternative{"beschissenen", Male, Akkusativ},
    Alternative{"beschissene", Female, Akkusativ},
    Alternative{"beschissenes", Neutrum, Akkusativ}
};

// Always pick this one as a fallback
static const int default_alternative = 0;

// Coin toss moment: Common prefix to avoid over-selecting similar alternatives
// TODO: Adapt users to an array.size() > 1
static const std::array<std::string, 1> alternative_prefixes = {
    "besch" // beschissener, beschissene
};

static const unsigned char question_mark = '?';
static const unsigned char exclamation_mark = '!';
static const unsigned char period = '.';

struct Replacement {
    std::string original;
    std::string replacement;
};

static const std::array<Replacement, 1> replacements = {
    Replacement{"Dreck", "Scheißdreck"}
};

static inline bool replacement_search(const std::string& word) {
    for (const auto& replacement : replacements) {
        if (replacement.original == word)
            return true;
    }
    return false;
}

static inline Spe replace_token(const std::string& word) {
    for (const auto& replacement : replacements) {
        if (replacement.original == word)
            return { replacement.replacement, false }; 
    }
    return { word, true };
}

static inline std::string to_lowercase(const std::string& source)
{
    std::string ret;
    for (const auto& c : source) {
        ret += static_cast<unsigned char>(std::tolower(c));
    }
    return ret;
}

static inline bool strict_search(const std::string& source_term, const std::string& search_term)
{
    return source_term == search_term;
}

static inline bool insensitive_search(const std::string& source_term, const std::string& search_term)
{
    const std::string source = to_lowercase(source_term);
    const std::string test = to_lowercase(search_term);
    return source.find(test) == 0 && source.length() == test.length();
}

static inline bool fuzzy_search(const std::string& source_term, const std::string& search_term)
{
    const std::string source = to_lowercase(source_term);
    const std::string test = to_lowercase(search_term);
    return source.find(test) == std::string::npos && source.length() < test.length();
}

static inline bool contains_alternative(const std::string& term, std::string* found_alternative = nullptr)
{
#define TRY_RETURN_IF_FOUND(arr, term, found_alternative) \
    for (const auto& alternative : arr) { \
            if (term.find(alternative.word) != std::string::npos && \
                alternative.word.find(alternative_prefixes[0]) == 0) { \
                if (found_alternative) \
                    *found_alternative = alternative.word; \
                return true; \
        } \
    }

    TRY_RETURN_IF_FOUND(alternatives_known, term, found_alternative);
    TRY_RETURN_IF_FOUND(alternatives_unknown, term, found_alternative);
    TRY_RETURN_IF_FOUND(alternatives_mixed_flex, term, found_alternative);
#undef TRY_RETURN_IF_FOUND

    return false;
}

static inline std::string random_scheiss(const TokenAnalysis& token)
{

    const auto& alternatives = (token.type == Artikel && token.declination_type != Declination_Unknown) ?
                                   (token.declination_type == MixedFlexion ? alternatives_mixed_flex : alternatives_known) :
                                   alternatives_unknown;

    for (int index = default_alternative + 1; index < alternatives.size(); index++) {
        const auto& alternative = alternatives[index];
        if ((alternative.genus & token.genus) && (alternative.casus & token.casus)) {
            // Coin toss whether to use the default, for spreading probabilities
            if (rand() % 10 == 0)
                return alternatives[default_alternative].word;
            else
                return alternative.word;
        }
    }
    return alternatives[default_alternative].word;
}

static inline std::vector<std::string> split_string(const std::string& to_split)
{
    std::vector<std::string> ret;
    std::string tmp;

#define PUSH_IF_POSSIBLE(tmp) \
    if (!tmp.empty()) { ret.push_back(tmp); tmp = ""; }

    for (const auto& c : to_split) {
        if (c == ' ') {
            PUSH_IF_POSSIBLE(tmp)
            continue;
        }
        tmp += c;
    }
    PUSH_IF_POSSIBLE(tmp)
#undef PUSH_IF_POSSIBLE

    return ret;
}

static inline std::string join_string(const std::vector<std::string>& to_join)
{
    std::string ret;
    for (auto it = to_join.cbegin(); it != to_join.cend(); it++) {
        const auto& joinable = *it;
        ret += joinable;
        if (it+1 != to_join.cend())
            ret += " ";
    }
    return ret;
}

static inline std::vector<std::string>::const_iterator find_duplicates(const std::vector<std::string>& clearable,
                                                                       const std::vector<std::string>& reference)
{
    for (auto fit = clearable.cbegin(); fit != clearable.cend(); fit++) {
        const auto& first_needle = *fit;
        for (auto sit = reference.cbegin(); sit != reference.cend(); sit++) {
            const auto& second_needle = *sit;
            if (first_needle == second_needle) {
                return fit;
            }
        }
    }
    return clearable.end();
}

static inline std::vector<Spe>::const_iterator find_duplicate_spes(const std::vector<Spe>& clearable,
                                                                   const std::vector<Spe>& reference)
{
    for (auto fit = clearable.cbegin(); fit != clearable.cend(); fit++) {
        const auto& first_needle = *fit;
        for (auto sit = reference.cbegin(); sit != reference.cend(); sit++) {
            const auto& second_needle = *sit;
            if (!first_needle.regular &&
                ((first_needle.word == second_needle.word) ||
                 (!second_needle.regular))) {
                return fit;
            }
        }
    }
    return clearable.cend();
}

static const std::array<std::string, 3> articles_nominativ_singular_known = {"der", "die", "das"};
static const std::array<std::string, 3> articles_genitiv_singular_known = {"des", "der", "des"};
static const std::array<std::string, 3> articles_dativ_singular_known = {"dem", "der", "dem"};
static const std::array<std::string, 3> articles_akkusativ_singular_known = {"den", "die", "das"};
static const std::array<std::string, 3> articles_nominativ_singular_mixedflex = {"ein", "eine", "ein"};
static const std::array<std::string, 3> articles_genitiv_singular_mixedflex = {"eines", "einer", "eines"};
static const std::array<std::string, 3> articles_dativ_singular_mixedflex = {"einem", "einer", "einem"};
static const std::array<std::string, 3> articles_akkusativ_singular_mixedflex = {"einen", "eine", "ein"};

static inline bool article_search(const std::string& term)
{
#define TRY_RETURN_SEARCH(arr, term) \
    for (const auto& article : arr) { \
            if (insensitive_search(term, article)) \
            return true; \
    }

    TRY_RETURN_SEARCH(articles_nominativ_singular_known, term);
    TRY_RETURN_SEARCH(articles_genitiv_singular_known, term);
    TRY_RETURN_SEARCH(articles_dativ_singular_known, term);
    TRY_RETURN_SEARCH(articles_akkusativ_singular_known, term);
#undef TRY_RETURN_SEARCH

    return false;
}

static inline Genus genus_for_article(const std::string& term)
{
    int index;

#define TRY_FIND_AND_DECIDE(arr, term, index) \
    index = 0; \
        for (const auto& article : arr) { \
            if (insensitive_search(term, article)) \
                goto decide; \
    }

    TRY_FIND_AND_DECIDE(articles_nominativ_singular_known, term, index);
    TRY_FIND_AND_DECIDE(articles_genitiv_singular_known, term, index);
    TRY_FIND_AND_DECIDE(articles_dativ_singular_known, term, index);
    TRY_FIND_AND_DECIDE(articles_akkusativ_singular_known, term, index);
    TRY_FIND_AND_DECIDE(articles_nominativ_singular_mixedflex, term, index);
    TRY_FIND_AND_DECIDE(articles_genitiv_singular_mixedflex, term, index);
    TRY_FIND_AND_DECIDE(articles_dativ_singular_mixedflex, term, index);
    TRY_FIND_AND_DECIDE(articles_akkusativ_singular_mixedflex, term, index);
#undef TRY_FIND_AND_DECIDE

decide:
    if (index == 0)
        return Male;
    else if (index == 1)
        return Female;
    else if (index == 2)
        return Neutrum;
    return Genus_Unknown;
}

static inline int article_index_for_genus(const Genus genus)
{
    switch (genus) {
    case Male: return 0;
    case Female: return 1;
    default: return 2;
    }
}

static inline Genus genus_for_article_index(const int index)
{
    switch (index) {
    case 0: return Male;
    case 1: return Female;
    default: return Neutrum;
    }
}

static inline std::vector<Genus> potential_genuses_for_article(const std::string& term)
{
    std::vector<Genus> genuses;

#define TRY_FIND(arr, term, gens) \
    for (int i = 0; i < 3; i++) { \
            const auto& article = arr[i]; \
            if (insensitive_search(term, article)) { \
                const auto found_genus = genus_for_article_index(i); \
                gens.push_back(found_genus); \
        } \
    }

    TRY_FIND(articles_nominativ_singular_known, term, genuses);
    TRY_FIND(articles_genitiv_singular_known, term, genuses);
    TRY_FIND(articles_dativ_singular_known, term, genuses);
    TRY_FIND(articles_akkusativ_singular_known, term, genuses);
    TRY_FIND(articles_nominativ_singular_mixedflex, term, genuses);
    TRY_FIND(articles_genitiv_singular_mixedflex, term, genuses);
    TRY_FIND(articles_dativ_singular_mixedflex, term, genuses);
    TRY_FIND(articles_akkusativ_singular_mixedflex, term, genuses);
#undef TRY_FIND

    return genuses;
}

static inline Case casus_for_article(const std::string& article, const Genus genus)
{
    if ((article == articles_nominativ_singular_known[0] && (genus & Male)) ||
        (article == articles_nominativ_singular_known[1] && (genus & Female)) ||
        (article == articles_nominativ_singular_known[2] && (genus & Neutrum)))
        return Nominativ;
    else if ((article == articles_genitiv_singular_known[0] && (genus & Male)) ||
             (article == articles_genitiv_singular_known[1] && (genus & Female)) ||
             (article == articles_genitiv_singular_known[2] && (genus & Neutrum)))
        return Genitiv;
    else if ((article == articles_dativ_singular_known[0] && (genus & Male)) ||
             (article == articles_dativ_singular_known[1] && (genus & Female)) ||
             (article == articles_dativ_singular_known[2] && (genus & Neutrum)))
        return Dativ;       
    else if ((article == articles_akkusativ_singular_known[0] && (genus & Male)) ||
             (article == articles_akkusativ_singular_known[1] && (genus & Female)) ||
             (article == articles_akkusativ_singular_known[2] && (genus & Neutrum)))
        return Akkusativ;

    return Case_Unknown;
}

static inline DeclinationType declination_type_for_article(const std::string& term, const Genus genus)
{
    if (!article_search(term) || genus == Genus_Unknown)
        return Declination_Unknown;

#define TRY_RETURN_TYPE(arr, term, genus, type) \
    if (arr[article_index_for_genus(genus)] == term) \
            return type;

    // With article?
    TRY_RETURN_TYPE(articles_nominativ_singular_known, term, genus, FlexionWithArticle);
    TRY_RETURN_TYPE(articles_genitiv_singular_known, term, genus, FlexionWithArticle);
    TRY_RETURN_TYPE(articles_dativ_singular_known, term, genus, FlexionWithArticle);
    TRY_RETURN_TYPE(articles_akkusativ_singular_known, term, genus, FlexionWithArticle);

    // Mixed flexion?
    TRY_RETURN_TYPE(articles_nominativ_singular_mixedflex, term, genus, MixedFlexion);
    TRY_RETURN_TYPE(articles_genitiv_singular_mixedflex, term, genus, MixedFlexion);
    TRY_RETURN_TYPE(articles_dativ_singular_mixedflex, term, genus, MixedFlexion);
    TRY_RETURN_TYPE(articles_akkusativ_singular_mixedflex, term, genus, MixedFlexion);
#undef TRY_RETURN_TYPE

    return FlexionWithoutArticle;
}

static inline Case preposition_search(const std::string& term)
{
    static const std::array<std::string, 6> akkusative_prepositions = {
        "bis", "durch", "für", "gegen", "ohne", "um"
    };
    static const std::array<std::string, 16> dative_prepositions = {
        "ab", "aus", "außer", "bei", "gegenüber", "mit", "nach", "nächst",
        "nebst", "seit", "von", "zu", "zufolge", "zuliebe", "zunächst", "zuwide"
    };
    for (const auto& preposition : akkusative_prepositions) {
        if (insensitive_search(term, preposition))
            return Akkusativ;
    }
    for (const auto& preposition : dative_prepositions) {
        if (insensitive_search(term, preposition))
            return Dativ;
    }
    return Case_Unknown;
}

static inline bool adverb_search(const std::string& term) {
    static const std::array<std::string, 228> adverbs = {
        "abends", "aber", "alle", "allein", "allerdings", "allzu", "alsbald", "also",
        "andererseits", "andernfalls", "anders", "anfangs", "auch", "aufwärts", "aussen",
        "ausserdem", "bald", "beieinander", "beinahe", "beizeiten", "bekanntlich",
        "bereits", "besonders", "bestens", "bisher", "bisschen", "bloss", "dabei",
        "dadurch", "dafür", "damals", "damit", "danach", "daneben", "dann", "daran",
        "darauf", "daraus", "darin", "darüber", "darum", "davon", "dazu", "dazwischen",
        "demnach", "derart", "dereinst", "deshalb", "deswegen", "doch", "dort", "dorther",
        "dorthin", "draussen", "drüben", "durchaus", "ebenso", "ehedem", "ehemals",
        "eher", "eigentlich", "eilends", "einfach", "einigermassen", "einmal", "eins",
        "einst", "einstmals", "endlich", "entgegen", "erst", "etwa", "etwas", "fast",
        "folglich", "fortan", "freilich", "ganz", "gegebenenfalls", "genug", "gern",
        "gestern", "gleich", "gleichfalls", "gleichwohl", "glücklicherweise", "günstigenfalls",
        "her", "heraus", "herein", "herum", "herunter", "heute", "hier", "hierauf", "hierbei",
        "hierdurch", "hierfür", "hierher", "hierhin", "hiermit", "hierzu", "hin", "hinauf",
        "hinein", "hinten", "hinterher", "hinunter", "höchstens", "hoffentlich", "immer",
        "innen", "irgendwo", "ja", "jawohl", "jedenfalls", "jemals", "jetzt", "kaum",
        "keinesfalls", "keineswegs", "kopfüber", "kurzerhand", "leider", "links", "los",
        "mal", "manchmal", "mehrmals", "meist", "meistens", "minder", "mindestens",
        "miteinander", "mithin", "mittags", "mittlerweile", "möglicherweise", "morgen",
        "morgens", "nacheinander", "nachher", "nächstens", "nachts",
        "nebenbei", "nebeneinander", "nein", "nicht", "nie", "niemals", "nirgends",
        "noch", "nochmals", "nötigenfalls", "nun", "nur", "oben", "oft", "öfters",
        "rechts,", "ringsum", "ringsumher", "rückwärts", "rundum", "schliesslich",
        "schlimmstenfalls", "schon", "schwerlich", "sehr", "seinerzeit", "seither",
        "seitwärts", "sicherlich", "so", "sodann", "soeben", "sofort", "sogar",
        "sogleich", "sonst", "stets", "tagsüber", "trotzdem", "überall", "überallhin",
        "überaus", "überdies", "überhaupt", "übrigens", "umsonst", "ungefähr", "ungestraft",
        "unglücklicherweise", "unten", "unterdessen", "untereinander", "unterwegs",
        "vergebens", "vermutlich", "vielleicht", "vielmals", "vielmehr", "vorbei", "vordem",
        "vorgestern", "vorher", "vorhin", "vormals", "vorn", "vorne", "wann", "warum",
        "weg", "weitaus", "weiter", "wenigstens", "wieder", "wiederum", "wirklich", "wo",
        "wohl", "zeitlebens", "zeitweise", "zuerst", "zugleich", "zuletzt", "zusammen", "zuweilen"
    };
    for (const auto& adverb : adverbs) {
        const auto input = to_lowercase(term);
        if (strict_search(adverb, input))
            return true;
    }
    return false;
}

static inline bool subjunction_search(const std::string& term)
{
    static const std::array<std::string, 28> subjunctions = {
        "als", "bevor", "bis", "da", "damit", "dass", "ehe", "falls", "indem",
        "nachdem", "ob", "obgleich", "obschon", "obwohl", "seit", "seitdem",
        "sobald", "sodass", "sofern", "solange", "sooft", "soweit", "soviel",
        "während", "weil", "wenn", "wie", "wohingegen"
    };
    for (const auto& subjunction : subjunctions) {
        const auto input = to_lowercase(term);
        if (strict_search(subjunction, input))
            return true;
    }
    return false;
}

#if INDEVELOPMENT
static inline bool verb_search(const std::string& term)
{

}
#endif

static inline bool nomen_check(const std::string& word)
{
    return word.length() >= 2 && word[0] == std::toupper(word[0]) && word[1] == std::tolower(word[1]);
}

static inline bool adjective_check(const TokenAnalysis* before, const std::string& word)
{
    return word.length() > 1 && std::tolower(word[0]) == word[0] &&
            (!before || (before && before->type == Artikel));
}

static inline std::vector<Spe> article_verscheissern(const TokenAnalysis* before, const TokenAnalysis& token, const TokenAnalysis* after)
{
    std::vector<Spe> ret;
    ret.push_back({token.word, true});

    // Opinionated workaround: with two articles in a row, prefer to verscheisser with the generic fallback.
    // Duplicate Spes will be filtered later
    if ((before && before->type == Artikel) || (after && after->type == Artikel)) {
        ret.push_back({alternatives_known[default_alternative].word, false});
    } else {
        const auto scheiss = random_scheiss(token);
        if (!scheiss.empty())
            ret.push_back({scheiss, false});
    }
    return ret;
}

static inline std::vector<Spe> nomen_verscheissern(const std::vector<TokenAnalysis>& analysis,
                                                   const TokenAnalysis* before,
                                                   const TokenAnalysis& token,
                                                   const TokenAnalysis* after)
{
    std::vector<Spe> ret;

    // Optimizations & workarounds: avoid a double verscheisserung
    // TODO: Maybe do this without ruling out completely valid grammatical circumstances.
    if (token.before_token && token.before_token->before_token &&
        (token.before_token->type == Artikel) && (token.before_token->before_token->type == Artikel)) {
        ret.push_back({token.word, true});
        return ret;
    }

    const auto scheiss = random_scheiss(token);
    if (!scheiss.empty())
        ret.push_back({scheiss, false});
    ret.push_back({token.word, true});
    return ret;
}

static inline std::vector<Spe> adjective_verscheissern(const std::vector<TokenAnalysis>& analysis,
                                                       const TokenAnalysis& token)
{
    if (token.before_token && token.before_token->token_type == SentenceBeginning &&
        token.before_token->type == Artikel &&
        token.after_token && token.after_token->type == Nomen) {
        return {
            {
                "scheiss-" + token.word,
                false
            }
        };
   }
   return { { token.word, true } };
}

static inline std::string strip_punctuation(const std::string& word, TokenAnalysis& followup_token)
{
    static const std::array<unsigned char, 6> punctuations = {',', ':', ';', period, question_mark, exclamation_mark };

    // Optimize short hand writing without stripping periods
    if (std::count(word.cbegin(), word.cend(), period) > 1)
        return word;

    std::string ret;
    for (const auto& c : word) {
        if (std::find(punctuations.begin(), punctuations.end(), c) != std::end(punctuations)) {
            const std::string punctuation(1, c);
            followup_token.word = punctuation;
            if (c == period || c == question_mark || c == exclamation_mark)
                followup_token.token_type = SentenceEnd;
            else
                followup_token.token_type = Unmeaning;
            continue;
        }
        ret += c;
    }
    return ret;
}

static inline std::vector<TokenAnalysis>::iterator find_next_nomen(std::vector<TokenAnalysis>::iterator begin,
                                                                   std::vector<TokenAnalysis>::iterator end)
{
    for (auto it = begin; it != end; it++) {
        if (nomen_check((*it).word))
            return it;
    }
    return end;
}

static inline Genus find_article_genus_for_casus(const std::string& article, const std::vector<Genus>& genuses, const Case casus)
{
    for (const auto& genus : genuses) {
        switch(casus) {
        case Nominativ: {
            const auto found = articles_nominativ_singular_known[article_index_for_genus(genus)];
            if (insensitive_search(found, article))
                return genus;
        }
        case Genitiv: {
            const auto found = articles_genitiv_singular_known[article_index_for_genus(genus)];
            if (insensitive_search(found, article))
                return genus;
        }
        case Dativ: {
            const auto found = articles_dativ_singular_known[article_index_for_genus(genus)];
            if (insensitive_search(found, article))
                return genus;
        }
        case Akkusativ: {
            const auto found = articles_akkusativ_singular_known[article_index_for_genus(genus)];
            if (insensitive_search(found, article))
                return genus;
        }
        default:
            break;
        }
    }

    return Genus_Unknown;
}

static inline void build_relations(std::vector<TokenAnalysis>& analysis)
{
    for (auto it = analysis.begin(); it != analysis.end(); it++) {
        auto& token = (*it);
        auto* look_backward_token = it != analysis.begin() ? &(*(it-1)) : nullptr;
        auto* peek_forward_token = &(*(it+1));

        token.before_token = look_backward_token;
        token.after_token = peek_forward_token;

        // If the followup token is also an article it's safe to assume
        // the relative word is placed way after.
        // Example: "Des Vodkas reinster Vergleichspunkt ist ->die der Kartoffel nachgesagte Verarbeitung<-."
        if (peek_forward_token && token.type == Artikel && (*peek_forward_token).type == Artikel) {
            // The one the second article pertains to
            auto first_nomen = find_next_nomen((it + 1), analysis.end());
            // The one this article pertains to
            auto second_nomen = find_next_nomen((first_nomen + 1), analysis.end());

            // Set up relationship
            (*first_nomen).dictating_token = &(*peek_forward_token);
            (*second_nomen).dictating_token = &(*it);

            // TODO: Figure out whether a jump-ahead optimization works
            //it = second_nomen;
            //continue;
        }
        // Less narrow case: regular article before a nomen
        else if (peek_forward_token && token.type == Artikel && (*peek_forward_token).type == Nomen) {
            (*peek_forward_token).dictating_token = &(*it);
        }
    }
}

std::vector<TokenAnalysis> analyse(const std::vector<std::string>& input)
{
    std::vector<TokenAnalysis> ret;

    for (auto it = input.cbegin(); it != input.cend(); it++) {
        const auto* previous_token = ret.size() > 0 ? &ret[ret.size() - 1] : nullptr;

        TokenAnalysis followup_token;
        const std::string word = strip_punctuation(*it, followup_token);

        Type type = Type_Unknown;
        Genus genus = Genus_Unknown;
        Case casus = Case_Unknown;
        DeclinationType declination_type = Declination_Unknown;
        TokenAnalysis* dictating_token = nullptr;
        TokenType token_type = (it == input.cbegin()) ? SentenceBeginning : TokenType_Unknown;
        if (previous_token  && (*previous_token).token_type == SentenceEnd)
            token_type = SentenceBeginning;

        if (article_search(word)) {
            type = Artikel;
            const auto potential_genuses = potential_genuses_for_article(word);

            // TODO: Clear winner? Assign immediately!
            /*if (potential_genuses.size() == 1) {
                genus = potential_genuses[0];
            }*/
            genus = genus_for_article(word);
            casus = casus_for_article(word, genus);
            declination_type = declination_type_for_article(word, genus);

            // Determine by narrowing down with more information available
            if (potential_genuses.size() > 1) {
                // TODO: finalize
                //genus = find_article_genus_for_casus(word, potential_genuses, casus);
            }
        }
#if INDEVELOPMENT
        else if (verb_search(word)) {
            type = Verb;
        }
#endif
        else if (adverb_search(word)) {
            type = Adverb;
        }
        else if (subjunction_search(word)) {
            type = Subjunktion;
        }
        // Check whether its a Nomen later to avoid beginnings of sentences to be misdetected.
        else if (nomen_check(word)) {
            type = Nomen;
            if (previous_token && article_search((*previous_token).word)) {
                genus = (*previous_token).genus;
                casus = (*previous_token).casus;
            }
        }
        else if (adjective_check(previous_token, word)) {
            type = Adjektiv;
        }

        ret.push_back(
            TokenAnalysis{nullptr, nullptr,
                dictating_token,
                word,
                token_type,
                type,
                genus,
                casus,
                declination_type
            });
        if (followup_token.token_type != TokenType_Unknown) {
            ret.push_back(followup_token);
            followup_token = TokenAnalysis();
        }
    }

    build_relations(ret);
    return ret;
}

std::vector<TokenAnalysis> analyse(const std::string& input)
{
    return analyse(split_string(input));
}

std::string verscheissern(const std::vector<std::string>& input, const ScheissFlags flags)
{
    std::vector<std::string> ret;
    std::vector<Spe> previous_spes;
    const auto analysis = analyse(input);

    for (auto it = analysis.cbegin(); it != analysis.cend(); it++) {
        auto token = (*it);
        const auto* look_backward_token = it != analysis.cbegin() ? &(*(it-1)) : nullptr;
        const auto* peek_forward_token = &(*(it+1));

        // Capitalize beginning of a sentence
        if (!token.word.empty() && token.token_type == SentenceBeginning)
            token.word[0] = std::toupper(token.word[0]);

        // Like "In Spe"
        std::vector<Spe> spes;

        if (replacement_search(token.word)) {
            spes.push_back(replace_token(token.word));
        } else if (((flags & ScheissFlags::BeforeArticles) && token.type == Artikel) &&
            peek_forward_token && (*peek_forward_token).type == Nomen) {
            spes = article_verscheissern(look_backward_token, token, peek_forward_token);
        } else if ((flags & ScheissFlags::BeforeNomen) && token.type == Nomen &&
                   (look_backward_token && token.dictating_token == look_backward_token) &&
                   /* TODO: Remove token_type check once smarter */
                   (token.token_type != SentenceBeginning)) {
            spes = nomen_verscheissern(analysis, look_backward_token, token, peek_forward_token);
        } else if ((flags & ScheissFlags::BeforeAdjectives) && token.type == Adjektiv &&
                   /* TODO: Remove token_type check once smarter */
                   (token.token_type != SentenceBeginning)) {
            spes = adjective_verscheissern(analysis, token);
        } else {
            spes.push_back({token.word, true});
        }

        // Avoid duplicates
        for (auto dit = find_duplicate_spes(spes, previous_spes); dit != spes.cend();) {
            if (!((*dit).regular))
                dit = spes.erase(dit);
            else
                ++dit;
        }

        // Add to the result(s)
        for (const auto& spe : spes) {
            ret.push_back(spe.word);
        }
        previous_spes = spes;
    }

    return join_string(ret);
}

std::string verscheissern(const std::string& input, const ScheissFlags flags)
{
    return verscheissern(split_string(input));
}
