#include "gtest/gtest.h"
#include "wavelet_trie.hpp"


std::vector<std::vector<std::vector<size_t>>> bits {
    {}, // 0
    {   // 1
        {4},
        {2},
        {4},
        {2},
        {4},
        {2},
        {2}
    },
    {   // 2
        {3},
        {2},
        {2, 3}
    },
    {   // 3
        {3},
        {2},
        {2, 3},
        {3, 4, 5}
    },
    {   // 4
        {1},
        {1, 3}
    },
    {   // 5
        {1},
        {1, 2}
    },
    {   // 6
        {1},
        {1, 3, 4}
    },
    {   // 7
        {1, 4},
        {1, 3, 4}
    },
    {   // 8
        {1},
        {1, 7},
        {1, 5, 7},
        {1, 3, 7}
    },
    {   // 9
        {0},
        {1},
        {1}
    },
    {   // 10
        {1},
        {5}
    },
    {   // 11
        {1},
        {1, 5}
    },
    {   // 12
        {0},
        {1},
        {5}
    },
    {   // 13
        {0},
        {1},
        {5},
        {1},
        {5},
        {1},
        {5}
    },
    {   // 14
        {1},
        {3},
        {1, 3},
        {1}
    },
    {   // 15
        {1, 3}
    },
    {   // 16
        {1},
        {1, 3},
        {1, 3, 4},
        {1},
        {1, 3},
        {1, 3}
    },
    {   // 17
        {3, 4},
        {3, 4, 6},
        {3, 4, 6, 7, 8, 9}
    },
    {   // 18
        {3, 4, 6, 7, 8, 9, 10},
        {3, 4, 6, 7, 9}
    },
    {   // 19
        {1},
        {3},
        {1},
        {3},
        {1},
        {3},
        {1, 3}
    },
    {   // 20
        {1, 3, 5},
        {1, 3, 4, 6},
        {1, 3, 4},
        {1, 2, 3, 5},
        {1, 2, 3, 4, 6},
        {1, 2, 3, 4}
    },
    {   // 21
        {1, 3, 5},
        {1, 3, 4, 6},
        {1, 3, 4},
        {0, 1, 2, 3, 5},
        {0, 1, 2, 3, 4, 6},
        {1, 2, 3, 4, 4}
    }
};


std::vector<annotate::cpp_int> generate_nums(std::vector<std::vector<size_t>> &bits) {
    std::vector<annotate::cpp_int> nums;
    nums.reserve(bits.size());
    std::transform(bits.begin(), bits.end(), std::back_inserter(nums), annotate::pack_indices);
    assert(nums.size() == bits.size());
    return nums;
}

std::vector<std::vector<size_t>> generate_indices(std::vector<std::vector<size_t>> &bits, size_t seed = 42) {
    std::srand(seed);
    std::vector<std::vector<size_t>> nums(bits.size());
    for (size_t i = 0; i < nums.size(); ++i) {
        nums[i].insert(nums[i].end(), bits[i].begin(), bits[i].end());
        std::random_shuffle(nums[i].begin(), nums[i].end());
    }
    return nums;
}

std::vector<std::set<size_t>> generate_indices_set(std::vector<std::vector<size_t>> &bits, size_t seed = 42) {
    std::srand(seed);
    std::vector<std::set<size_t>> nums(bits.size());
    for (size_t i = 0; i < nums.size(); ++i) {
        nums[i].insert(bits[i].begin(), bits[i].end());
    }
    return nums;
}

void check_wtr(annotate::WaveletTrie &wt,
               const std::vector<annotate::cpp_int> &nums,
               const std::string &message = std::string()) {
    ASSERT_EQ(nums.size(), wt.size()) << message << ": size fail" << std::endl;
    for (size_t i = 0; i < nums.size(); ++i) {
        ASSERT_EQ(nums.at(i), wt.at(i)) << message << ":" << std::to_string(i) << std::endl;
    }
}

// Note: the vector needs to be a copy, since WaveletTrie modifies it
template <class Container>
void generate_wtr(std::vector<annotate::WaveletTrie> &wtrs, Container nums, size_t p, size_t step = 0) {
    if (nums.empty()) {
        wtrs.emplace_back(p);
        return;
    }
    auto it = nums.begin();
    while (step && it + step <= nums.end()) {
        wtrs.emplace_back(it, it + step, p);
        it += step;
    }
    if (it != nums.end())
        wtrs.emplace_back(it, nums.end(), p);
}

