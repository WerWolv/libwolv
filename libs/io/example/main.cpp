#include <wolv/io/file.hpp>
#include <wolv/io/fs.hpp>
#include <wolv/io/buffered_reader.hpp>

#include <cstring>

void StringReader(std::string *userData, void *buffer, wolv::u64 address, size_t size) {
    memcpy(buffer, &(*userData)[address], size);
}

int main() {

    std::string testString = "Hello World";
    wolv::io::BufferedReader<std::string, StringReader> reader(&testString, testString.size());

    for (auto c : reader) {
        printf("%c", c);
    }

}