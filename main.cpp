#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ios>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <thread>
#include <mutex>
#include <queue>

using namespace std;

const int processCount = 4;
mutex ctrMutex;
string data;
unordered_map<string, int> wordcounts;
queue<string> words;

string* txtParts;
int threadsDone = 0;

string readFile(const char* filename) {
    ifstream in;
    in.open(filename);
    stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

void cleanDataThread(int index) {
    string* threadData = &txtParts[index];
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

void cleanData() {
    string tp[processCount];
    txtParts = tp;
    int threadlength = data.length() / processCount;
    thread processes[processCount];
    for(int i = 0; i < processCount; i++) {
        int start = i * threadlength;
        int end;
        if((i + 1) == processCount)
            end = data.length();
        else
            end = (i + 1) * threadlength;
        txtParts[i] = data.substr(start, end - start);
        processes[i] = thread(cleanDataThread, i);
    }
    data = "";
    for(int i = 0; i < processCount; i++) {
        processes[i].join();
        data.append(txtParts[i]);
    }
    
    data.append(new char(' '));
}

void wordcounter(int start, int end) {
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
            ctrMutex.lock();
            words.push(word);
            ctrMutex.unlock();
            word = "";
        } else {
            word += sign;
        }
    }
    ++threadsDone;
}

template<typename A, typename B>
pair<B,A> flip_pair(const pair<A,B> &p)
{
    return pair<B,A>(p.second, p.first);
}

template<typename A, typename B>
multimap<B,A> flip_map(const unordered_map<A,B> &src)
{
    multimap<B,A> dst;
    transform(src.begin(), src.end(), std::inserter(dst, dst.begin()), flip_pair<A,B>);
    return dst;
}

int main(int argc, char **argv) {
    if(argc == 1) {
        cout << "Einzulesende Datei angeben\n" << endl;
        return 0;
    }
    data = readFile(argv[1]);
    //cout << data << endl;
    
    cleanData();
    
    //cout << "Neue saubere Version:\n" << endl;
    //cout << data << endl;
    
    int threadlength = data.length() / processCount;
    thread processes[processCount];
    for(int i = 0; i < processCount; i++) {
        int start = i * threadlength;
        int end;
        if((i + 1) == processCount)
            end = data.length();
        else
            end = (i + 1) * threadlength;
        processes[i] = thread(wordcounter, start, end);
    }
    while(threadsDone < processCount || !words.empty()) {
        string word = words.front();
        try {
            int count = wordcounts.at(word);
            wordcounts[word] = ++count;
        } catch (const std::out_of_range& oor) {
            wordcounts[word] = 1;
        }
        ctrMutex.lock();
        words.pop();
        ctrMutex.unlock();
    }
    for(int i = 0; i < processCount; i++) {
        processes[i].join();
    }
    cout << "\nWortanzahl:\n" << endl;
    
    multimap<int, string> sortedMap = flip_map(wordcounts);
    for(multimap<int, string>::iterator iter = sortedMap.begin(); iter!=sortedMap.end(); ++iter) {
        cout << iter->second << ": " << iter->first << endl;
    }
    
    //cin.get();
    return 0;
}