annotate::WaveletTrie merge_wtrs(std::vector<annotate::WaveletTrie> &wtrs, size_t p) {
    annotate::WaveletTrie wts(p);
    for (auto &wtr : wtrs) {
        wts.insert(std::move(wtr));
    }
    return wts;
}

annotate::WaveletTrie merge_wtrs_copy(std::vector<annotate::WaveletTrie> &wtrs, size_t p) {
    annotate::WaveletTrie wts(p);
    for (const auto &wtr : wtrs) {
        wts.insert(wtr);
    }
    return wts;
}

annotate::WaveletTrie test_wtr(size_t i, size_t p, int mode = 0) {
    std::vector<annotate::WaveletTrie> wtrs;
    auto nums = generate_nums(bits[i]);
    if (mode == 0) {
        generate_wtr(wtrs, nums, p);
    } else if (mode == 1) {
        auto nums = generate_indices(bits[i]);
        generate_wtr(wtrs, nums, p);
    } else if (mode == 2) {
        auto nums = generate_indices_set(bits[i]);
        generate_wtr(wtrs, nums, p);
    } else {
        generate_wtr(wtrs, bits[i], p);
    }
    auto wts = merge_wtrs(wtrs, p);
    check_wtr(wts, nums, std::to_string(i));
    return wts;
}
annotate::WaveletTrie test_wtr_step(size_t i, size_t p, int mode = 0) {
    std::vector<annotate::WaveletTrie> wtrs;
    auto nums = generate_nums(bits[i]);
    constexpr size_t step = 2;

    if (mode == 0) {
        generate_wtr(wtrs, nums, p, step);
    } else if (mode == 1) {
        auto nums = generate_indices(bits[i]);
        generate_wtr(wtrs, nums, p, step);
    } else if (mode == 2) {
        auto nums = generate_indices_set(bits[i]);
        generate_wtr(wtrs, nums, p, step);
    } else {
        generate_wtr(wtrs, bits[i], p, step);
    }

    auto wts = merge_wtrs(wtrs, p);
    check_wtr(wts, nums);
    return wts;
}

annotate::WaveletTrie test_wtr_copy(size_t i, size_t p, int mode = 0) {
    std::vector<annotate::WaveletTrie> wtrs;
    auto nums = generate_nums(bits[i]);

    if (mode == 0) {
        generate_wtr(wtrs, nums, p);
    } else if (mode == 1) {
        auto nums = generate_indices(bits[i]);
        generate_wtr(wtrs, nums, p);
    } else if (mode == 2) {
        auto nums = generate_indices_set(bits[i]);
        generate_wtr(wtrs, nums, p);
    } else {
        generate_wtr(wtrs, bits[i], p);
    }

    auto wts = merge_wtrs_copy(wtrs, p);
    check_wtr(wts, nums, std::to_string(i));
    return wts;
}
annotate::WaveletTrie test_wtr_copy_step(size_t i, size_t p, int mode = 0) {
    std::vector<annotate::WaveletTrie> wtrs;
    auto nums = generate_nums(bits[i]);
    constexpr size_t step = 2;

    if (mode == 0) {
        generate_wtr(wtrs, nums, p, step);
    } else if (mode == 1) {
        auto nums = generate_indices(bits[i]);
        generate_wtr(wtrs, nums, p, step);
    } else if (mode == 2) {
        auto nums = generate_indices_set(bits[i]);
        generate_wtr(wtrs, nums, p, step);
    } else {
        generate_wtr(wtrs, bits[i], p, step);
    }
    auto wts = merge_wtrs_copy(wtrs, p);
    check_wtr(wts, nums);
    return wts;
}

