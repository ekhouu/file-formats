#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET 256 // ascii

void count_freq(FILE *fp, size_t freq[ALPHABET]) {
  // set entire mem block to 0
  memset(freq, 0, sizeof(size_t) * ALPHABET);

  int ch;
  // fgetc -> get next char
  while ((ch = fgetc(fp)) != EOF) {
    freq[(unsigned char)ch]++;
  }
}

/* Compression Procedure
 * To compress a file, you need a table of bit encodings, constructed from a
 * coding tree (do look at jpeg/huffman.md!)
 *
 * The general outline of our procedure is as follows:
 *      1. Build a table of per-character encodings (can take either a provided
 *         table or define a seperate function for Huffman coding trees)
 *      2. Read in the plain file and process char-by-char.
 *          0. Find the bit sequence from the table
 *          1. Write out the corresponding sequence to the compressed file
 *  We will add some more details like the file header and EOD information.
 * */

struct TreeNode {
  int weight;
  int value;
  struct TreeNode *left, *right;
};

struct MinHeap {
  struct TreeNode **data; // arr of node ptrs
  size_t size;
  size_t capacity;
};

// CALL WITH & BC U WANT TO USE ADDRESS
void nodeSwap(struct TreeNode **a, struct TreeNode **b) {
  // swap POINTERS
  struct TreeNode *res = *a;
  *a = *b;
  *b = res;
}

int heapIns(struct MinHeap *self, struct TreeNode *node) {
  // heap data is array
  // add at end of array [1,2,3,X]
  // X<3, swap X & 3, [1,2,X,3]
  // X<2, swap X & 2, [1,X,2,3]
  // X<1, swap X & 1, [X,1,2,3]
  if (!node)
    return 1;

  struct TreeNode **data = self->data;
  data[self->size] = node;

  for (size_t i = self->size; i > 0; --i) {
    if (data[i]->weight < data[i - 1]->weight) {
      nodeSwap(&data[i], &data[i - 1]);
    } else
      break;
  }

  self->size++;

  return 0;
}

struct TreeNode *popMin(struct MinHeap *self) {
  if (self->size == 0)
    return NULL;

  struct TreeNode *min = self->data[0];
  self->size--;

  if (self->size > 0) {
    self->data[0] = self->data[self->size];

    size_t i = 0;

    while (1) {
      size_t l = 2 * i + 1;
      size_t r = 2 * i + 2;
      size_t smallest = i;

      if (l < self->size &&
          self->data[l]->weight < self->data[smallest]->weight)
        smallest = l;

      if (r < self->size &&
          self->data[r]->weight < self->data[smallest]->weight)
        smallest = r;

      if (smallest == i)
        break;

      nodeSwap(&self->data[i], &self->data[smallest]);
      i = smallest;
    }
  }
  return min;
}

/* Building the Table
 * We need to use the greedy Huffman algorithm (again, consult jpeg/huffman.md)
 * to generate a Huffman coding tree. Then, we need to loop through every
 * root-leaf path to generate the table of optimal pairs based on each path.
 * */

struct TreeNode *mergeTwo(struct TreeNode *t1, struct TreeNode *t2) {
  struct TreeNode *newHead = malloc(sizeof(struct TreeNode));
  newHead->left = newHead->right = NULL;

  if (!newHead) {
    return NULL;
  }

  newHead->weight = t1->weight + t2->weight;
  newHead->left = t1;
  newHead->right = t2;

  return newHead;
}

struct Code {
  uint8_t bits[(ALPHABET + 7) / 8];
  uint16_t nbits;
};

// will use lens later
// prolly
void visit(struct TreeNode *node, uint8_t *path, size_t depth,
           struct Code table[ALPHABET], uint8_t lens[ALPHABET]) {
  if (!node)
    return;

  if (!node->left && !node->right) {
    int sym = node->value; // 0 .. ALPHABET-1
    struct Code *out = &table[sym];

    memset(out->bits, 0, sizeof out->bits);
    out->nbits = (uint16_t)depth;
    lens[sym] = (uint8_t)depth;

    for (size_t i = 0; i < depth; ++i) {
      if (path[i]) {
        size_t byte = i >> 3; // i / 8
        size_t bit = i & 7;   // i % 8
        out->bits[byte] |= (uint8_t)(1u << bit);
      }
    }
    return;
  }

  if (node->left) {
    path[depth] = 0;
    visit(node->left, path, depth + 1, table, lens);
  }

  if (node->right) {
    path[depth] = 1;
    visit(node->right, path, depth + 1, table, lens);
  }
}

