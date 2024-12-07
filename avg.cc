#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

int main() {
    std::string file_name = "results.txt"; // Change to the name of your file
    std::ifstream file(file_name);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file " << file_name << std::endl;
        return 1;
    }

    std::string line;
    int total_score = 0;
    int count = 0;

    while (std::getline(file, line)) {
        if (line.find("info: player jim3 got score") != std::string::npos) {
            // Extract the score from the line
            std::istringstream iss(line);
            std::string word;
            int score;

            while (iss >> word) {
                // Try to convert the last part to an integer
                try {
                    score = std::stoi(word);
                } catch (const std::invalid_argument&) {
                    continue; // Skip words that aren't integers
                }
            }

            total_score += score;
            count++;
        }
    }

    file.close();

    if (count == 0) {
        std::cout << "No scores found in the file." << std::endl;
    } else {
        double average = static_cast<double>(total_score) / count;
        std::cout << "Average score: " << average << std::endl;
    }

    return 0;
}
