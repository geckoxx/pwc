#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ios>
#include <map>
#include <algorithm>
#include <string>
#include <thread>
#include <mutex>

using namespace std;

mutex ctrMutex;
string data;
map<string, int> wordcounts;

string readFile(const char* filename) {
    ifstream in;
    in.open(filename);
    stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

void cleanData() {
    int i = 0;
    while(i < data.length()) {
        string strsign = data.substr(i, 2);
        char sign = data.at(i);
        if(sign == '\n')
            data.replace(i, 1, " ");
        else if(sign == ' ')
            data.replace(i, 1, " ");
        else if(strsign == "Ä" || strsign == "ä") {
            data.replace(i, 1, "ä");
            i++;
        } else if(strsign == "Ö" || strsign == "ö") {
            data.replace(i, 1, "ö");
            i++;
        } else if(strsign == "Ü" || strsign == "ü") {
            data.replace(i, 1, "ü");
            i++;
        } else if(strsign == "ß") {
            data.replace(i, 1, "ß");
            i++;
        } else if(sign < 65 || (sign > 90 && sign < 97) || sign > 122) { //Sonderzeichen entfernen
            data.replace(i, 1, "");
            i--;
        } else if(sign > 64 && sign < 91) { //große Buchstaben
            string newsign;
            newsign += (char)(sign + 32); //kleine Schreibweise
            data.replace(i, 1, newsign);
        }
        i++;
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
            try {
                int count = wordcounts.at(word);
                wordcounts[word] = ++count;
            } catch (const std::out_of_range& oor) {
                wordcounts[word] = 1;
            }
            ctrMutex.unlock();
            word = "";
        } else {
            word += sign;
        }
    }
}

template<typename A, typename B>
pair<B,A> flip_pair(const pair<A,B> &p)
{
    return pair<B,A>(p.second, p.first);
}

template<typename A, typename B>
multimap<B,A> flip_map(const map<A,B> &src)
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
    int processCount = 4;
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
    for(int i = 0; i < processCount; i++) {
        processes[i].join();
    }
    cout << "\nWortanzahl:\n" << endl;
    
    multimap<int, string> sortedMap = flip_map(wordcounts);
    for(multimap<int, string>::iterator iter = --sortedMap.end(); iter!=sortedMap.begin(); --iter) {
        cout << iter->second << ": " << iter->first << endl;
    }
    return 0;
}
