#include <iostream>
#include <fstream>
#include <vector>
#include <map>

using namespace std;
class BitStream {
public:
    BitStream() : buffer(0), pos(0) {}
    void write(bool bit) {
        buffer |= bit << (7 - pos);
        pos++;
        if (pos == 8) {
            stream.put(buffer);
            buffer = 0;
            pos = 0;
        }
    }
    bool read() {
        if (pos == 0) {
            buffer = stream.get();
            pos = 8;
        }
        bool bit = (buffer >> (pos - 1)) & 1;
        pos--;
        return bit;
    }
    string str() {
        return stream.str();
    }

private:
    ostringstream stream;
    uint8_t buffer;
    uint8_t pos;
};
class Alphabet {
public:
    void add(char symbol) {
        if (symbols.find(symbol) == symbols.end()) {
            symbols[symbol] = freq.size();
            freq.push_back(1);
        } else {
            freq[symbols[symbol]]++;
        }
    }
    size_t size() const {
        return freq.size();
    }
    int symbolToIndex(char symbol) const {
        return symbols.at(symbol);
    }
    char indexToSymbol(int index) const {
        for (auto pair : symbols) {
            if (pair.second == index) {
                return pair.first;
            }
        }
        return 0;
    }
    int getFrequency(int index) const {
        return freq[index];
    }
    int getTotalFrequency() const {
        int total = 0;
        for (int i = 0; i < freq.size(); i++) {
            total += freq[i];
        }
        return total;
    }

private:
    map<char, int> symbols;
    vector<int> freq;
};
class ArithmeticCoding {
public:
    ArithmeticCoding(Alphabet alphabet) : alphabet(alphabet) {}

    void encode(string inputFilename, string outputFilename) {
        ifstream inputFile(inputFilename, ios::binary);
        BitStream output;
        int symbol;
        double lower = 0;
        double upper = 1;
        double range = 1;
        while ((symbol = inputFile.get()) != EOF) {
            int index = alphabet.symbolToIndex(static_cast<char>(symbol));
            int totalFreq = alphabet.getTotalFrequency();
            int freq = alphabet.getFrequency(index);
            double newRange = range * freq / totalFreq;
            upper = lower + newRange;
            lower += range * alphabet.getFrequency(index - 1) / totalFreq;
            range = newRange;
            while (true) {
                                if (upper <= 0.5) {
                    output.write(false);
                    while (!pendingBits.empty()) {
                        output.write(!pendingBits.back());
                        pendingBits.pop_back();
                    }
                } else if (lower >= 0.5) {
                    output.write(true);
                    while (!pendingBits.empty()) {
                        output.write(pendingBits.back());
                        pendingBits.pop_back();
                    }
                } else if (lower >= 0.25 && upper <= 0.75) { 
                    pendingBits.push_back(upper >= 0.5);
                    upper = 2 * (upper - 0.5);
                    lower = 2 * (lower - 0.5);
                } else {
                    break;
                }
            }
        }

        for (int i = 0; i < 16; i++) {
            output.write(lower < 0.5);
            lower = 2 * lower;
            upper = 2 * upper;
            if (upper >= 1) {
                output.write(true);
                while (!pendingBits.empty()) {
                    output.write(pendingBits.back());
                    pendingBits.pop_back();
                }
                break;
            } else if (lower >= 1) {
                output.write(false);
                while (!pendingBits.empty()) {
                    output.write(!pendingBits.back());
                    pendingBits.pop_back();
                }
                break;
            }
        }

        inputFile.close();
        output.write(true); // Stop bit

        ofstream outputFile(outputFilename, ios::binary);
        outputFile << output.str();
        outputFile.close();
    }

    void decode(string inputFilename, string outputFilename) {
        BitStream input;
        ifstream inputFile(inputFilename, ios::binary);
        while (!input.read()) {
            // Пропускаем ведущие нули
        }
        double lower = 0;
        double upper = 1;
        double range = 1;
        int symbol;
        while (true) {
            symbol = 0;
            while (range < 0.5) {
                range *= 2;
                lower *= 2;
                upper *= 2;
                if (input.read()) {
                    lower += range / 2;
                }
            }
            while (true) {
                int totalFreq = alphabet.getTotalFrequency();
                double newRange = range / totalFreq;
                int index;
                for (index = 0; index < alphabet.size(); index++) {
                    int freq = alphabet.getFrequency(index);
                    if (lower + newRange * freq > upper) {
                        break;
                    }
                }
                if (index == alphabet.size()) {
                    break;
                }
                symbol = alphabet.indexToSymbol(index);
                upper = lower + newRange * alphabet.getFrequency(index);
                lower += newRange * alphabet.getFrequency(index - 1);
                range = newRange;
            }
            if (symbol == 0) {
                break;
            }
            ofstream outputFile(outputFilename, ios::binary | ios::app);
            outputFile.put(symbol);
            outputFile.close();
        }
        inputFile.close();
    }

private:
    Alphabet alphabet;
    vector<bool> pendingBits;
};

int main() {
    cout << "Enter input filename: ";
    string inputFilename;
    cin >> inputFilename;
    cout << "Enter output filename: ";
    string outputFilename;
    cin >> outputFilename;
    cout << "Enter mode (0 for encode, 1 for decode): ";
    int mode;
    cin >> mode;
    if (mode == 0) {
        Alphabet alphabet;
        ifstream inputFile(inputFilename, ios::binary);
        int symbol;
        while ((symbol = inputFile.get()) != EOF) {
            alphabet.add(static_cast<char>(symbol));
        }
        inputFile.close();
        ArithmeticCoding encoder(alphabet);
        encoder.encode(inputFilename, outputFilename);
    } else if (mode == 1) {
        Alphabet alphabet;
        ifstream inputFile(inputFilename, ios::binary);
        int symbol;
        while ((symbol = inputFile.get()) != EOF) {
            alphabet.add(static_cast<char>(symbol));
        }
        inputFile.close();
        ArithmeticCoding decoder(alphabet);
        decoder.decode(inputFilename, outputFilename);
    } else {
        cout << "Invalid mode. Please enter 0 or 1." << endl;
    }

    return 0;
}