annotate::WaveletTrie test_wtr_pairs(size_t i, size_t j, size_t p, int mode = 0) {
    auto nums1 = generate_nums(bits[i]);
    auto nums2 = generate_nums(bits[j]);
    std::vector<annotate::WaveletTrie> wtrs;
    std::vector<annotate::cpp_int> ref;
    ref.reserve(nums1.size() + nums2.size());
    ref.insert(ref.end(), nums1.begin(), nums1.end());
    ref.insert(ref.end(), nums2.begin(), nums2.end());

    if (mode == 0) {
        generate_wtr(wtrs, nums1, p);
        generate_wtr(wtrs, nums2, p);
    } else if (mode == 1) {
        auto nums1 = generate_indices(bits[i]);
        auto nums2 = generate_indices(bits[j]);
        generate_wtr(wtrs, nums1, p);
        generate_wtr(wtrs, nums2, p);
    } else if (mode == 2) {
        auto nums1 = generate_indices_set(bits[i]);
        auto nums2 = generate_indices_set(bits[j]);
        generate_wtr(wtrs, nums1, p);
        generate_wtr(wtrs, nums2, p);
    } else {
        generate_wtr(wtrs, bits[i], p);
        generate_wtr(wtrs, bits[j], p);
    }
    auto wts = merge_wtrs(wtrs, p);
    check_wtr(wts, ref, std::to_string(i) + "," + std::to_string(j));
    return wts;
}
annotate::WaveletTrie test_wtr_pairs_step(size_t i, size_t j, size_t p, int mode = 0) {
    auto nums1 = generate_nums(bits[i]);
    auto nums2 = generate_nums(bits[j]);
    constexpr size_t step = 2;
    std::vector<annotate::WaveletTrie> wtrs;
    std::vector<annotate::cpp_int> ref;
    ref.reserve(nums1.size() + nums2.size());
    ref.insert(ref.end(), nums1.begin(), nums1.end());
    ref.insert(ref.end(), nums2.begin(), nums2.end());

    if (mode == 0) {
        generate_wtr(wtrs, nums1, p, step);
        generate_wtr(wtrs, nums2, p, step);
    } else if (mode == 1) {
        auto nums1 = generate_indices(bits[i]);
        auto nums2 = generate_indices(bits[j]);
        generate_wtr(wtrs, nums1, p, step);
        generate_wtr(wtrs, nums2, p, step);
    } else if (mode == 2) {
        auto nums1 = generate_indices_set(bits[i]);
        auto nums2 = generate_indices_set(bits[j]);
        generate_wtr(wtrs, nums1, p, step);
        generate_wtr(wtrs, nums2, p, step);
    } else {
        generate_wtr(wtrs, bits[i], p, step);
        generate_wtr(wtrs, bits[j], p, step);
    }
    auto wts = merge_wtrs(wtrs, p);
    check_wtr(wts, ref, std::to_string(i) + "," + std::to_string(j) + "," + std::to_string(step));
    return wts;
}

annotate::WaveletTrie test_wtr_pairs_copy(size_t i, size_t j, size_t p, int mode = 0) {
    auto nums1 = generate_nums(bits[i]);
    auto nums2 = generate_nums(bits[j]);
    std::vector<annotate::WaveletTrie> wtrs;
    std::vector<annotate::cpp_int> ref;
    ref.reserve(nums1.size() + nums2.size());
    ref.insert(ref.end(), nums1.begin(), nums1.end());
    ref.insert(ref.end(), nums2.begin(), nums2.end());

    if (mode == 0) {
        generate_wtr(wtrs, nums1, p);
        generate_wtr(wtrs, nums2, p);
    } else if (mode == 1) {
        auto nums1 = generate_indices(bits[i]);
        auto nums2 = generate_indices(bits[j]);
        generate_wtr(wtrs, nums1, p);
        generate_wtr(wtrs, nums2, p);
    } else if (mode == 2) {
        auto nums1 = generate_indices_set(bits[i]);
        auto nums2 = generate_indices_set(bits[j]);
        generate_wtr(wtrs, nums1, p);
        generate_wtr(wtrs, nums2, p);
    } else {
        generate_wtr(wtrs, bits[i], p);
        generate_wtr(wtrs, bits[j], p);
    }
    auto wts = merge_wtrs_copy(wtrs, p);
    check_wtr(wts, ref, std::to_string(i) + "," + std::to_string(j));
    return wts;
}

annotate::WaveletTrie test_wtr_pairs_copy_step(size_t i, size_t j, size_t p, int mode = 0) {
    auto nums1 = generate_nums(bits[i]);
    auto nums2 = generate_nums(bits[j]);
    constexpr size_t step = 2;
    std::vector<annotate::WaveletTrie> wtrs;
    std::vector<annotate::cpp_int> ref;
    ref.reserve(nums1.size() + nums2.size());
    ref.insert(ref.end(), nums1.begin(), nums1.end());
    ref.insert(ref.end(), nums2.begin(), nums2.end());

    if (mode == 0) {
        generate_wtr(wtrs, nums1, p, step);
        generate_wtr(wtrs, nums2, p, step);
    } else if (mode == 1) {
        auto nums1 = generate_indices(bits[i]);
        auto nums2 = generate_indices(bits[j]);
        generate_wtr(wtrs, nums1, p, step);
        generate_wtr(wtrs, nums2, p, step);
    } else if (mode == 2) {
        auto nums1 = generate_indices_set(bits[i]);
        auto nums2 = generate_indices_set(bits[j]);
        generate_wtr(wtrs, nums1, p, step);
        generate_wtr(wtrs, nums2, p, step);
    } else { 
        generate_wtr(wtrs, bits[i], p, step);
        generate_wtr(wtrs, bits[j], p, step);
    }
    auto wts = merge_wtrs_copy(wtrs, p);
    check_wtr(wts, ref, std::to_string(i) + "," + std::to_string(j) + "," + std::to_string(step));
    return wts;
}

