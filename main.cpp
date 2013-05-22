#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ios>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <omp.h>
#include <parallel/algorithm>
#include <ctime>

using namespace std;

string data;
unordered_map<string, int> wordcounts;
map<int, vector<string>> int2words;

clock_t in_time;
clock_t seq_time;
clock_t par_time;
clock_t par_clean_time;
clock_t par_count_time;

string readFile(const char* filename) {
    ifstream in;
    in.open(filename);
    stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

void ompClean() {
    clock_t clean_par = clock();
    string* strParts;
    int threads;
    #pragma omp parallel
    {
        strParts = new string[omp_get_num_threads()];
    }
    #pragma omp parallel
    {
        threads = omp_get_num_threads();
        int threadlength = data.length() / omp_get_num_threads();
        int start = omp_get_thread_num() * threadlength;
        int end;
        if((omp_get_thread_num() + 1) == omp_get_num_threads())
            end = data.length();
        else
            end = (omp_get_thread_num() + 1) * threadlength;
        strParts[omp_get_thread_num()] = data.substr(start, end - start);

        string* threadData = &strParts[omp_get_thread_num()];
        int i = 0;
        while(i < threadData->length()) {
            string strsign = threadData->substr(i, 2);
            char sign = threadData->at(i);
            if(sign == '\n') {
                threadData->replace(i, 1, " ");
            } else if(strsign == "Ä" || strsign == "ä") {
                threadData->replace(i, 1, "ä");
                i++;
            } else if(strsign == "Ö" || strsign == "ö") {
                threadData->replace(i, 1, "ö");
                i++;
            } else if(strsign == "Ü" || strsign == "ü") {
                threadData->replace(i, 1, "ü");
                i++;
            } else if(strsign == "ß") {
                threadData->replace(i, 1, "ß");
                i++;
            } else if(sign != ' ' && (sign < 65 || (sign > 90 && sign < 97) || sign > 122)) { //Sonderzeichen entfernen
                threadData->replace(i, 1, "");
                i--;
            } else if(sign > 64 && sign < 91) { //große Buchstaben
                string newsign;
                newsign += (char)(sign + 32); //kleine Schreibweise
                threadData->replace(i, 1, newsign);
            }
            i++;
        }
    }
    clock_t clean_par_end = clock();
    par_clean_time = (clean_par_end - clean_par);
    par_time += par_clean_time;

    data = "";
    for(int i = 0; i < threads; i++) {
        data.append(strParts[i]);
    }
    data.append(new char(' '));
    seq_time += (clock() - clean_par_end);
}

void ompWordcounter() {
    #pragma omp parallel
    {
        int threadlength = data.length() / omp_get_num_threads();
        int start = omp_get_thread_num() * threadlength;
        int end;
        if((omp_get_thread_num() + 1) == omp_get_num_threads())
            end = data.length();
        else
            end = (omp_get_thread_num() + 1) * threadlength;

        string word;
        for(int i = start; i > -1; i--) {
            if(data.at(i) == ' ')
                break;
            else
                start = i;
        }
        for(int j = start; j < end; j++) {
            char sign = data.at(j);
            if(sign == ' ') {
                if(word.empty())
                    continue;
                #pragma omp critical
                if(wordcounts.end() == wordcounts.find(word))
                    wordcounts[word] = 1;
                else {
                    int count = wordcounts[word];
                    wordcounts[word] = ++count;
                }
                #pragma omp end critical
                word = "";
            } else {
                word += sign;
            }
        }
    }
}

void mySort(std::pair<const string, int>& pair) {

    if(int2words.end() == int2words.find(pair.second)) {
        vector<string> newWord;
        newWord.push_back(pair.first);
        int2words[pair.second] = newWord;
    } else {
        int2words[pair.second].push_back(pair.first);
    }
}

int main(int argc, char **argv) {
    
    seq_time = 0;
    par_time = 0;
    
    clock_t comp_start = clock();
    if(argc == 1) {
        cout << "Einzulesende Datei angeben\n" << endl;
        return 0;
    }
    
    clock_t start = clock();
    data = readFile(argv[1]);
    in_time = clock() - start;
    seq_time += in_time;

    ompClean();

    start = clock();
    ompWordcounter();
    par_count_time = (clock() - start);
    par_time += par_count_time;

    start = clock();
    __gnu_parallel::for_each(wordcounts.begin(), wordcounts.end(), mySort);
    seq_time += (clock() - start);

    start = clock();
    cout << "\nWortanzahl:\n" << endl;
    for(map<int, vector<string>>::iterator iter = int2words.begin(); iter!=int2words.end(); ++iter) {
        if(iter->first < 1000)
            continue;
        cout << iter->first << ": ";
        for(int i = 0; i < iter->second.size(); i++) {
            cout << iter->second.at(i);
            if(i < (iter->second.size() - 1))
                cout << ", ";
        }
        cout << endl;
    }
    seq_time += (clock() - start);
    
    long clock_per_sec = CLOCKS_PER_SEC / 1000;
    clock_t comp_end = clock() - comp_start;
    cout << endl << "Zeit des sequenzellen Teils: " << (long)(seq_time / clock_per_sec) << "ms" << endl;
    cout << "    davon für Einlesen: " << (long)(in_time / clock_per_sec) << "ms" << endl;
    cout << "Zeit des parallelen Teils: " << (long)(par_time / clock_per_sec) << "ms" << endl;
    cout << "    davon für Textbereinigung: " << (long)(par_clean_time / clock_per_sec) << "ms" << endl;
    cout << "    davon für Wortzählung: " << (long)(par_count_time / clock_per_sec) << "ms" << endl;
    cout << "Gesamtzeit: " << (long)(comp_end / clock_per_sec) << "ms" << endl;

    return 0;
}
