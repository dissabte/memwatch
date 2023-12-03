#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <unistd.h> // for getpagesize()

namespace {

std::pair<ssize_t, std::string> humanFriendlyBytes(ssize_t bytes) {
    if (bytes < 1024) {
        return {bytes, "bytes"}; 
    } else if (bytes < 1024*1024) {
        return {bytes / 1024, "KiB"};
    } else if (bytes < 1024*1024*1024) {
        return {bytes / (1024*1024), "MiB"};
    } else {
        return {bytes / (1024*1024*1024), "GiB"};
    }
}

} // unnamed namespace

// Function to parse and print memory usage statistics
void printMemoryUsage(int pid) {
    // Construct the path to the statm file
    const std::string statmPath = "/proc/" + std::to_string(pid) + "/statm";

    // Open the statm file
    std::ifstream statmFile(statmPath);
    if (!statmFile.is_open()) {
        std::cerr << "Error: Could not open " << statmPath << std::endl;
        return;
    }

    // Read the content of the statm file
    std::string line;
    getline(statmFile, line);

    // Close the statm file
    statmFile.close();

    // Parse the memory usage statistics
    std::istringstream iss(line);
    long size, resident, shared, text, lib, data, dirty;

    iss >> size >> resident >> shared >> text >> lib >> data >> dirty;

    // TODO: use it
    const int pageSize = getpagesize();

    // Print the memory usage statistics
    std::cout << "Memory Usage Statistics for Process " << pid << ":" << std::endl;

    const auto& [sizeBytes, sizeSuffix] = humanFriendlyBytes(size * pageSize); 
    std::cout << "Virtual:      " << size << " pages (" << sizeBytes << " " << sizeSuffix << ")\n";

    const auto& [residentBytes, residentSuffix] = humanFriendlyBytes(resident * pageSize);
    std::cout << "Resident:     " << resident << " pages (" << residentBytes << " " << residentSuffix << ")\n";

    const auto& [sharedBytes, sharedSuffix] = humanFriendlyBytes(shared * pageSize);
    std::cout << "Shared:       " << shared << " pages (" << sharedBytes << " " << sharedSuffix << ")\n";

    const auto& [textBytes, textSuffix] = humanFriendlyBytes(text * pageSize);
    std::cout << "Text (Code):  " << text << " pages (" << textBytes << " " << textSuffix << ")\n";

    const auto& [dataBytes, dataSuffix] = humanFriendlyBytes(data * pageSize);
    std::cout << "Data + Stack: " << data << " pages (" << dataBytes << " " << dataSuffix << ")" << std::endl;
}


// Function to parse and print process statistics
void printProcessStats(int pid) {
    // Construct the path to the stat file
    std::string statPath = "/proc/" + std::to_string(pid) + "/stat";

    // Measure the time required for file operations
    auto start = std::chrono::high_resolution_clock::now();

    // Open the stat file
    std::ifstream statFile(statPath);
    if (!statFile.is_open()) {
        std::cerr << "Error: Could not open " << statPath << std::endl;
        return;
    }

    // Read the content of the stat file
    std::string line;
    getline(statFile, line);

    // Close the stat file
    statFile.close();

    // Measure the time taken
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    // Parse the process statistics
    std::istringstream iss(line);
    std::string comm;  // Command (executable name)
    char state;   // Process state
    int ppid, pgrp, session, tty_nr, tpgid;
    unsigned int flags;
    long minflt, cminflt, majflt, cmajflt, utime, stime;

    iss >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr >> tpgid >> flags
        >> minflt >> cminflt >> majflt >> cmajflt >> utime >> stime;

    double ticksPerSecond = static_cast<double>(sysconf(_SC_CLK_TCK));

    // Print the process statistics
    std::cout << "Process Statistics for Process " << pid << ":" << "\n";
    std::cout << "Command: " << comm << "\n";
    std::cout << "State: " << state << "\n";
    std::cout << "Parent Process ID: " << ppid << "\n";
    std::cout << "Process Group ID: " << pgrp << "\n";
    std::cout << "Session ID: " << session << "\n";
    std::cout << "Minor Page Faults: " << minflt << "\n";
    std::cout << "Major Page Faults: " << majflt << "\n";
    std::cout << "Child Minor Page Faults: " << cminflt << "\n";
    std::cout << "Child Major Page Faults: " << cmajflt << "\n";
    std::cout << "User Mode Time: " << static_cast<double>(utime) / ticksPerSecond << " seconds\n";
    std::cout << "Kernel Mode Time: " << static_cast<double>(stime) / ticksPerSecond << " seconds\n";
    std::cout << "Time it took to get the data: " << duration.count() << "us" << std::endl;
}


// Function to parse and print process memory and status
void printProcessMemoryStatus(int pid) {
    // Construct the path to the status file
    std::string statusPath = "/proc/" + std::to_string(pid) + "/status";

    // Open the status file
    std::ifstream statusFile(statusPath);
    if (!statusFile.is_open()) {
        std::cerr << "Error: Could not open " << statusPath << std::endl;
        return;
    }

    // Read the content of the status file
    std::string line;
    while (getline(statusFile, line)) {
        // Extract and print relevant information
        if (line.compare(0, 5, "Name:") == 0 || line.compare(0, 4, "Pid:") == 0 ||
            line.compare(0, 6, "State:") == 0 || line.compare(0, 5, "PPid:") == 0 ||
            //line.compare(0, 4, "Uid:") == 0 || line.compare(0, 4, "Gid:") == 0 ||
            //line.compare(0, 6, "VmRSS:") == 0 || line.compare(0, 7, "VmSwap:") == 0) {
            line.compare(0, 2, "Vm") == 0 || line.compare(0, 3, "Rss") == 0) {
            std::cout << line << std::endl;
        }
    }

    // Close the status file
    statusFile.close();
}


int main(int argc, char* argv[]) {
    // Check if a process ID is provided as a command-line argument
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <pid>" << std::endl;
        return 1;
    }

    // Convert the process ID from string to integer
    int pid;
    try {
        pid = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid process ID" << std::endl;
        return 1;
    }

    // Print memory usage statistics for the specified process
    printMemoryUsage(pid);

    std::cout << std::endl;

    printProcessStats(pid);

    std::cout << std::endl;

    printProcessMemoryStatus(pid);

    return 0;
}
