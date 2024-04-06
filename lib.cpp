#include "lib.h"

#include <algorithm>
#include <array>

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
    return source_term.find(search_term) == 0 && source_term.length() == search_term.length();
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

static inline bool article_search(const std::string& term)
{
    static const std::array<std::string, 3> articles = {"der", "die", "das"};
    for (const auto& article : articles) {
        if (insensitive_search(term, article))
            return true;
    }
    return false;
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

static inline bool nomen_check(const std::string& word)
{
    return word.length() >= 2 && word[0] == std::toupper(word[0]) && word[1] == std::tolower(word[1]);
}

static inline std::string article_verscheissern(const std::string& article)
{
    return article + std::string(" scheiß");
}

static inline std::string nomen_verscheissern(const std::string& nomen)
{
    return std::string("scheiß ") + nomen;
}

static inline std::string strip_punctuation(const std::string& word, TokenAnalysis& followup_token)
{
    static const unsigned char question_mark = '?';
    static const unsigned char exclamation_mark = '!';
    static const unsigned char period = '.';
    static const std::array<unsigned char, 6> punctuations = {',', ':', ';', period, question_mark, exclamation_mark };

    // Optimize short hand writing without stripping periods
    if (std::count(word.cbegin(), word.cend(), period) > 1)
        return word;

    std::string ret;
    for (const auto& c : word) {
        if (std::find(punctuations.begin(), punctuations.end(), c) != std::end(punctuations)) {
            if (c == period || c == exclamation_mark || c == question_mark) {
                const std::string punctuation(1, c);
                followup_token.word = punctuation;
                followup_token.token_type = SentenceEnd;
            }
            continue;
        }
        ret += c;
    }
    return ret;
}

std::vector<TokenAnalysis> analyse(const std::vector<std::string>& input)
{
    std::vector<TokenAnalysis> ret;

    for (auto it = input.cbegin(); it != input.cend(); it++) {
        const auto* previous_token = ret.size() > 0 ? &ret[ret.size() - 1] : nullptr;

        TokenAnalysis followup_token;
        const std::string word = strip_punctuation(*it, followup_token);

        Type type = Type_Unknown;
        TokenType token_type = (it == input.cbegin()) ? SentenceBeginning : TokenType_Unknown;
        if (previous_token  && (*previous_token).token_type == SentenceEnd)
            token_type = SentenceBeginning;

        if (article_search(word))
            type = Artikel;
        else if (adverb_search(word))
            type = Adverb;
        else if (subjunction_search(word))
            type = Subjunktion;
        // Check whether its a Nomen later to avoid beginnings of sentences to be misdetected.
        else if (nomen_check(word))
            type = Nomen;

        ret.push_back(TokenAnalysis{word, token_type, type});
        if (followup_token.token_type != TokenType_Unknown) {
            ret.push_back(followup_token);
            followup_token = TokenAnalysis();
        }
    }

    return ret;
}

std::vector<TokenAnalysis> analyse(const std::string& input)
{
    return analyse(split_string(input));
}

std::string verscheissern(const std::vector<std::string>& input, const ScheissFlags flags)
{
    std::string ret;
    const auto analysis = analyse(input);

    for (auto it = analysis.cbegin(); it != analysis.cend(); it++) {
        const auto& token = (*it);
        const auto* look_backward_token = it != analysis.cbegin() ? &(*(it-1)) : nullptr;
        const auto* peek_forward_token = &(*(it+1));

        std::string spe;

        if ((flags & ScheissFlags::BeforeArticles) && token.type == Artikel &&
            peek_forward_token && (*peek_forward_token).type == Nomen) {
            spe = article_verscheissern(token.word);
        } else if ((flags & ScheissFlags::BeforeNomen) && token.type == Nomen &&
                   /*TODO: Remove token_type check once smarter*/
                   token.token_type != SentenceBeginning) {
            spe = nomen_verscheissern(token.word);
        } else {
            spe = token.word;
        }

        // Capitalize beginning of a sentence
        if (!token.word.empty() && token.token_type == SentenceBeginning)
            spe[0] = std::toupper(spe[0]);

        ret += spe;
        if (token.word != *input.cend())
            ret += " ";
    }

    return ret;
}

std::string verscheissern(const std::string& input, const ScheissFlags flags)
{
    return verscheissern(split_string(input));
}
