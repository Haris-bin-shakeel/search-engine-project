#include "stage2_forward_index.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

void ForwardIndex::build(const std::string& dataset_path, const Lexicon& lex) {
    std::ifstream infile(dataset_path);
    if (!infile.is_open()) throw std::runtime_error("Cannot open dataset");

    std::string line;
    int doc_id = 0;
    while (getline(infile, line)) {
        std::istringstream iss(line);
        std::string token;
        ForwardDoc doc;
        doc.doc_id = doc_id++;
        std::unordered_map<int,int> freq_map;
        int pos = 0;
        while (iss >> token) {
            int tid = lex.get_term_id(token);
            if (tid != -1) {
                doc.term_ids.push_back(tid);
                doc.positions.push_back(pos++);
                freq_map[tid]++;
            }
        }
        for (auto t : doc.term_ids) doc.term_freqs.push_back(freq_map[t]);
        doc.length = doc.term_ids.size();
        documents.push_back(doc);
    }
    infile.close();
    std::cout << "Forward index built. Total docs: " << documents.size() << "\n";
}

void ForwardIndex::save_segmented(const std::string& folder, int segment_size) {
    fs::create_directories(folder);

    int segment_id = 0;
    for (size_t i = 0; i < documents.size(); i += segment_size) {
        size_t end = std::min(i + segment_size, documents.size());
        std::ofstream out(folder + "/forward_index_" + std::to_string(segment_id++) + ".bin", std::ios::binary);
        for (size_t j = i; j < end; ++j) {
            auto& doc = documents[j];
            size_t term_count = doc.term_ids.size();
            out.write(reinterpret_cast<const char*>(&doc.doc_id), sizeof(doc.doc_id));
            out.write(reinterpret_cast<const char*>(&term_count), sizeof(term_count));
            out.write(reinterpret_cast<const char*>(doc.term_ids.data()), term_count*sizeof(int));
            out.write(reinterpret_cast<const char*>(doc.term_freqs.data()), term_count*sizeof(int));
            out.write(reinterpret_cast<const char*>(doc.positions.data()), term_count*sizeof(int));
            out.write(reinterpret_cast<const char*>(&doc.length), sizeof(doc.length));
        }
        out.close();
        std::cout << "Segment " << segment_id-1 << " saved: " << end-i << " docs\n";
    }
}
