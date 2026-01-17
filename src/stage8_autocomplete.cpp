#include "stage8_autocomplete.h"
#include <algorithm>
#include <cctype>
#include <sstream>

// Helper: lowercase
static std::string to_lower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

void Autocomplete::build_trie() {
    for (const auto& doc : documents) {
        std::istringstream iss(doc);
        std::string token;
        while (iss >> token) {
            token = to_lower(token);
            auto node = root;
            for (char c : token) {
                if (!node->children.count(c)) {
                    node->children[c] = std::make_shared<TrieNode>();
                }
                node = node->children[c];
            }
            node->is_word = true;
            node->word = token;
        }
    }
}

void Autocomplete::dfs(std::shared_ptr<TrieNode> node, std::vector<std::pair<std::string, int>>& result) {
    if (node->is_word) {
        int term_id = lexicon.get_term_id(node->word);
        int df = term_id >= 0 ? lexicon.get_df(term_id) : 0;
        result.push_back({node->word, df});
    }
    for (auto& p : node->children) { // classic iteration
        dfs(p.second, result);
    }
}

std::vector<std::string> Autocomplete::get_suggestions(const std::string& prefix, int max_suggestions) {
    auto node = root;
    std::string pre = to_lower(prefix);

    for (char c : pre) {
        if (!node->children.count(c)) return {};
        node = node->children[c];
    }

    std::vector<std::pair<std::string, int>> words;
    dfs(node, words);

    // Sort by DF descending
    std::sort(words.begin(), words.end(), [](const auto& a, const auto& b) {
        return b.second > a.second;
    });

    std::vector<std::string> suggestions;
    for (size_t i = 0; i < words.size() && i < (size_t)max_suggestions; ++i) {
        suggestions.push_back(words[i].first);
    }
    return suggestions;
}
