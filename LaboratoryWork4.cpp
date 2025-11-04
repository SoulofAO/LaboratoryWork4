#include <iostream>      
#include <vector>         
#include <string>         
#include <sstream>        
#include <iomanip>         
#include <algorithm>       
#include <unordered_map>  
#include <random>          
#include <array>           
#include <chrono>         


using namespace std;

struct DieGroup {
    int count = 1;
    int sides = 6;
    int modifier = 0;
};

static inline std::string trim(const std::string& input_string) {
    size_t start_position = input_string.find_first_not_of(" \t\n\r");
    if (start_position == std::string::npos) {
        return "";
    }
    size_t end_position = input_string.find_last_not_of(" \t\n\r");
    return input_string.substr(start_position, end_position - start_position + 1);
}

bool parse_token(const string& token, DieGroup& output_group) {
    string trimmed_token = trim(token);
    if (trimmed_token.empty()) {
        return false;
    }
    auto dice_position = trimmed_token.find_first_of("dD");
    if (dice_position == string::npos) {
        return false;
    }

    string count_string = trimmed_token.substr(0, dice_position);
    string remaining_string = trimmed_token.substr(dice_position + 1);

    if (count_string.empty()) {
        output_group.count = 1;
    } else {
        try {
            output_group.count = stoi(count_string);
        } catch (...) {
            return false;
        }
        if (output_group.count < 0) {
            return false;
        }
    }

    size_t modifier_position = string::npos;
    for (size_t i = 0; i < remaining_string.size(); ++i) {
        if (remaining_string[i] == '+' || remaining_string[i] == '-') {
            modifier_position = i;
            break;
        }
    }

    string sides_string;
    string modifier_string;
    if (modifier_position == string::npos) {
        sides_string = remaining_string;
        modifier_string = "";
    } else {
        sides_string = remaining_string.substr(0, modifier_position);
        modifier_string = remaining_string.substr(modifier_position);
    }

    if (sides_string.empty()) {
        return false;
    }
    try {
        output_group.sides = stoi(sides_string);
    } catch (...) {
        return false;
    }
    if (output_group.sides <= 0) {
        return false;
    }

    if (!modifier_string.empty()) {
        try {
            output_group.modifier = stoi(modifier_string);
        } catch (...) {
            return false;
        }
    } else {
        output_group.modifier = 0;
    }

    return true;
}

bool parse_specification(const string& specification, vector<DieGroup>& output_groups) {
    output_groups.clear();
    string specification_copy = specification;
    size_t start_position = 0;
    while (start_position < specification_copy.size()) {
        size_t comma_position = specification_copy.find(',', start_position);
        string token;
        if (comma_position == string::npos) {
            token = specification_copy.substr(start_position);
            start_position = specification_copy.size();
        }
        else {
            token = specification_copy.substr(start_position, comma_position - start_position);
            start_position = comma_position + 1;
        }
        if (trim(token).empty()) {
            continue;
        }
        DieGroup group;
        if (!parse_token(token, group)) {
            return false;
        }
        output_groups.push_back(group);
    }
    return !output_groups.empty();
}

string groups_to_string(const vector<DieGroup>& groups) {
    stringstream string_stream;
    bool is_first = true;
    for (auto& group : groups) {
        if (!is_first) {
            string_stream << ",";
        }
        is_first = false;
        string_stream << group.count << "d" << group.sides;
        if (group.modifier > 0) {
            string_stream << "+" << group.modifier;
        } else if (group.modifier < 0) {
            string_stream << group.modifier;
        }
    }
    return string_stream.str();
}

int roll_dice(const vector<DieGroup>& groups) {
    std::mt19937 random_generator(std::random_device{}());
    int total = 0;
    for (const auto& group : groups) {
        if (group.sides <= 0 || group.count <= 0) {
            continue;
        }
        std::uniform_int_distribution<int> distribution(1, group.sides);
        for (int i = 0; i < group.count; ++i) {
            total += distribution(random_generator);
        }
        total += group.modifier;
    }
    return total;
}

int roll_dice(const string& specification) {
    vector<DieGroup> groups;
    if (!parse_specification(specification, groups)) {
        throw runtime_error("Invalid dice spec: " + specification);
    }
    return roll_dice(groups);
}

unordered_map<int, long long> simulate_dice_rolls(const vector<DieGroup>& groups, long long number_of_trials) {
    unordered_map<int, long long> count_map;
    count_map.reserve(1024);
    for (long long trial = 0; trial < number_of_trials; ++trial) {
        int roll_result = roll_dice(groups);
        ++count_map[roll_result];
    }
    return count_map;
}

void print_distribution_table(const unordered_map<int, long long>& count_map, long long number_of_trials, int max_width = 60) {
    if (count_map.empty()) {
        return;
    }
    int min_value = INT_MAX, max_value = INT_MIN;
    for (auto& pair : count_map) {
        min_value = min(min_value, pair.first);
        max_value = max(max_value, pair.first);
    }
    long long maximum_count = 0;
    for (auto& pair : count_map) {
        maximum_count = max(maximum_count, pair.second);
    }

    cout << "Value\tCount\tProb\tHistogram\n";
    for (int value = min_value; value <= max_value; ++value) {
        long long count = 0;
        auto iterator = count_map.find(value);
        if (iterator != count_map.end()) {
            count = iterator->second;
        }
        double probability = (double)count / (double)number_of_trials;
        int bar_length = (maximum_count == 0) ? 0 : (int)round((double)count / (double)maximum_count * max_width);
        cout << setw(3) << value << "\t" << setw(7) << count << "\t" << fixed << setprecision(5) << probability << "\t";
        for (int i = 0; i < bar_length; ++i) {
            cout << "#";
        }
        cout << "\n";
    }
}

void demonstrate_specification_and_distribution(const string& specification_string, long long number_of_trials = 100000) {
    cout << "Specification: \"" << specification_string << "\"\n";
    vector<DieGroup> groups;
    if (!parse_specification(specification_string, groups)) {
        cout << "  Parsing error in specification.\n";
        return;
    }
    cout << "  Parsed as: " << groups_to_string(groups) << "\n";
    cout << "  One sample roll: " << roll_dice(groups) << "\n";
    cout << "  Simulation of " << number_of_trials << " rolls...\n";
    auto count_map = simulate_dice_rolls(groups, number_of_trials);
    print_distribution_table(count_map, number_of_trials);
    cout << "\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    try {
        cout << "Example: roll_dice(\"2d6+2,3d10, 8d10+6\") => ";
        cout << roll_dice("2d6+2,3d10, 8d10+6") << "\n\n";

        vector<string> specification_list = { "1d6", "2d6", "3d6", "1d10", "2d10", "3d10" };
        const long long number_of_trials = 100000;
        for (const auto& specification : specification_list) {
            demonstrate_specification_and_distribution(specification, number_of_trials);
        }

    }
    catch (const std::exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

