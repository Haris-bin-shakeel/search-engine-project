#include "stage1_lexicon.h"
#include <filesystem>

namespace fs = std::filesystem;

void Lexicon::build(const std::string& dataset_path) {
    std::ifstream infile(dataset_path);
    if (!infile.is_open()) throw std::runtime_error("Cannot open dataset");

    std::unordered_map<std::string,int> df_map;
    std::string line;
    while (getline(infile, line)) {
        std::unordered_set<std::string> tokens;
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) tokens.insert(token);
        for (const auto& t : tokens) df_map[t]++;
    }
    infile.close();

    int term_id = 0;
    entries.reserve(df_map.size());
    for (auto& [token, df] : df_map) entries.push_back({term_id++, token, df, 0});
    for (auto& e : entries) token_to_id[e.term] = e.term_id;
    std::sort(entries.begin(), entries.end(), [](auto& a, auto& b){ return a.term < b.term; });

    std::cout << "Lexicon built. Total tokens: " << entries.size() << "\n";
}

void Lexicon::save_segmented(const std::string& folder) {
    fs::create_directories(folder);

    // Binary file
    std::ofstream bin(folder + "/lexicon.bin", std::ios::binary);
    for (const auto& e : entries) {
        size_t len = e.term.size();
        bin.write(reinterpret_cast<const char*>(&e.term_id), sizeof(e.term_id));
        bin.write(reinterpret_cast<const char*>(&len), sizeof(len));
        bin.write(e.term.c_str(), len);
        bin.write(reinterpret_cast<const char*>(&e.df), sizeof(e.df));
        bin.write(reinterpret_cast<const char*>(&e.posting_ptr), sizeof(e.posting_ptr));
    }
    bin.close();

    // JSON file
    std::ofstream json(folder + "/lexicon.json");
    json << "[\n";
    for (size_t i = 0; i < entries.size(); ++i) {
        const auto& e = entries[i];
        json << "  {\"term_id\":" << e.term_id << ",\"term\":\"" << e.term
             << "\",\"df\":" << e.df << ",\"posting_ptr\":" << e.posting_ptr << "}";
        if (i != entries.size()-1) json << ",";
        json << "\n";
    }
    json << "]";
    json.close();

    std::cout << "Lexicon saved.\n";
}
