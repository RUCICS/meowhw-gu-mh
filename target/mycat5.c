// mycat5.c
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define OPTIMAL_MULTIPLIER 8  // 4KB * 8 = 32KB 最优缓冲区

// 检查是否为2的幂
int is_power_of_two(size_t x) {
    return x && !(x & (x - 1));
}

// 获取推荐的I/O缓冲区大小
size_t io_blocksize(int fd) {
    struct stat st;
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096;

    size_t block_size = page_size;
    if (fstat(fd, &st) == 0 && is_power_of_two((size_t)st.st_blksize)) {
        block_size = (size_t)st.st_blksize;
    }

    return OPTIMAL_MULTIPLIER * block_size; // 32KB
}

// 分配页对齐的缓冲区
char* align_alloc(size_t size) {
    void* ptr = NULL;
    size_t alignment = sysconf(_SC_PAGESIZE);
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return (char*)ptr;
}

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

    size_t buf_size = io_blocksize(fd);
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
