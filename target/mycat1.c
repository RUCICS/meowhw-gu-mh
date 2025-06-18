#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // 检查参数数量
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // 打开文件
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        exit(EXIT_FAILURE);
    }

    char c;
    ssize_t bytes_read;
    
    // 每次读取和输出1个字符
    while ((bytes_read = read(fd, &c, 1)) > 0) {
        if (write(STDOUT_FILENO, &c, 1) != 1) {
            perror("write failed");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    // 检查是否有读取错误
    if (bytes_read == -1) {
        perror("read failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // 关闭文件
    if (close(fd) == -1) {
        perror("close failed");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}