#include <random>
#include <string>

std::string randomFilename() {
    const std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const int length = 16;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, charset.length() - 1);

    std::string filename;
    for (int i = 0; i < length; ++i) {
        filename += charset[dis(gen)];
    }

    return filename;
}