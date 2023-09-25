#ifndef CONNECTED_COMPONENT_COUNT_H_
#define CONNECTED_COMPONENT_COUNT_H_

//#include <stdio.h>
#include "Arduino.h"

void dfs(int y, int x, int dim, int current_label, int **label, uint8_t **m);
int find_components(int dim, uint8_t **m);

#endif //CONNECTED_COMPONENT_COUNT_H_
