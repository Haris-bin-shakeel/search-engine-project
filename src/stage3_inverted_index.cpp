#include "stage3_inverted_index.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

void InvertedIndex::build(const ForwardIndex& fwd) {
    for (const auto& doc : fwd.get_documents()) {
        for (size_t i = 0; i < doc.term_ids.size(); ++i) {
            int tid = doc.term_ids[i];
            bool found = false;
            for (auto& p : index[tid]) {
                if (p.doc_id == doc.doc_id) {
                    found = true;
                    p.term_freq++;
                    p.positions.push_back(doc.positions[i]);
                    break;
                }
            }
            if (!found) index[tid].push_back({doc.doc_id, 1, {doc.positions[i]}});
        }
    }

    for (auto& [tid, plist] : index) {
        std::sort(plist.begin(), plist.end(), [](const Posting& a, const Posting& b) {
            return a.doc_id < b.doc_id;
        });
    }

    std::cout << "Inverted index built. Total terms: " << index.size() << "\n";
}

void InvertedIndex::save_segmented(const std::string& folder, int segment_size) {
    fs::create_directories(folder);

    int segment_id = 0;
    std::vector<int> tids;
    for (auto& [tid, _] : index) tids.push_back(tid);

    for (size_t i = 0; i < tids.size(); i += segment_size) {
        size_t end = std::min(i + segment_size, tids.size());
        std::ofstream out(folder + "/inverted_index_segment_" + std::to_string(segment_id++) + ".bin", std::ios::binary);
        for (size_t j = i; j < end; ++j) {
            int tid = tids[j];
            auto& plist = index[tid];
            size_t plist_size = plist.size();
            out.write(reinterpret_cast<const char*>(&tid), sizeof(tid));
            out.write(reinterpret_cast<const char*>(&plist_size), sizeof(plist_size));
            for (auto& p : plist) {
                size_t pos_size = p.positions.size();
                out.write(reinterpret_cast<const char*>(&p.doc_id), sizeof(p.doc_id));
                out.write(reinterpret_cast<const char*>(&p.term_freq), sizeof(p.term_freq));
                out.write(reinterpret_cast<const char*>(&pos_size), sizeof(pos_size));
                out.write(reinterpret_cast<const char*>(p.positions.data()), pos_size * sizeof(int));
            }
        }
        out.close();
        std::cout << "Segment " << segment_id-1 << " saved: " << end-i << " terms\n";
    }
}

void InvertedIndex::save_segmented_with_lexicon(const std::string& folder, Lexicon& lex, int segment_size) {
    fs::create_directories(folder);

    int segment_id = 0;
    std::vector<int> tids;
    for (auto& [tid, _] : index) tids.push_back(tid);

    size_t global_offset = 0;
    auto& lex_entries = lex.get_entries_mutable();

    for (size_t i = 0; i < tids.size(); i += segment_size) {
        size_t end = std::min(i + segment_size, tids.size());
        std::ofstream out(folder + "/inverted_index_segment_" + std::to_string(segment_id++) + ".bin", std::ios::binary);

        for (size_t j = i; j < end; ++j) {
            int tid = tids[j];
            auto& plist = index[tid];

            lex_entries[tid].posting_ptr = global_offset;

            size_t plist_size = plist.size();
            out.write(reinterpret_cast<const char*>(&tid), sizeof(tid));
            out.write(reinterpret_cast<const char*>(&plist_size), sizeof(plist_size));
            global_offset += sizeof(tid) + sizeof(plist_size);

            for (auto& p : plist) {
                size_t pos_size = p.positions.size();
                out.write(reinterpret_cast<const char*>(&p.doc_id), sizeof(p.doc_id));
                out.write(reinterpret_cast<const char*>(&p.term_freq), sizeof(p.term_freq));
                out.write(reinterpret_cast<const char*>(&pos_size), sizeof(pos_size));
                out.write(reinterpret_cast<const char*>(p.positions.data()), pos_size * sizeof(int));

                global_offset += sizeof(p.doc_id) + sizeof(p.term_freq) + sizeof(pos_size) + pos_size * sizeof(int);
            }
        }
        out.close();
        std::cout << "Segment " << segment_id-1 << " saved: " << end-i << " terms\n";
    }

    std::cout << "Inverted index saved with updated posting_ptr in lexicon.\n";
}
