// DSA2-P021 : Plagiarism Pattern Detector (Education)

// String-matching algorithm : KMP (Knuth-Morris-Pratt)
// Comparison algorithm      : Naive (brute-force) search

// Declared rules / assumptions:
//   - Matching is exact and CASE-SENSITIVE (no normalization).
//   - Indices are 0-based character positions (newline counts as one character).
//   - Empty patterns are rejected.
//   - Limits: pattern length <= 100, at most 1000 patterns, document length <= 5000 chars.

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

const size_t MAX_PATTERN_LEN = 100;
const size_t MAX_PATTERNS = 1000;
const size_t MAX_DOC_LEN = 5000;

// statistics
struct Stats {
    long long comparisons = 0;  // character comparisons
};

typedef vector<size_t> (*SearchFn)(const string&, const string&, Stats&);

// Naive search
vector<size_t> naiveSearch(const string& t, const string& p, Stats& st) {
    vector<size_t> found;
    size_t n = t.size(), m = p.size();
    if (m == 0 || m > n) return found;
    for (size_t i = 0; i + m <= n; i++) {
        size_t j = 0;
        while (j < m) {
            st.comparisons++;
            if (t[i + j] != p[j]) break;
            j++;
        }
        if (j == m) found.push_back(i);
    }
    return found;
}

// KMP search
vector<size_t> kmpSearch(const string& t, const string& p, Stats& st) {
    vector<size_t> found;
    size_t n = t.size(), m = p.size();
    if (m == 0 || m > n) return found;

    
    vector<size_t> lps(m, 0);
    for (size_t i = 1, len = 0; i < m;) {
        st.comparisons++;
        if (p[i] == p[len]) lps[i++] = ++len;
        else if (len > 0) len = lps[len - 1];
        else lps[i++] = 0;
    }

    // Scan the text
    for (size_t i = 0, j = 0; i < n;) {
        st.comparisons++;
        if (t[i] == p[j]) {
            i++; j++;
            if (j == m) { found.push_back(i - m); j = lps[j - 1]; }
        } else if (j > 0) {
            j = lps[j - 1];
        } else {
            i++;
        }
    }
    return found;
}

void printComparison(const Stats& kmp, const Stats& naive) {
    cout << "    [Algorithm statistics]\n"
         << "      KMP   : " << kmp.comparisons << " character comparisons\n"
         << "      Naive : " << naive.comparisons << " character comparisons\n";
    if (kmp.comparisons < naive.comparisons)
        cout << "      KMP used " << naive.comparisons - kmp.comparisons << " fewer comparisons.\n";
    else if (kmp.comparisons > naive.comparisons)
        cout << "      Naive used " << kmp.comparisons - naive.comparisons << " fewer comparisons.\n";
    else
        cout << "      Both used the same number of comparisons.\n";
}

// highlighting
void printHighlighted(const string& text, const vector<pair<size_t, size_t>>& ranges) {
    vector<char> mark(text.size() + 1, 0);
    for (auto& r : ranges)
        for (size_t i = r.first; i < r.first + r.second && i < text.size(); i++) mark[i] = 1;
    cout << "    ";
    bool open = false;
    for (size_t i = 0; i < text.size(); i++) {
        if (mark[i] && !open) { cout << "[["; open = true; }
        if (!mark[i] && open) { cout << "]]"; open = false; }
        cout << text[i];
        if (text[i] == '\n') cout << "    ";
    }
    if (open) cout << "]]";
    cout << "\n";
}

// program data
string doc1, doc2;
bool has1 = false, has2 = false;
vector<string> patterns;

// input helpers
string readLine() {
    string s;
    if (!getline(cin, s)) { cout << "\nInput closed. Exiting.\n"; exit(0); }
    if (!s.empty() && s.back() == '\r') s.pop_back();
    return s;
}

long long readInt(const string& prompt) {
    while (true) {
        cout << prompt;
        string s = readLine();
        try {
            size_t pos = 0;
            long long v = stoll(s, &pos);
            if (pos == s.size()) return v;
        } catch (...) {}
        cout << "  Invalid number, try again.\n";
    }
}

