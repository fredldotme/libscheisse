#include "lib.h"

#include <algorithm>
#include <array>
#include <cctype>

static inline std::string transform_lowercase(const std::string& source)
{
    std::string ret;
    std::transform(source.cbegin(), source.cend(), ret.begin(), [=](unsigned char c){
        return static_cast<unsigned char>(std::tolower(c));
    });
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
        if (strict_search(source_term, article))
           return true;
    }
    return false;
}

static inline std::string article_verscheissern(const std::string& article)
{
    return article + std::string(" scheiß");
}

std::string verscheissern(const std::vector<std::string>& input, const ScheissFlags flags)
{
    std::string ret;

    for (auto it = input.begin(); it != input.end(); it++) {
        const std::string& word = *it;
        std::string spe;

        if ((flags & ScheissFlags::BeforeArticles) && article_search(word)) {
            spe = article_verscheissern(word);
        } else {
            spe = word;
        }

        ret += spe;
        if (word != *input.cend())
            ret += " ";
    }

    return ret;
}
