#include <iterator>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>


class CSVRow {
    private:
        std::string line;
        std::vector<double> data;

    public:
        std::string_view operator [] (std::size_t index) const;
        std::size_t size() const;
        void readNextRow(std::istream& str);
};