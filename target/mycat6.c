// mycat6.c
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>

// 获取最佳缓冲区大小（根据实验结果）
size_t io_blocksize(void) {
    // 之前实验得出最佳缓冲区大小为 32768 字节
    return 32768;
}

// 分配对齐到内存页的缓冲区
char* align_alloc(size_t size) {
    void* ptr = NULL;
    size_t alignment = sysconf(_SC_PAGESIZE);
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return (char*)ptr;
}

// 释放缓冲区
void align_free(void* ptr) {
    free(ptr);
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

    // 使用 fadvise 提示顺序访问
    if (posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL) != 0) {
        perror("posix_fadvise failed");
        // 注意：fadvise 是优化提示，失败也不致命，可继续执行
    }

    size_t buf_size = io_blocksize();
    char *buffer = align_alloc(buf_size);
    if (!buffer) {
        perror("align_alloc failed");
        close(fd);
        return EXIT_FAILURE;
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, buffer, buf_size)) != 0) {
        if (bytes_read == -1) {
            if (errno == EINTR) continue;
            perror("read failed");
            align_free(buffer);
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
                align_free(buffer);
                close(fd);
                return EXIT_FAILURE;
            }
            bytes_remaining -= bytes_written;
            buf_ptr += bytes_written;
        }
    }

    align_free(buffer);
    if (close(fd) == -1) {
        perror("close failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
