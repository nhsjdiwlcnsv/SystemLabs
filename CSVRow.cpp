#include "CSVRow.h"


std::string_view CSVRow::operator [] (std::size_t index) const {
    return std::string_view(&line[data[index] + 1], data[index + 1] - data[index] - 1);
}

std::size_t CSVRow::size() const {
    return data.size() - 1;
}

void CSVRow::readNextRow(std::istream& str) {
    std::getline(str, line);
        
    data.clear();
    data.emplace_back(-1);
            
    std::size_t pos = 0;

    while(line.find(',', pos) != std::string::npos) {
        data.emplace_back(pos);
        ++pos;
    }

    // This checks for a trailing comma with no data after it.
    pos = line.size();
    data.emplace_back(pos);
}