bool readDocument(const string& name, string& target) {
    cout << "Enter " << name << " text. Finish with a line containing only END.\n";
    string text, line;
    bool first = true;
    while (true) {
        line = readLine();
        if (line == "END") break;
        if (!first) text += '\n';
        text += line;
        first = false;
    }
    if (text.empty()) { cout << "  Document cannot be empty. Not saved.\n"; return false; }
    if (text.size() > MAX_DOC_LEN) {
        cout << "  Document too long (" << text.size() << " chars, limit " << MAX_DOC_LEN << "). Not saved.\n";
        return false;
    }
    target = text;
    cout << "  " << name << " saved (" << text.size() << " characters).\n";
    return true;
}

// pattern controls
void addPattern() {
    if (patterns.size() >= MAX_PATTERNS) { cout << "  Pattern limit reached.\n"; return; }
    cout << "Enter pattern: ";
    string p = readLine();
    if (p.empty()) { cout << "  Empty pattern rejected.\n"; return; }
    if (p.size() > MAX_PATTERN_LEN) { cout << "  Pattern too long (limit " << MAX_PATTERN_LEN << ").\n"; return; }
    if (find(patterns.begin(), patterns.end(), p) != patterns.end()) { cout << "  Pattern already added.\n"; return; }
    patterns.push_back(p);
    cout << "  Pattern added.\n";
}

void listPatterns() {
    if (patterns.empty()) { cout << "  No patterns.\n"; return; }
    for (size_t i = 0; i < patterns.size(); i++)
        cout << "  " << i + 1 << ". \"" << patterns[i] << "\"\n";
}

void removePattern() {
    if (patterns.empty()) { cout << "  No patterns to remove.\n"; return; }
    listPatterns();
    long long idx = readInt("Pattern number to remove: ");
    if (idx < 1 || idx > (long long)patterns.size()) { cout << "  Invalid number.\n"; return; }
    patterns.erase(patterns.begin() + (idx - 1));
    cout << "  Pattern removed.\n";
}

// pattern search
void searchInDocument(const string& name, const string& text) {
    cout << "\n=== " << name << " ===\n";
    vector<pair<size_t, size_t>> ranges;
    for (const string& p : patterns) {
        Stats kmp, naive;
        vector<size_t> pos = kmpSearch(text, p, kmp);
        vector<size_t> posNaive = naiveSearch(text, p, naive);
        if (pos != posNaive)
            cout << "  ERROR: KMP and Naive returned different results for \"" << p << "\"!\n";
        cout << "  Pattern \"" << p << "\": count = " << pos.size();
        if (!pos.empty()) {
            cout << ", positions = ";
            for (size_t i = 0; i < pos.size(); i++) {
                cout << pos[i] << (i + 1 < pos.size() ? ", " : "");
                ranges.push_back({pos[i], p.size()});
            }
        }
        cout << "\n";
        printComparison(kmp, naive);
    }
    cout << "  Highlighted text:\n";
    printHighlighted(text, ranges);
}

void runPatternSearch() {
    if (!has1 && !has2) { cout << "  Enter at least one document first.\n"; return; }
    if (patterns.empty()) { cout << "  Add at least one pattern first.\n"; return; }
    if (has1) searchInDocument("Document 1", doc1);
    if (has2) searchInDocument("Document 2", doc2);
}

// plagiarism detection
struct Region {
    size_t a, b, len;  // start in doc1, start in doc2, length
};

