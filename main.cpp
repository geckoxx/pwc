#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ios>
#include <map>
#include <string>

using namespace std;

string readFile(const char* filename) {
    ifstream in;
    in.open(filename);
    stringstream sstr;
    sstr << in.rdbuf();
    return sstr.str();
}

int main(int argc, char **argv) {
    if(argc == 1) {
        cout << "Einzulesende Datei angeben\n" << endl;
        return 0;
    }
    int processes = 4;
    string data = readFile(argv[1]);
    //cout << data << endl;
    
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
    //cout << "Neue saubere Version:\n" << endl;
    //cout << data << endl;
    
    map<string, int> wordcounts;
    string word;
    for(int j = 0; j < data.length(); j++) {
        char sign = data.at(j);
        if(sign == ' ') {
            if(word.empty())
                continue;
            try {
                int count = wordcounts.at(word);
                wordcounts[word] = ++count;
            } catch (const std::out_of_range& oor) {
                wordcounts[word] = 1;
            }
            word = "";
        } else {
            word += sign;
        }
    }
    cout << "\nWortanzahl:\n" << endl;
    
    for(map<string, int>::iterator iter = wordcounts.begin(); iter!=wordcounts.end(); ++iter) {
        cout << iter->first << ": " << iter->second << endl;
    }
    return 0;
}
