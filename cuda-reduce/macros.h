#ifndef __MACROS_H__
#define __MACROS_H__

#include <cinttypes>
#include <string>

int32_t new_matrix_from_file(std::string filename, int32_t* dest);

int32_t minT(int32_t a, int32_t b);

void print_matrices(int32_t *list_m, int32_t num_m);

#endif
