//basado en https://stackoverflow.com/questions/14465297/connected-component-labeling-implementation

/*
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
*/
#include "connected_component_count.h"

// matrix dimensions
/*
int const width = 6;
int const height = 5;
*/

// the input matrix
/*
int m[height][width] = {{0,0,1,0,0,0},
                        {0,0,0,0,0,0},
                        {0,0,1,1,0,0},
                        {0,0,1,0,0,0},
                        {0,0,1,0,0,1}};
*/

void dfs(int y, int x, int dim, int current_label, int **label, uint8_t **m) {
  if (x < 0 || x == dim) return; // out of bounds
  if (y < 0 || y == dim) return; // out of bounds
  if (label[y][x] || !m[y][x]) return; // already labeled or not marked with 1 in m
  
  // mark the current cell
  label[y][x] = current_label;
  
  // direction vectors
  const int dx[] = {+1, 0, -1, 0};
  const int dy[] = {0, +1, 0, -1};

  // recursively mark the neighbors
  for (int direction = 0; direction < 4; ++direction)
    dfs(y + dy[direction], x + dx[direction], dim, current_label, label, m);
}

// se asumen matrices cuadradas, que son las unicas utilizadas como output
int find_components(int dim, uint8_t **m) {

  int component = 0;

  int i, j; // iteradores
  // the labels, 0 means unlabeled
  int **label ;
  label = new int*[dim];
  for (i = 0; i < dim; i++){
    label[i] = new int[dim];
  }

  for (i = 0; i < dim; ++i) {
    for (j = 0; j < dim; ++j){
      if (label[i][j]==0 && m[i][j]==1) dfs(i, j, dim, ++component, label, m);
    }
  }
  return component;
}


/*
* Imprimir matriz
std::cout << std::endl;
for (int i = 0; i < height; ++i) {
  for (int j = 0; j < width; ++j){ 
    std::cout << label[i][j] << " ";
  }
  std::cout << std::endl;
}*/