int build_codes(struct MinHeap *heap, struct Code table[ALPHABET],
                uint8_t lens[ALPHABET]) {
  if (!heap || heap->size == 0)
    return 1;

  struct TreeNode *root = heap->data[0];

  if (!root->left && !root->right) {
    int sym = root->value;

    memset(table[sym].bits, 0, sizeof table[sym].bits);
    table[sym].nbits = 1;
    lens[sym] = 1;
    return 0;
  }

  uint8_t path[ALPHABET];
  memset(lens, 0, ALPHABET);

  visit(root, path, 0, table, lens);

  return 0;
}

// TODO: add writing to a file w/ a proper header

/* File Header
 * Since we are encoding our data via a specific table, we want to make sure
 * that when someone reads the file, they have access to the same information.
 * If they don't, our file is literally just gibberish.
 *
 * We have a few main ways to do this, though there's obviously more:
 *  - Store the char counts at the start (prolly for non-zero characters; best
 * to store a pair)
 *  - Use a standard character frequency so that we don't have to generate a new
 * tree for every case
 *  - Store the tree at the file start
 *        - Do a pre-order traversal and write every node visited.
 *        - Diff leaf notes from internal or non-leaf nodes
 *        - If node is leaf, write the char stored ~ if not, internal node
 * */

/* ---------------------------
 * |      MAIN . LOGIC       |
 * --------------------------- */

int main() {
  char file_path[256];
  printf("File path? ");
  fgets(file_path, sizeof(file_path), stdin);
  printf("Reading in file %s", file_path);

  file_path[strcspn(file_path, "\n")] = '\0';

  FILE *file_ptr = fopen(file_path, "r");
  if (!file_ptr) {
    perror("Can't open file :(");
    return 1;
  }

  size_t freq[ALPHABET];
  count_freq(file_ptr, freq);

  // if need a lot of diff types (more than ascii) might have to preprocess w /
  // loop for now i can just use this
  struct MinHeap *heap = malloc(sizeof(struct MinHeap));
  heap->data = malloc(sizeof(struct TreeNode *) * (2 * ALPHABET - 1));
  heap->size = 0;
  heap->capacity = 2 * ALPHABET - 1;

  for (size_t i = 0; i < ALPHABET; ++i) {
    if (freq[i] > 0) {
      struct TreeNode *node = malloc(sizeof(struct TreeNode));
      node->left = node->right = NULL;
      node->weight = freq[i];
      node->value = i;
      heapIns(heap, node);
    }
  }

  struct TreeNode **data = heap->data;
  size_t size = heap->size;

  for (size_t i = 0; i < size; ++i) {
    printf("Weight: %d\nValue: %d\n-------\n", data[i]->weight, data[i]->value);
  }

  while (heap->size > 1) {
    struct TreeNode *a = popMin(heap);
    struct TreeNode *b = popMin(heap);
    struct TreeNode *merged = mergeTwo(a, b);
    heapIns(heap, merged);
  }

  struct Code table[ALPHABET];
  uint8_t lens[ALPHABET];
  build_codes(heap, table, lens);

  for (size_t i = 0; i < ALPHABET; ++i) {
    if (lens[i] > 0) {
      printf("Symbol %zu: len=%u, code=", i, table[i].nbits);

      for (uint16_t b = 0; b < table[i].nbits; ++b) {
        size_t byte = b >> 3; // b / 8
        size_t bit = b & 7;   // b % 8
        int bitval = (table[i].bits[byte] >> bit) & 1;
        putchar(bitval ? '1' : '0');
      }
      putchar('\n');
    }
  }

  return 0;
}
