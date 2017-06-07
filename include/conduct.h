#include <sys/types.h>
#include <stddef.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <stdbool.h>
#include <pthread.h>


struct conduct
{
    bool is_named;
    char *name;
    char *conduct_map_ptr;
    char *data_map_ptr;    
};

extern struct conduct *conduct_create(const char *name, size_t a, size_t c);
struct conduct *conduct_open(const char *name);
ssize_t conduct_read(struct conduct *c, void *buf, size_t count);
ssize_t conduct_write(struct conduct *c, const void *buf, size_t count);
int conduct_write_eof(struct conduct *c);
void conduct_close(struct conduct *conduct);
void conduct_destroy(struct conduct *conduct);




// helper function
int create_file_name (const char *name, char * out_conduct_file_name, char * out_data_file_name);
size_t get_conduct_space(struct conduct *c);

ssize_t initialize_header_information(char* conduct_map_ptr, size_t atomic_length, size_t real_global_length);
ssize_t initialize_data_file(char* data_map_ptr, size_t real_global_length);

size_t get_read_offset(char *conduct_map_ptr);
size_t get_write_offset(char *conduct_map_ptr);
size_t get_atomic_length(char *conduct_map_ptr);
size_t get_global_length(char *conduct_map_ptr);
bool get_eof_insered (char *conduct_map_ptr);
bool get_data_isEMpty(char *conduct_map_ptr);