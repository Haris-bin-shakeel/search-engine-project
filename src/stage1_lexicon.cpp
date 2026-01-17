#include "stage1_lexicon.h"
#include <sstream>
#include <algorithm>

void Lexicon::build_from_docs(const std::vector<std::string>& docs) {
    for (const auto& doc : docs) {
        std::istringstream iss(doc);
        std::string token;
        while (iss >> token) {
            // Convert to lowercase
            std::transform(token.begin(), token.end(), token.begin(), ::tolower);

            // Add token if not exists
            if (token_to_id.find(token) == token_to_id.end()) {
                token_to_id[token] = next_id;
                id_to_token[next_id] = token; // reverse mapping
                ++next_id;
            }

            // Increase DF count
            int term_id = token_to_id[token];
            df_map[term_id]++;
        }
    }
}