vector<Region> findRegions(SearchFn fn, size_t k, Stats& st) {
    vector<Region> regions;
    unordered_map<long long, pair<size_t, size_t>> open;  
    auto close = [&](long long d, const pair<size_t, size_t>& run) {
        regions.push_back({run.first, (size_t)((long long)run.first + d), run.second - run.first + k});
    };

    for (size_t i = 0; i + k <= doc1.size(); i++) {
        string window = doc1.substr(i, k);
        vector<size_t> hits = fn(doc2, window, st);
        for (size_t j : hits) {
            long long d = (long long)j - (long long)i;
            auto f = open.find(d);
            if (f != open.end() && f->second.second + 1 == i) {
                f->second.second = i;
            } else {
                if (f != open.end()) close(d, f->second);
                open[d] = {i, i};
            }
        }
    }
    for (auto& kv : open) close(kv.first, kv.second);

    vector<Region> kept;
    for (const Region& r : regions) {
        bool redundant = false;
        for (const Region& o : regions) {
            if (o.len > r.len && o.a <= r.a && r.a + r.len <= o.a + o.len &&
                o.b <= r.b && r.b + r.len <= o.b + o.len) { redundant = true; break; }
        }
        if (!redundant) kept.push_back(r);
    }

    sort(kept.begin(), kept.end(), [](const Region& x, const Region& y) {
        return x.a != y.a ? x.a < y.a : x.b < y.b;
    });
    return kept;
}

void runPlagiarism() {
    if (!has1 || !has2) { cout << "  Both Document 1 and Document 2 are required.\n"; return; }
    long long kk = readInt("Minimum match length k (characters): ");
    if (kk < 1) { cout << "  k must be at least 1.\n"; return; }
    size_t k = (size_t)kk;
    if (k > doc1.size() || k > doc2.size()) {
        cout << "  k is larger than a document. No matching regions possible.\n";
        return;
    }

    Stats kmp, naive;
    vector<Region> regions = findRegions(kmpSearch, k, kmp);
    vector<Region> regionsNaive = findRegions(naiveSearch, k, naive);
    bool same = regions.size() == regionsNaive.size();
    for (size_t i = 0; same && i < regions.size(); i++)
        same = regions[i].a == regionsNaive[i].a && regions[i].b == regionsNaive[i].b &&
               regions[i].len == regionsNaive[i].len;
    if (!same) cout << "  ERROR: KMP and Naive returned different regions!\n";

    cout << "\n=== Plagiarism matching regions (k = " << k << ") ===\n";
    cout << "  Matching regions found: " << regions.size() << "\n";
    vector<pair<size_t, size_t>> r1, r2;
    for (size_t i = 0; i < regions.size(); i++) {
        const Region& r = regions[i];
        cout << "  Region " << i + 1 << ": Doc1[" << r.a << ", " << r.a + r.len - 1
             << "]  <->  Doc2[" << r.b << ", " << r.b + r.len - 1 << "]  length " << r.len << "\n";
        cout << "    \"" << doc1.substr(r.a, r.len) << "\"\n";
        r1.push_back({r.a, r.len});
        r2.push_back({r.b, r.len});
    }
    printComparison(kmp, naive);
    cout << "\n  Document 1 with matching regions highlighted:\n";
    printHighlighted(doc1, r1);
    cout << "  Document 2 with matching regions highlighted:\n";
    printHighlighted(doc2, r2);
}

// menu
void printMenu() {
    cout << "\n Plagiarism Pattern Detector \n\n\n"
         << " 1. Enter / replace Document 1\n"
         << " 2. Enter / replace Document 2\n"
         << " 3. Add pattern\n"
         << " 4. List patterns\n"
         << " 5. Remove pattern\n"
         << " 6. Search patterns (KMP vs Naive)\n"
         << " 7. Detect plagiarism between Document 1 and Document 2 (KMP vs Naive)\n"
         << " 0. Exit\n";
}

int main() {
    while (true) {
        printMenu();
        long long choice = readInt("Choice: ");
        switch (choice) {
            case 1: if (readDocument("Document 1", doc1)) has1 = true; break;
            case 2: if (readDocument("Document 2", doc2)) has2 = true; break;
            case 3: addPattern(); break;
            case 4: listPatterns(); break;
            case 5: removePattern(); break;
            case 6: runPatternSearch(); break;
            case 7: runPlagiarism(); break;
            case 0: cout << "Goodbye.\n"; return 0;
            default: cout << "  Invalid choice.\n";
        }
    }
}