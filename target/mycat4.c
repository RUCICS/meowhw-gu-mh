// mycat4.c
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>

// 判断一个数是否是 2 的幂次
int is_power_of_two(size_t x) {
    return x && ((x & (x - 1)) == 0);
}

// 获取缓冲区大小：取内存页大小与文件系统推荐块大小的最大值
size_t io_blocksize(int fd) {
    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) page_size = 4096; //4KB

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat failed, falling back to page size");
        return (size_t)page_size;
    }

    size_t fs_block = st.st_blksize;

    // 检查文件系统块大小是否是 2 的幂次
    if (!is_power_of_two(fs_block)) {
        fs_block = (size_t)page_size;
    }

    // 返回内存页大小与文件系统推荐块较大的那个作为缓冲区大小
    return (fs_block > (size_t)page_size) ? fs_block : (size_t)page_size;
}

// 分配对齐到内存页的缓冲区
char* align_alloc(size_t size) {
    void* ptr = NULL;
    size_t alignment = size; // 对齐到缓冲区大小（也满足页对齐）
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return (char*)ptr;
}

// 释放由 align_alloc 分配的内存
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
