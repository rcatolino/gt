#ifndef SERIAL_H_
#define SERIAL_H_

//Utilities to serialize entries and candidate under the form :
//  dirname [path score ]+ \n
//One entry per line allowed, empty lines and lines beginning with # are ignored.
#include "datalist.h"

int read_next_entry();
void dump_entry(const struct entry * to_dump);
int init_serial();
int end_serial();

#endif
