#include <os.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>

// --- file io stuff ---

FILE *stdin = NULL;
FILE *stdout = NULL;
FILE *stderr = NULL;

FILE *fopen(const char *path, const char *mode) {
    return (FILE *)os_open_file(path);
}

int fclose(FILE *stream) {
    return os_close_file((int32_t)stream);
}

int fseek(FILE *stream, long offset, int whence) {
    if (whence == SEEK_SET) {
        os_set_file_offset((int32_t)stream, offset);
        return 0;
    }

    if (whence == SEEK_CUR) {
        int32_t cur_offset = (int32_t) os_get_file_offset((int32_t) stream);

        os_set_file_offset((int32_t)stream, cur_offset + offset);
        return 0;
    }

    if (whence == SEEK_END) {
        int32_t size = (int32_t) os_get_file_size((int32_t) stream);

        os_set_file_offset((int32_t)stream, size + offset);
        return 0;
    }

    os_print("fseek uhhhh");
    os_exit();
}

long ftell(FILE *stream) {
    return os_get_file_offset((int32_t) stream);
}

int fflush(FILE *stream) {
    os_print("fflush");
    return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return os_read_file((int32_t) stream, (char*) ptr, size * nmemb);
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return os_write_file((int32_t) stream, (const char*) ptr, size * nmemb);
}

int mkdir(const char *path, uint32_t mode) {
    os_print("mkdir");
    return 0;
}

int remove(const char *pathname) {
    os_print("remove");
    os_exit();
}

int rename(const char *oldpath, const char *newpath) {
    os_print("rename");
    os_exit();
}

// --- other stuff ---

void exit(int status) {
    os_exit();
}

int system(const char *command) {
    os_printf("tried to system(): %s", command);
    return -1;
}

int puts(const char *s) {
    os_print(s);
    return 0;
}

int fputc(int c, FILE *stream) {
    //os_printf("tried to fputc(): %u %u", c, (uint32_t) stream);
    return 0;
}

#define isdigit(c) (c >= '0' && c <= '9')

double atof(const char *s) {
    // This function stolen from either Rolf Neugebauer or Andrew Tolmach.
    // Probably Rolf.
    double a = 0.0;
    int e = 0;
    int c;
    while ((c = *s++) != '\0' && isdigit(c)) {
        a = a * 10.0 + (c - '0');
    }
    if (c == '.') {
        while ((c = *s++) != '\0' && isdigit(c)) {
            a = a * 10.0 + (c - '0');
            e = e - 1;
        }
    }
    if (c == 'e' || c == 'E') {
        int sign = 1;
        int i = 0;
        c = *s++;
        if (c == '+')
            c = *s++;
        else if (c == '-') {
            c = *s++;
            sign = -1;
        }
        while (isdigit(c)) {
            i = i * 10 + (c - '0');
            c = *s++;
        }
        e += i * sign;
    }
    while (e > 0) {
        a *= 10.0;
        e--;
    }
    while (e < 0) {
        a *= 0.1;
        e++;
    }
    return a;
}
