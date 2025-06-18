#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// 获取最佳I/O块大小（系统内存页大小）
size_t io_blocksize(void) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size > 0) return (size_t)page_size;
    
    // 获取失败时使用安全默认值
    fprintf(stderr, "Warning: sysconf failed, using 4096-byte buffer\n");
    return 4096;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        return EXIT_FAILURE;
    }

    // 动态获取并分配缓冲区
    const size_t buf_size = io_blocksize();
    char *buffer = malloc(buf_size);
    if (!buffer) {
        perror("malloc failed");
        close(fd);
        return EXIT_FAILURE;
    }

    ssize_t bytes_read;
    while (bytes_read = read(fd, buffer, buf_size)) {//循环读
        if (bytes_read == -1) {
            if (errno == EINTR) continue;  // 处理信号中断
            perror("read failed");
            free(buffer);
            close(fd);
            return EXIT_FAILURE;
        }

        char *buf_ptr = buffer;
        ssize_t bytes_remaining = bytes_read;
        while (bytes_remaining > 0) {
            ssize_t bytes_written = write(STDOUT_FILENO, buf_ptr, bytes_remaining);
            if (bytes_written <= 0) {
                if (bytes_written == -1 && errno == EINTR) continue;
                perror("write failed");
                free(buffer);
                close(fd);
                return EXIT_FAILURE;
            }
            bytes_remaining -= bytes_written;
            buf_ptr += bytes_written;
        }
    }

    free(buffer);
    if (close(fd) == -1) {
        perror("close failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}