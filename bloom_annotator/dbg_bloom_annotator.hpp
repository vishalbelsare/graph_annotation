#ifndef __DBG_BLOOM_ANNOTATOR_HPP__
#define __DBG_BLOOM_ANNOTATOR_HPP__

#include "hashers.hpp"

#include <map>
#include <unordered_map>


namespace hash_annotate {

class DeBruijnGraphWrapper {
  public:
    typedef uint64_t edge_index;

    virtual ~DeBruijnGraphWrapper() {}

    virtual size_t get_k() const = 0;

    virtual edge_index first_edge() const = 0;
    virtual edge_index last_edge() const = 0;
    virtual edge_index map_kmer(const std::string &kmer) const = 0;

    virtual size_t get_num_edges() const = 0;

    // Transform sequence to the same kind as the de bruijn graph stores
    virtual std::string encode_sequence(const std::string &sequence) const {
        return sequence;
    }

    virtual std::string transform_sequence(const std::string &sequence,
                                           bool rooted = false) const {
        return rooted ? sequence : sequence;
    }

    virtual std::string get_node_kmer(edge_index i) const = 0;
    virtual char get_edge_label(edge_index i) const = 0;

    // Check if the source k-mer for this edge has the only outgoing edge
    virtual bool has_the_only_outgoing_edge(edge_index i) const = 0;
    virtual bool has_the_only_incoming_edge(edge_index i) const = 0;

    virtual bool is_dummy_label(char edge_label) const = 0;
    virtual bool is_dummy_edge(const std::string &kmer) const = 0;

    virtual edge_index next_edge(edge_index i, char edge_label) const = 0;
    virtual edge_index prev_edge(edge_index i) const = 0;

};

class PreciseAnnotator {
  public:
    virtual ~PreciseAnnotator() {}
    virtual std::vector<uint64_t> annotate_edge(DeBruijnGraphWrapper::edge_index i,
                                                bool permute = false) const = 0;
};


class PreciseHashAnnotator : public PreciseAnnotator {
  public:
    PreciseHashAnnotator(const DeBruijnGraphWrapper &graph) : graph_(graph) {}

    void add_sequence(const std::string &sequence,
                      pos_t column = static_cast<pos_t>(-1),
                      bool rooted = false);

    void add_column(const std::string &sequence, bool rooted = false);

    std::vector<uint64_t> annotation_from_kmer(const std::string &kmer,
                                               bool permute = false) const;

    std::vector<uint64_t> annotate_edge(DeBruijnGraphWrapper::edge_index i,
                                        bool permute = false) const;

    std::set<pos_t> annotate_edge_indices(DeBruijnGraphWrapper::edge_index i,
                                           bool permute = false) const;

    std::string get_kmer(DeBruijnGraphWrapper::edge_index i) const;

    uint64_t serialize(std::ostream &out) const;
    uint64_t serialize(const std::string &filename) const;

    void load(std::istream &in);
    void load(const std::string &filename);

    void clear_prefix() { prefix_indices_.clear(); }

    std::set<pos_t> get_prefix() const { return prefix_indices_; }

    void make_column_prefix(pos_t i) { prefix_indices_.insert(i); }

    template <class Iterator>
    void make_columns_prefix(const Iterator &begin, const Iterator &end) {
        prefix_indices_.insert(begin, end);
    }

    uint64_t export_rows(std::ostream &out, bool permute = true) const;
    uint64_t export_rows(const std::string &filename, bool permute = true) const;

    size_t num_columns() const { return annotation_exact.size(); }

    size_t num_prefix_columns() const { return prefix_indices_.size(); }

    size_t size() const { return annotation_exact.get_num_edges(); }

    std::unordered_map<size_t, size_t> compute_permutation_map() const;

    static std::unordered_map<size_t, size_t>
    compute_permutation_map(size_t num_columns,
                            const std::set<pos_t> &prefix_indices);

    std::vector<uint64_t> permute_indices(const std::vector<uint64_t> &a,
                                          const std::unordered_map<size_t, size_t> &index_map) const;

    bool operator==(const PreciseHashAnnotator &that) const {
        return (annotation_exact == that.annotation_exact)
            && std::equal(prefix_indices_.begin(), prefix_indices_.end(),
                          that.prefix_indices_.begin(), that.prefix_indices_.end())
            && (&graph_ == &that.graph_);
    }

  protected:
    const DeBruijnGraphWrapper &graph_;

  private:
    ExactHashAnnotation annotation_exact;
    std::set<pos_t> prefix_indices_;
};

class BloomAnnotator {
  public:
    // Computes optimal `bloom_size_factor` and `num_hash_functions` automatically
    BloomAnnotator(const DeBruijnGraphWrapper &graph,
                   double bloom_fpp,
                   bool verbose = false);

    // If not provided, computes optimal `num_hash_functions` automatically
    BloomAnnotator(const DeBruijnGraphWrapper &graph,
                   double bloom_size_factor,
                   size_t num_hash_functions,
                   bool verbose = false);

    void add_sequence(const std::string &sequence, size_t column, size_t num_elements = 0);

    void add_column(const std::string &sequence, size_t num_elements = 0);

    std::vector<uint64_t> get_annotation(DeBruijnGraphWrapper::edge_index i) const;

    std::vector<uint64_t> get_annotation_corrected(DeBruijnGraphWrapper::edge_index i,
                                                   bool check_both_directions = false,
                                                   size_t path_cutoff = 50) const;

    std::vector<uint64_t> annotation_from_kmer(const std::string &kmer) const;

    void test_fp_all(const PreciseAnnotator &annotation_exact,
                     size_t num = 0,
                     bool check_both_directions = false) const;

    uint64_t serialize(std::ostream &out) const;
    uint64_t serialize(const std::string &filename) const;

    bool load(std::istream &in);
    bool load(const std::string &filename);

    static std::vector<size_t> unpack(const std::vector<uint64_t> &packed);

    size_t num_hash_functions() const;

    double size_factor() const;

    double approx_false_positive_rate() const;

    size_t get_size(size_t i) const;

    size_t num_columns() const { return annotation.size(); }

  private:
    std::string kmer_from_index(DeBruijnGraphWrapper::edge_index index) const;

    std::vector<uint64_t> test_fp(DeBruijnGraphWrapper::edge_index i,
                                const PreciseAnnotator &annotation_exact,
                                bool check_both_directions = false) const;

    const DeBruijnGraphWrapper &graph_;
    double bloom_size_factor_;
    double bloom_fpp_;
    BloomHashAnnotation annotation;

    //TODO: get rid of this if not using degree Bloom filter
    std::vector<size_t> sizes_v;

    size_t total_traversed_;
    bool verbose_;
};

} // namespace hash_annotate

#endif // __DBG_BLOOM_ANNOTATOR_HPP__
