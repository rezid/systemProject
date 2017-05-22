#ifdef DEBUG
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT(...)
#endif

#include "conduct.h"


struct element_t {
    char element;
    pthread_mutex_t mutex;
    pthread_cond_t cond_empty;
    pthread_cond_t cond_full;
};

struct conduct_size_t {
    size_t size;
    pthread_mutex_t mutex;
};

struct conduct_bool_t {
    bool value;
    pthread_mutex_t mutex;
};

const char * first_extention = ".conduct";
const char * second_extention = ".dataConduct";
const size_t buffer_size = 1024;

struct conduct *conduct_create(const char *name, size_t atomic_length, size_t global_length)
{
    // Error
    if (name == NULL)
        return NULL;
    if (global_length < atomic_length)
        return NULL;

    // create the path(s) files name
    DEBUG_PRINT("---------------------Create the path(s) files name---------------------\n");
    char path_conduct_file[buffer_size], path_data_file[buffer_size];
    if (create_file_name(name, path_conduct_file, path_data_file) == -1){
        perror("create_file_name");
        return NULL;
    }
    DEBUG_PRINT("(DEBUG)The conduct file name is: %s\n", path_conduct_file);
    DEBUG_PRINT("(DEBUG)The data file name is: %s\n", path_data_file);
    DEBUG_PRINT("(DEBUG)done\n");

