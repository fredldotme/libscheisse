#include "lib.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>
#include <iostream>

static inline std::vector<std::string> split_string(const std::string& to_split)
{
    std::istringstream split_stream(to_split);
    std::vector<std::string> ret;
    std::string tmp;
    while (std::getline(split_stream, tmp, ' ')) {
        ret.push_back(tmp);
    }
    return ret;
}

static inline std::string transform_lowercase(const std::string& source)
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
    const std::string source = transform_lowercase(source_term);
    const std::string test = transform_lowercase(search_term);
    return source.find(test) == 0 && source.length() == test.length();
}

static inline bool fuzzy_search(const std::string& source_term, const std::string& search_term)
{
    const std::string source = transform_lowercase(source_term);
    const std::string test = transform_lowercase(search_term);
    return source.find(test) == std::string::npos && source.length() < test.length();
}

static inline bool article_search(const std::string& source_term)
{
    static const std::array<std::string, 3> articles = {"der", "die", "das"};
    for (const auto& article : articles) {
        if (insensitive_search(source_term, article))
           return true;
    }
    return false;
}

static inline std::string article_verscheissern(const std::string& article)
{
    return article + std::string(" scheiß");
}

std::vector<TokenAnalysis> analyse(const std::vector<std::string>& input)
{
    std::vector<TokenAnalysis> ret;

    for (auto it = input.cbegin(); it != input.cend(); it++) {
        const std::string& word = *it;
        Type type = Type_Unknown;

        if (article_search(word))
            type = Artikel;

        ret.push_back(TokenAnalysis{word, type});
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
        std::string spe;

        if ((flags & ScheissFlags::BeforeArticles) && token.type == Artikel) {
            spe = article_verscheissern(token.word);
        } else {
            spe = token.word;
        }

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