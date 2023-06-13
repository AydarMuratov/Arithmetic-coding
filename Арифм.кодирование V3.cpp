#include <iostream>
#include <fstream>
#include <vector>

constexpr auto NO_OF_CHARS = 256;
constexpr auto EOF_SYMBOL = NO_OF_CHARS;

std::vector<int> freq(NO_OF_CHARS + 1, 0);
std::vector<int> cum_freq(NO_OF_CHARS + 2, 0);

void initialize()
{
    for (int i = 0; i <= NO_OF_CHARS; ++i) {
        freq[i] = 1;
        cum_freq[i + 1] = cum_freq[i] + freq[i];
    }
    freq[EOF_SYMBOL] = 0;
}

void compress(const std::string& inputFileName, const std::string& outputFileName)
{
    std::ifstream inputFile(inputFileName, std::ios::binary);
    std::ofstream outputFile(outputFileName, std::ios::binary);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "Failed to open input or output file!" << std::endl;
        return;
    }

    initialize();

    int low = 0;
    int high = cum_freq[NO_OF_CHARS + 1] - 1;
    int range = high - low + 1;
    int bitsToFollow = 0;
    int bitsBuffer = 0;
    int bitsCount = 0;

    int symbol;
    while ((symbol = inputFile.get()) != EOF_SYMBOL) {
        int newLow = low + (range * cum_freq[symbol]) / cum_freq[NO_OF_CHARS + 1];
        int newHigh = low + (range * cum_freq[symbol + 1]) / cum_freq[NO_OF_CHARS + 1] - 1;

        range = newHigh - newLow + 1;
        low = newLow;
        high = newHigh;

        for (;;) {
            if (high < 0x8000) {
               
                if (bitsCount == 8) {
                    outputFile.put(static_cast<char>(bitsBuffer));
                    bitsBuffer = 0;
                    bitsCount = 0;
                }
                bitsBuffer <<= 1;
                bitsCount++;
            }
            else if (low >= 0x8000) {
               
                bitsBuffer <<= 1;
                bitsBuffer |= 1;
                bitsCount++;
        
                for (; bitsToFollow > 0; --bitsToFollow) {
                    bitsBuffer <<= 1;
                    if (bitsCount == 8) {
                        outputFile.put(static_cast<char>(bitsBuffer));
                        bitsBuffer = 0;
                        bitsCount = 0;
                    }
                    bitsCount++;
                }
                low -= 0x8000;
                high -= 0x8000;
            }
            else if (low >= 0x4000 && high < 0xC000) {
                // Write out a bit of zero
                bitsToFollow++;
                low -= 0x4000;
                high -= 0x4000;
            }
            else {
                break;
            }

            low <<= 1;
            high <<= 1;
            high |= 1;
        }
    }

    bitsToFollow++;
    if (low < 0x4000) {
        bitsBuffer <<= 1;
        bitsCount++;
    }
    else {
        bitsBuffer <<= 1;
        bitsBuffer |= 1;
        bitsCount++;
    }

    for (; bitsToFollow > 0; --bitsToFollow) {
        bitsBuffer <<= 1;
        if (bitsCount == 8) {
            outputFile.put(static_cast<char>(bitsBuffer));
            bitsBuffer = 0;
            bitsCount = 0;
        }
        bitsCount++;
    }

    if (bitsCount > 0) {
        bitsBuffer <<= (8 - bitsCount);
        outputFile.put(static_cast<char>(bitsBuffer));
    }

    inputFile.close();
    outputFile.close();
}

void decompress(const std::string& inputFileName, const std::string& outputFileName)
{
    std::ifstream inputFile(inputFileName, std::ios::binary);
    std::ofstream outputFile(outputFileName, std::ios::binary);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "Failed to open input or output file!" << std::endl;
        return;
    }

    initialize();

    int low = 0;
    int high = cum_freq[NO_OF_CHARS + 1] - 1;
    int value = 0;

    for (int i = 0; i < 16; ++i) {
        value <<= 1;
        value |= (inputFile.get() & 1);
    }

    for (;;) {
        int range = high - low + 1;
        int scaledValue = ((value - low + 1) * cum_freq[NO_OF_CHARS]) / range;
        int symbol = 0;

        while (cum_freq[symbol + 1] <= scaledValue) {
            ++symbol;
        }

        int newLow = low + (range * cum_freq[symbol]) / cum_freq[NO_OF_CHARS + 1];
        int newHigh = low + (range * cum_freq[symbol + 1]) / cum_freq[NO_OF_CHARS + 1] - 1;

        outputFile.put(static_cast<char>(symbol));

        while ((low & 0x8000) == (newLow & 0x8000) && (high & 0x8000) == (newHigh & 0x8000)) {
            low <<= 1;
            high <<= 1;
            high |= 1;
            value <<= 1;
            value |= (inputFile.get() & 1);
        }

        if ((low & 0x4000) != 0 && (high & 0x4000) == 0) {
            low &= 0x3FFF;
            high |= 0x4000;
            value &= 0x3FFF;
            value |= (inputFile.get() & 1);
        }
        else if ((low & 0x8000) == 0x8000 && (high & 0x8000) == 0x8000) {
            low &= 0x7FFF;
            high |= 0x8000;
            value &= 0x7FFF;
            value |= (inputFile.get() & 1);
        }
        else {
            break;
        }

        low = newLow;
        high = newHigh;
    }

    inputFile.close();
    outputFile.close();
}

int main()
{
    compress("DataFile.txt", "CompressedDataFile.txt");
    decompress("CompressedDataFile.txt", "DecompressedDataFile.txt");

    return 0;
}
