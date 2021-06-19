#ifndef OS_RISC_V_TRIE_H
#define OS_RISC_V_TRIE_H

#include "list.h"
#include "string.h"

/**
 * Trie
 * @tparam U search unit
 * @tparam T data stored in end node
 */
template<typename U, typename T>
class Trie {
private:
    Trie<U, T> *findChild(List<Trie<U, T>> &child_s, const U &c) {
        auto *cnt = child_s.start;
        while (cnt) {
            if (c == cnt->data.character) {
                return &cnt->data;
            }
            cnt = cnt->next;
        }
        return &cnt->data;
    }

public:

    U character{};
    T *data;
    bool isEnd;
    List<Trie<U, T>> children;

    explicit Trie(const U &c) : character(c), isEnd(false), data(nullptr) {}

    Trie() : isEnd(false), data(nullptr) {}


    void insert(const List<U> &word, const T &endData) {
        auto *node = this;
        auto *word_head = word.start;
        while (word_head) {
            auto &c = word_head->data;
            auto *target = findChild(node->children, c);
            if (target == nullptr) {
                node->children.push_back(Trie<U, T>(c));
                target = findChild(node->children, c);
            }
            node = target;
            word_head = word_head->next;
        }
        node->isEnd = true;
        node->data = new T(endData);
    }

    /**
     * return the first node.data which is marked End
     * ! it will pop prefix in word
     * @param word
     * @return if find, return node.data otherwise nullptr
     */
    T *firstMatch(List<U> &word) {
        auto *node = this;
        auto *word_head = word.start;
        int count = 0;
        while (word_head) {
            count++;
            auto &c = word_head->data;
            auto *target = findChild(node->children, c);
            if (target == nullptr) {
                return nullptr;
            }
            node = target;
            if (node->isEnd){
                while (count--){
                    word.pop_front();
                }
                return node->data;
            }
            word_head = word_head->next;
        }
        return nullptr;
    }

    /**
     * return the node totally suitable with word
     * @param word
     * @return if find, return node.data otherwise nullptr
     */
    T* fullMatch(const List<U> &word){
        auto *node = this;
        auto *word_head = word.start;
        while (word_head) {
            auto &c = word_head->data;
            auto *target = findChild(node->children, c);
            if (target == nullptr) {
                return nullptr;
            }
            node = target;
            word_head = word_head->next;
        }
        return node->data;
    }
};

class TestTrie {
public:
    TestTrie() {
//        printf("#=> Start Test Trie\n");
        Trie<String, int> strTrie;
        List<String> file1, file2, file3;
        file1.push_back("/");
        file1.push_back("dev");

        file2.push_back("dev");
        file2.push_back("/");
        file2.push_back("sdb");

        file3.push_back("/");
        file3.push_back("dev");
        file3.push_back("/");
        file3.push_back("sdb");

        strTrie.insert(file1, 1);
        strTrie.insert(file2, 2);
        strTrie.insert(file3, 3);

        printf("Trie.h 0\n");

        List<String> f1 = file1, f2 = file2, f3 = file3;
        printf("0.5\n");

        assert(*strTrie.firstMatch(f1) == 1);

        printf("1\n");

        assert(*strTrie.firstMatch(f2) == 2);
        printf("2\n");

        assert(*strTrie.firstMatch(f3) == 1);
        printf("3\n");
        f3 = file3;
        assert(*strTrie.fullMatch(f3) == 3);
        printf("4\n");
    }
};

#endif //OS_RISC_V_TRIE_H
