#include "LineReader.h"
#include <cctype>
#include "common.h"

using std::vector;
using std::string;

LineReader::LineReader(const char *file_name):reader_(file_name) {
    logger("filename: %s", file_name);
}

LineReader::~LineReader() {
    if (reader_ && reader_.is_open()) {
        reader_.close();
    }
}

bool LineReader::NextLine(string &line) {
    if (! reader_ ) {
        logger("fail to open file");
        return false;
    }

    while (getline(reader_, line)) {
        if (!Skip(line)) {
            return true;
        }
    }
    
    return false;
}

bool LineReader::NextSplitItem(vector<string> &items, char delim) {
    string line;
    items.clear();
    if (NextLine(line)) {
        Split(line, items, delim);
        return true;
    }

    return false;
}

bool LineReader::Skip(std::string &line) {
    Trim(line);
    if (line.empty() || line[0] ==  '#' ||
        line[0] == '\n' || line[0] == '\0') {
        return true;
    }

    return false;
}

void LineReader::Trim(string &line) {
    size_t start = 0;
    size_t end = line.size();
    //logger("before trim %s", line.c_str());
    while (start < end && isspace(line[start])) {
        ++start;
    }

    while (end > start && isspace(line[end])) {
        --end;
    }

    string tmp;
    if (start < end) {
        tmp = line.substr(start, end - start + 1);
    }

    line = tmp;
    //logger("after trim %s", tmp.c_str());
}

void LineReader::Split(string line,  vector<string> &items, char delim) {
    items.clear();
    ssize_t start = 0;
    ssize_t pos = 0;
    string item;
    while (start < line.size()) {
        pos = line.find(delim, start);
        if (pos == string::npos) {
            item = line.substr(start);
            items.push_back(item);
            break;
        }
        item = line.substr(start, pos - start);
        items.push_back(item);
        start = pos + 1;
    }
}
