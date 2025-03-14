//
// Created by m_luc on 14.03.2025.
//

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void copy_file(const char* src, const char* dest) {
    if(strcmp(src,dest) != 0) {
        int src_fd = open(src, O_RDONLY);
        if (src_fd == -1) {
            std::cerr << "Error opening source file: " << src << "\n";
            return;
        }

        int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (dest_fd == -1) {
            std::cerr << "Error opening destination file: " << dest << "\n";
            close(src_fd);
            return;
        }

        const size_t buffer_size = 1024;
        char buffer[buffer_size];
        size_t bytes_read;

        while ((bytes_read = read(src_fd, buffer, buffer_size)) > 0) {
            write(dest_fd, buffer, bytes_read);
        }

        close(src_fd);
        close(dest_fd);
    } else std::cout << "Wrong name of copy file\n";
}

void move_file(const char* src, const char* dest) {
    if (rename(src, dest) == -1) {
        std::cerr << "Error moving file: " << src << " to " << dest << "\n";
    }
}

void get_file_info(const char* path) {
    struct stat st;
    if (stat(path, &st) == -1) {
        std::cerr << "Error getting file info: " << path << "\n";
        return;
    }

    std::cout << "Permissions: " << std::oct << (st.st_mode & 0777) << "\n";
    std::cout << "File size: " << st.st_size << " bytes\n";
    std::cout << "Last modified: " << ctime(&st.st_mtime);
}

void change_permissions(const char* path, mode_t mode) {
    if (chmod(path, mode) == -1) {
        std::cerr << "Error changing permissions: " << path << "\n";
    }
}

void show_help(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --help              Show this help message\n";
    std::cout << "  --copy <src> <dest> Copy a file\n";
    std::cout << "  --move <src> <dest> Move a file\n";
    std::cout << "  --info <file>       Get file info\n";
    std::cout << "  --chmod <file> <mode> Change file permissions\n";
}

void interactive_work(char choise){
    std::string src_inp;
    std::string dest_inp;
    std::cout << "Input src file path: ";
    std::cin >> src_inp;
    const char *src = src_inp.c_str();

    if (choise == '1'){
        std::cout << "Input copy file path: ";
        std::cin >> dest_inp;
        const char *dest = dest_inp.c_str();
        copy_file(src, dest);
    } else if(choise == '2'){
        std::cout << "Input move file path: ";
        std::cin >> dest_inp;
        const char *dest = dest_inp.c_str();
        move_file(src, dest);
    } else if(choise == '3'){
        get_file_info(src);
    } else if(choise == '4'){
        std::cout << "Input file permissions in num format (owner)(group)(others)\nEach digit is a sum of:\n1 - execution\n2 - wright\n4 - read";
        mode_t mode;
        std::cin >> std::oct >> mode;
        change_permissions(src, mode);
    } else if(choise == '5') return;
    else std::cout << "Wrong input\n";
}

int unix_file_work(int argc, char* argv[]) {
    if (argc == 1) {
        std::cout << "Select an action:\n";
        std::cout << "1. Copy file\n";
        std::cout << "2. Move file\n";
        std::cout << "3. Get file info\n";
        std::cout << "4. Change file permissions\n";
        std::cout << "5. Exit\n";

        char choice;
        std::cin >> choice;
        interactive_work(choice);
    } else if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0) {
            show_help(argv[0]);
        } else if (strcmp(argv[1], "--copy") == 0 && argc == 4) {
            copy_file(argv[2], argv[3]);
        } else if (strcmp(argv[1], "--move") == 0 && argc == 4) {
            move_file(argv[2], argv[3]);
        } else if (strcmp(argv[1], "--info") == 0 && argc == 3) {
            get_file_info(argv[2]);
        } else if (strcmp(argv[1], "--chmod") == 0 && argc == 4) {
            change_permissions(argv[2], std::stoi(argv[3], nullptr, 8));
        } else {
            std::cerr << "Invalid arguments. Use --help for usage.\n";
        }
    }

    return 0;
}