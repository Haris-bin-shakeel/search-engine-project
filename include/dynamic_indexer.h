#pragma once
#include "stage1_lexicon.h"
#include "stage2_forward_index.h"
#include "stage3_inverted_index.h"
#include "stage4_ranking.h"
#include <vector>
#include <string>

class DynamicIndexer {
public:
    DynamicIndexer(Lexicon& lex, ForwardIndex& fwd, InvertedIndex& inv, Stage4Ranking& rank)
        : lexicon(lex), fwd_index(fwd), inv_index(inv), ranking(rank) {}

    int add_document(const std::string& doc_text);

private:
    Lexicon& lexicon;
    ForwardIndex& fwd_index;
    InvertedIndex& inv_index;
    Stage4Ranking& ranking;
};
