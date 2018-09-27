#ifndef LINE_READER_H
#define LINE_READER_H

#include <fstream>
#include <string>
#include <vector>

class LineReader
{
public:
    LineReader(const char *file_name);
    ~LineReader();
    bool NextLine(std::string &line);
    bool NextSplitItem(std::vector<std::string> &items, char delim='\t');
    void Split(std::string line, std::vector<std::string> &items, char delim);
private:
    bool Skip(std::string &line);
    void Trim(std::string &line);

private:
    std::ifstream reader_;
};

#endif