    // create files
    DEBUG_PRINT("---------------------Create files---------------------\n");
    int conduct_fd = open (path_conduct_file, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (conduct_fd == -1){
        perror("open(conduct_file)");
        return NULL;
    }
    int data_fd = open (path_data_file, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (data_fd == -1){
        perror("open(data_file)");
        return NULL;
    }
    DEBUG_PRINT("(DEBUG)done\n");

    // set the files size
    DEBUG_PRINT("---------------------Set the files size---------------------\n");
    if (ftruncate(conduct_fd, sizeof(struct conduct_size_t) * 4 + 2 * sizeof(struct conduct_bool_t)) == -1) {
        perror("ftruncate(conduct_file)");
        return NULL;
    }
    if (ftruncate(data_fd, global_length * sizeof(struct element_t)) == -1) {
        perror("ftruncate(data_file)");
        return NULL;
    }
    DEBUG_PRINT("(DEBUG)done\n");

    // get files status
    DEBUG_PRINT("---------------------Get files status---------------------\n"); 
    struct stat conduct_file_status, data_file_status;
    if (fstat(conduct_fd, &conduct_file_status) == -1){
        perror("fstat(conduct_file)");
        return NULL;
    }
    if (fstat(data_fd, &data_file_status) == -1){
        perror("fstat(data_file)");
        return NULL;
    }
    DEBUG_PRINT("(DEBUG)conduct file size: %d\n", conduct_file_status.st_size);
    DEBUG_PRINT("(DEBUG)data file size: %d\n", data_file_status.st_size);
    DEBUG_PRINT("(DEBUG)done\n");
   

    // map files to memory
     DEBUG_PRINT("---------------------Map files to memory---------------------\n");
    char *conduct_map_ptr, *data_map_ptr;
    conduct_map_ptr = mmap(0, conduct_file_status.st_size, PROT_WRITE, MAP_SHARED, conduct_fd, 0);
    if (conduct_map_ptr == MAP_FAILED){
        perror("mmap(conduct_file)");
        return NULL;
    }
    data_map_ptr = mmap(0, data_file_status.st_size, PROT_WRITE, MAP_SHARED, data_fd, 0);
    if (data_map_ptr == MAP_FAILED){
        perror("mmap(data_file)");
        return NULL;
    }
    DEBUG_PRINT("(DEBUG)done\n");

    // write header information to conduct file
    if (initialize_header_information(conduct_fd, atomic_length, data_file_status.st_size / sizeof(struct element_t)) == -1){
        perror("initialize_header_information");
        return NULL;
    }

    // initialize mutex and variable condition in data file
    if (initialize_data_file(data_fd, data_file_status.st_size / sizeof(struct element_t)) == -1){
        perror("initialize_header_information");
        return NULL;
    }

    // close the files
    close(conduct_fd);
    close(data_fd);
    DEBUG_PRINT("---------------------close the files---------------------\n");
    DEBUG_PRINT("(DEBUG)done\n");

    // return the conduct structure
    struct conduct * out = malloc(sizeof(struct conduct));
    out->conduct_fd = conduct_fd;
    out->data_fd = data_fd;
    out->conduct_map_ptr = conduct_map_ptr;
    out->data_map_ptr = data_map_ptr;
    return out;
}

struct conduct *conduct_open(const char *name)
{
     // Error
    if (name == NULL)
        return NULL;

    // create the path(s) files name
    DEBUG_PRINT("---------------------Create the path(s) files name---------------------\n");
    char path_conduct_file[buffer_size], path_data_file[buffer_size];
    if (create_file_name(name, path_conduct_file, path_data_file) == -1){
        perror("create_file_name");
        return NULL;
    }
    DEBUG_PRINT("(DEBUG)The conduct file name is: %s\n", path_conduct_file);
    DEBUG_PRINT("(DEBUG)The data file name is: %s\n", path_data_file);
    DEBUG_PRINT("(DEBUG)done\n");

    // open files
    DEBUG_PRINT("---------------------Open files---------------------\n");
    int conduct_fd = open (path_conduct_file, O_RDWR , S_IRUSR | S_IWUSR);
    if (conduct_fd == -1){
        perror("open(conduct_file)");
        return NULL;
    }
    int data_fd = open (path_data_file, O_RDWR, S_IRUSR | S_IWUSR);
    if (data_fd == -1){
        perror("open(data_file)");
        return NULL;
    }
    DEBUG_PRINT("(DEBUG)done\n");

    // get files status
    DEBUG_PRINT("---------------------Get files status---------------------\n"); 
    struct stat conduct_file_status, data_file_status;
    if (fstat(conduct_fd, &conduct_file_status) == -1){
        perror("fstat(conduct_file)");
        return NULL;
    }
    if (fstat(data_fd, &data_file_status) == -1){
        perror("fstat(data_file)");
        return NULL;
    }
    DEBUG_PRINT("(DEBUG)conduct file size: %d\n", conduct_file_status.st_size);
    DEBUG_PRINT("(DEBUG)data file size: %d\n", data_file_status.st_size);
    DEBUG_PRINT("(DEBUG)done\n");
   

    // map files to memory
    DEBUG_PRINT("---------------------Map files to memory---------------------\n");
    char *conduct_map_ptr, *data_map_ptr;
    conduct_map_ptr = mmap(0, conduct_file_status.st_size, PROT_WRITE, MAP_SHARED, conduct_fd, 0);
    if (conduct_map_ptr == MAP_FAILED){
        perror("mmap(conduct_file)");
        return NULL;
    }
    data_map_ptr = mmap(0, data_file_status.st_size, PROT_WRITE, MAP_SHARED, data_fd, 0);
    if (data_map_ptr == MAP_FAILED){
        perror("mmap(data_file)");
        return NULL;
    }
    DEBUG_PRINT("(DEBUG)done\n");

    // close the files
    close(conduct_fd);
    close(data_fd);
    DEBUG_PRINT("---------------------close the files---------------------\n");
    DEBUG_PRINT("(DEBUG)done\n");

    // return the conduct structure
    struct conduct * out = malloc(sizeof(struct conduct));
    out->conduct_fd = conduct_fd;
    out->data_fd = data_fd;
    out->conduct_map_ptr = conduct_map_ptr;
    out->data_map_ptr = data_map_ptr;
    return out;
}

ssize_t conduct_write(struct conduct *c, const void *buf, size_t count)
{
    // error
    if (c->conduct_map_ptr == NULL || c->data_map_ptr == NULL)
        return -1;
    if (count == 0)
        return -1;

    struct conduct_size_t * conduct_size = (struct conduct_size_t *) (c->conduct_map_ptr);
    struct conduct_bool_t * conduct_bool = (struct conduct_bool_t *) (&conduct_size[4]);
    struct element_t * element = (struct element_t *) (c->data_map_ptr);

    // cast for array subscripting
    const char * buff = (const char *) buf;

    pthread_mutex_lock(&(element[0].mutex));
    
    DEBUG_PRINT("---------------------Print mmap conduct information---------------------\n");
    DEBUG_PRINT("(DEBUG)read offset is: %d\n", conduct_size[0].size);
    DEBUG_PRINT("(DEBUG)write offset is: %d\n", conduct_size[1].size);
    DEBUG_PRINT("(DEBUG)atomic length is: %d\n", conduct_size[2].size);
    DEBUG_PRINT("(DEBUG)global length is: %d\n", conduct_size[3].size);
    DEBUG_PRINT("(DEBUG)eof is: %d\n", conduct_bool[0].value);
    DEBUG_PRINT("(DEBUG)isEmpty is: %d\n", conduct_bool[1].value);
    DEBUG_PRINT("(DEBUG)conduct free space is: %d\n", get_conduct_space(c));
    DEBUG_PRINT("(DEBUG)count: %d\n", count);
    DEBUG_PRINT("(DEBUG)done\n");
    
    //sleep(3);

    size_t space = get_conduct_space(c);
    size_t a = conduct_size[2].size;
    size_t return_value = 0;


    if (count <= a) {
        // partiel write: bloque if not count Bytes free
        while (get_conduct_space(c) <= count) {   
            if(conduct_bool[0].value == true) {
                pthread_mutex_unlock(&(element[0].mutex));
                errno = EPIPE;
                return -1;
            }
            pthread_cond_wait(&(element[0].cond_full), &(element[0].mutex));
        }

        // writing bytes
        for(int i = 0; i< count; ++i) {
            element[conduct_size[1].size + i].element = buff[i];
        }
        conduct_size[1].size += count;
        return_value = count;
        // Wakeup all
        pthread_cond_broadcast(&(element[0].cond_empty));    
    }
    else if (count > a){
        // if not count Bytes free, then write anythink >= 1 Byte
        while (get_conduct_space(c) == 0) {
            if(conduct_bool[0].value == true) {
                pthread_mutex_unlock(&(element[0].mutex));
                errno = EPIPE;
                return -1;
            }  
            pthread_cond_wait(&(element[0].cond_full), &(element[conduct_size[1].size].mutex));
        }

        // writing bytes
        for(int i = 0; i< get_conduct_space(c); ++i) {
            element[conduct_size[1].size + i].element = buff[i];
        }
        conduct_size[1].size += get_conduct_space(c);
        return_value = get_conduct_space(c);
        // Wakeup all
        pthread_cond_broadcast(&(element[0].cond_empty)); 
    }

    pthread_mutex_unlock(&(element[0].mutex));
    return return_value;
}


ssize_t conduct_read(struct conduct *c, void *buf, size_t count)
{
    // error
    if (c->conduct_map_ptr == NULL || c->data_map_ptr == NULL)
        return -1;
    if (count == 0)
        return -1;

    struct conduct_size_t * conduct_size = (struct conduct_size_t *) (c->conduct_map_ptr);
    struct conduct_bool_t * conduct_bool = (struct conduct_bool_t *) (&conduct_size[4]);
    struct element_t * element = (struct element_t *) (c->data_map_ptr);

    // cast for array subscripting
    char * buff = (const char *) buf;

    pthread_mutex_lock(&(element[0].mutex));
    
    DEBUG_PRINT("---------------------Print mmap conduct information---------------------\n");
    DEBUG_PRINT("(DEBUG)read offset is: %d\n", conduct_size[0].size);
    DEBUG_PRINT("(DEBUG)write offset is: %d\n", conduct_size[1].size);
    DEBUG_PRINT("(DEBUG)atomic length is: %d\n", conduct_size[2].size);
    DEBUG_PRINT("(DEBUG)global length is: %d\n", conduct_size[3].size);
    DEBUG_PRINT("(DEBUG)eof is: %d\n", conduct_bool[0].value);
    DEBUG_PRINT("(DEBUG)isEmpty is: %d\n", conduct_bool[1].value);
    DEBUG_PRINT("(DEBUG)conduct free space is: %d\n", get_conduct_space(c));
    DEBUG_PRINT("(DEBUG)count: %d\n", count);
    DEBUG_PRINT("(DEBUG)done\n");
    
    //sleep(3);

    size_t space = get_conduct_space(c);
    size_t a = conduct_size[2].size;
    size_t return_value = 0;

    while(get_conduct_space(c) == conduct_size[3].size) {
        if (conduct_bool[0].value == true) {
            pthread_mutex_unlock(&(element[0].mutex));
            return 0;
        }
        pthread_cond_wait(&(element[0].cond_empty), &(element[0].mutex));
    }

    if (count <= conduct_size[3].size - get_conduct_space(c)) {
        // reading bytes
        for(int i = 0; i< count; ++i) {
            buff[i] = element[conduct_size[0].size + i].element;
        }
        conduct_size[0].size += count;
        return_value = count;
        // Wakeup all
        pthread_cond_broadcast(&(element[0].cond_full));    
    }
    else {
        int number = conduct_size[3].size - get_conduct_space(c);
        // reading bytes
        for(int i = 0; i< number; ++i) {
             buff[i] = element[conduct_size[0].size + i].element;
        }
        conduct_size[0].size += number;
        return_value = number;
        // Wakeup all
        pthread_cond_broadcast(&(element[0].cond_full)); 
    }

    pthread_mutex_unlock(&(element[0].mutex));
    return return_value;
}

int conduct_write_eof(struct conduct *c)
{
     // error
    if (c->conduct_map_ptr == NULL || c->data_map_ptr == NULL)
        return -1;
 
    struct conduct_size_t * conduct_size = (struct conduct_size_t *) (c->conduct_map_ptr);
    struct conduct_bool_t * conduct_bool = (struct conduct_bool_t *) (&conduct_size[4]);
    struct element_t * element = (struct element_t *) (c->data_map_ptr);

    if (conduct_bool[1].value == true)
        return 0;

    pthread_mutex_lock(&(element[0].mutex));
    conduct_bool[0].value = true;
    pthread_mutex_unlock(&(element[0].mutex));
    return 0;
}


int create_file_name (const char *name, char * out_conduct_file_name, char * out_data_file_name) 
{
    if (out_conduct_file_name == NULL || out_data_file_name == NULL) {
        errno = ENXIO;
        return -1;
    }

    snprintf(out_conduct_file_name, buffer_size , "./%s%s", name, first_extention);
    snprintf(out_data_file_name, buffer_size, "./%s%s", name, second_extention);
} 


void conduct_close(struct conduct *conduct)
{
     // error
    if (conduct->conduct_map_ptr == NULL || conduct->data_map_ptr == NULL)
        return;
   
    struct conduct_size_t * conduct_size = (struct conduct_size_t *) (conduct->conduct_map_ptr);
    struct conduct_bool_t * conduct_bool = (struct conduct_bool_t *) (&conduct_size[4]);
    struct element_t * element = (struct element_t *) (conduct->data_map_ptr);

    munmap(conduct->conduct_map_ptr, 4 * sizeof(struct conduct_size_t) + 2 * sizeof(struct conduct_bool_t));
    munmap(conduct->data_map_ptr, conduct_size[3].size * sizeof(struct element_t));
}

void conduct_destroy(struct conduct *conduct)
{
    conduct_close(conduct);
    // TO DO : delete file
}

size_t get_conduct_space(struct conduct *c)
{
    struct conduct_size_t * conduct_size = (struct conduct_size_t *) (c->conduct_map_ptr);
    struct conduct_bool_t * conduct_bool = (struct conduct_bool_t *) (&conduct_size[4]);

    bool data_isEmpty = conduct_bool[1].value;
    size_t read_offset = conduct_size[0].size;
    size_t write_offset = conduct_size[1].size;
    size_t atomic_length = conduct_size[2].size;
    size_t global_length = conduct_size[3].size;

    if (read_offset == write_offset) {
        if (data_isEmpty)
            return global_length;
        else
            return 0;
    }
    else 
        return (global_length + (read_offset - write_offset))%global_length;
}

ssize_t initialize_header_information(int conduct_fd, size_t a, size_t real_global_length) 
{
    DEBUG_PRINT("------Create header information------\n");
    int err;
    pthread_mutexattr_t mutex_attr;
    err = pthread_mutexattr_init(&mutex_attr); if (err) return -1;
    err = pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED); if (err) return -1;
    struct conduct_size_t read_offset = {.size = 1};
    struct conduct_size_t write_offset = {.size = 1};
    struct conduct_size_t atomic_length = {.size = a};
    struct conduct_size_t global_length = {.size = real_global_length};
    struct conduct_bool_t eof_insered = {.value = false};
    struct conduct_bool_t data_isEMpty = {.value = true};
    err = pthread_mutex_init(&(read_offset.mutex), &mutex_attr); if (err) return -1;
    err = pthread_mutex_init(&(write_offset.mutex), &mutex_attr); if (err) return -1;
    err = pthread_mutex_init(&(atomic_length.mutex), &mutex_attr); if (err) return -1;
    err = pthread_mutex_init(&(global_length.mutex), &mutex_attr); if (err) return -1;
    err = pthread_mutex_init(&(eof_insered.mutex), &mutex_attr); if (err) return -1;
    err = pthread_mutex_init(&(data_isEMpty.mutex), &mutex_attr); if (err) return -1;
    DEBUG_PRINT("(DEBUG)done\n");

    DEBUG_PRINT("------Writing header information to file------\n");
    size_t return_value;
    return_value = write(conduct_fd, &read_offset, sizeof(read_offset));
    if (return_value == -1){
        perror("write(read_offset)");
        return -1;
    }
    DEBUG_PRINT("(DEBUG)write 'read_offset': %d\n", read_offset.size);
    return_value = write(conduct_fd, &write_offset, sizeof(write_offset));
    if (return_value == -1){
        perror("write(write_offset)");
        return -1;
    }
    DEBUG_PRINT("(DEBUG)write 'write_offset': %d\n", write_offset.size);
    return_value = write(conduct_fd, &atomic_length, sizeof(atomic_length));
    if (return_value == -1){
        perror("write(atomic_length)");
        return -1;
    }
    DEBUG_PRINT("(DEBUG)write 'atomic_length': %d\n", atomic_length.size);
    return_value = write(conduct_fd, &global_length, sizeof(global_length));
    if (return_value == -1){
        perror("write(global_length)");
        return -1;
    }
    DEBUG_PRINT("(DEBUG)write 'global_length: %d\n", global_length.size);
    return_value = write(conduct_fd, &eof_insered, sizeof(eof_insered));
    if (return_value == -1){
        perror("write(eof_insered)");
        return -1;
    }
    DEBUG_PRINT("(DEBUG)write 'eof_insered': %d\n", eof_insered.value);
    return_value = write(conduct_fd, &data_isEMpty, sizeof(data_isEMpty));
    if (return_value == -1){
        perror("write(data_isEMpty)");
        return -1;
    }
    DEBUG_PRINT("(DEBUG)write 'data_isEMpty': %d\n", data_isEMpty.value);
    DEBUG_PRINT("(DEBUG)done\n");
    return 0;
}

ssize_t initialize_data_file(int data_fd, size_t real_global_length)
{
    DEBUG_PRINT("------Create mutex and condition varriable------\n");
    int err;
    for(int i = 0; i < real_global_length; ++i) {
        pthread_mutexattr_t mutex_attr;
        pthread_condattr_t cond_attr;
        err = pthread_mutexattr_init(&mutex_attr); if (err) return -1;
        err = pthread_condattr_init(&cond_attr); if (err) return -1;
        err = pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED); if (err) return -1;
        err = pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED); if (err) return -1;
        struct element_t element = {.element = 0};
        err = pthread_mutex_init(&(element.mutex), &mutex_attr); if (err) return -1;
        err = pthread_cond_init(&(element.cond_empty), &cond_attr); if (err) return -1;
        err = pthread_cond_init(&(element.cond_full), &cond_attr); if (err) return -1;
        size_t return_value;
        return_value = write(data_fd, &element, sizeof(element));
        if (return_value == -1){
            perror("write(elements)");
            return -1;
        }
    }
    DEBUG_PRINT("(DEBUG)done\n");
    return 0;
}



