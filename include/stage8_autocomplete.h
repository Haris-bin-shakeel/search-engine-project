#pragma once
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <sstream>
#include "stage1_lexicon.h"

// Trie node
struct TrieNode {
    std::unordered_map<char, std::shared_ptr<TrieNode>> children;
    bool is_word = false;
    std::string word;
};

class Autocomplete {
public:
    // Constructor
    Autocomplete(const std::vector<std::string>& docs, const Lexicon& lex)
        : documents(docs), lexicon(lex) { }

    void build_trie();
    
    // Added for Stage 9 compatibility: Rebuild trie from Lexicon (not documents)
    // Industry standard: Autocomplete should reflect lexicon, not static corpus
    void rebuild_from_lexicon();
    
    std::vector<std::string> get_suggestions(const std::string& prefix, int max_suggestions = 5);

private:
    const Lexicon& lexicon;
    const std::vector<std::string>& documents;
    std::shared_ptr<TrieNode> root = std::make_shared<TrieNode>();

    void dfs(std::shared_ptr<TrieNode> node, std::vector<std::pair<std::string, int>>& result);
};