annotate::WaveletTrie test_wtr_pairs_insert(size_t i, size_t j, size_t p, size_t k, int mode = 0) {
    std::ignore = mode;
    auto nums1 = generate_nums(bits[i]);
    auto nums2 = generate_nums(bits[j]);
    std::vector<annotate::cpp_int> ref;
    ref.reserve(nums1.size() + nums2.size());
    ref.insert(ref.end(), nums1.begin(), nums1.begin() + k);
    ref.insert(ref.end(), nums2.begin(), nums2.end());
    ref.insert(ref.end(), nums1.begin() + k, nums1.end());

    std::vector<annotate::WaveletTrie> wtrs;
    generate_wtr(wtrs, nums1, p);
    generate_wtr(wtrs, nums2, p);

    EXPECT_EQ(2u, wtrs.size());
    assert(wtrs.size() == 2);

    wtrs[0].insert(std::move(wtrs[1]), k);
    check_wtr(wtrs[0], ref, std::to_string(i) + "," + std::to_string(j) + "," + std::to_string(k));
    return std::move(wtrs[0]);
}

annotate::WaveletTrie test_wtr_pairs_insert_copy(size_t i, size_t j, size_t p, size_t k, int mode = 0) {
    std::ignore = mode;
    auto nums1 = generate_nums(bits[i]);
    auto nums2 = generate_nums(bits[j]);
    std::vector<annotate::cpp_int> ref;
    ref.reserve(nums1.size() + nums2.size());
    ref.insert(ref.end(), nums1.begin(), nums1.begin() + k);
    ref.insert(ref.end(), nums2.begin(), nums2.end());
    ref.insert(ref.end(), nums1.begin() + k, nums1.end());

    std::vector<annotate::WaveletTrie> wtrs;
    generate_wtr(wtrs, nums1, p);
    generate_wtr(wtrs, nums2, p);

    EXPECT_EQ(2u, wtrs.size());
    assert(wtrs.size() == 2);

    wtrs[0].insert(wtrs[1], k);
    check_wtr(wtrs[0], ref, std::to_string(i) + "," + std::to_string(j) + "," + std::to_string(k));
    return std::move(wtrs[0]);
}

void check_wtr_vector(std::vector<annotate::WaveletTrie> &wtrs) {
    if (wtrs.size()) {
        for (auto it = wtrs.begin(); it + 1 != wtrs.end(); ++it) {
            EXPECT_TRUE(*it == *(it + 1))
                << "EQUAL FAIL: " << std::distance(wtrs.begin(), it);
            if (*it != *(it + 1)) {
                it->print();
                (it+1)->print();
                assert(false);
            }
        }
    }
}

std::vector<size_t> num_threads = {1, 4};
std::vector<size_t> modes = {0, 1, 2, 3};

TEST(WaveletTrie, TestSingle) {
    std::vector<annotate::WaveletTrie> wtrs;
    for (size_t i = 0; i < bits.size(); ++i) {
        wtrs.clear();
        for (auto p : num_threads) {
            for (auto m : modes) {
                wtrs.push_back(test_wtr(i, p, m));
                wtrs.push_back(test_wtr_step(i, p, m));
                wtrs.push_back(test_wtr_copy(i, p, m));
                wtrs.push_back(test_wtr_copy_step(i, p, m));
            }
        }
        check_wtr_vector(wtrs);
    }
}

TEST(WaveletTrie, TestPairs) {
    std::vector<annotate::WaveletTrie> wtrs;
    for (size_t i = 0; i < bits.size(); ++i) {
        for (size_t j = 0; j < bits.size(); ++j) {
            wtrs.clear();
            for (auto p : num_threads) {
                for (auto m : modes) {
                    wtrs.push_back(test_wtr_pairs(i, j, p, m));
                    wtrs.push_back(test_wtr_pairs_step(i, j, p, m));
                    wtrs.push_back(test_wtr_pairs_copy(i, j, p, m));
                    wtrs.push_back(test_wtr_pairs_copy_step(i, j, p, m));
                }
            }
            check_wtr_vector(wtrs);
        }
    }
}


