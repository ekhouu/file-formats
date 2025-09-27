Notes on : https://web.archive.org/web/20191129105025/https://www2.cs.duke.edu/csed/poop/huff/info/

# Huffman Coding

aka Entropy Coding

IN: bytes / DCT coefficients / etc
OUT: IN but encoded with variable length codes

Symbols that are more frequent get converted w/ shorter codes, while less frequent symbols get longer-bit codes.

## ASCII to Huffman

ASCII:

- Every char = 8 bits
- thus 256 possible values ($2^8$)
- most common chars can be encoded w/ 7 bits ($2^7 = 128$)

Huffman:

- Compresses data by using fewer bits for most commons

### Coding a Message

ASCII Coding
| char | ASCII | binary |
| - | - | - |
| g | 103 | 1100111 |
| o | 111 | 1101111 |
| p | 112 | 1110000 |
| h | 104 | 1101000 |
| e | 101 | 1100101 |
| r | 114 | 1110010 |
| s | 115 | 1110011 |
| space | 32 | 1000000 |

Right now, we use 104 bits:

```sh
1100111 1101111 1100000 1100111 1101111 1100000 1100111 1101111 1101000 1100101 1110010 1110011
```

But since there are only 8 characters, we can actually just use 3-bit encoding instead of 8-bit encoding ($2^3$ = 8)

| char  | code | binary |
| ----- | ---- | ------ |
| g     | 0    | 000    |
| o     | 1    | 001    |
| p     | 2    | 010    |
| h     | 3    | 011    |
| e     | 4    | 100    |
| r     | 5    | 101    |
| s     | 6    | 110    |
| space | 7    | 111    |

Then, we can rewrite as:

```sh
000 001 111 000 001 111 000 001 010 011 100 101 110 111
```

This is only 39 bits; we've saved 104-39=65 bits.

We could have saved even more space if we used less than 3 bits to encode letters like g and o (very common) then less letters for more uncommon letters.

### Towards a Coding Tree

https://web.archive.org/web/20191129105025im_/https://www2.cs.duke.edu/csed/poop/huff/info/asciitree.jpg

We can use a tree (more like a trie) to store character at different leaves.

ASCII codes for any char/leaf are obtained by going on a path based on the 1s and 0s in the input stream.

'a' = 97 ASCII , 1100001 binary

0. Start at root
1. Go right (to one) pass 100001
2. Go right (to one) pass 00001
3. Go left (to zero) pass 0001
4. Go left (to zero) pass 001
5. Go left (to zero) pass 01
6. Go left (to zero) pass 1
7. Go right (to one) end

You can use this to figure out the coding of any leaf.

If you use a different tree, you can get a different coding. We can turn the example from earlier ('go go gophers') into a new encoding & trie.

| char | binary |
| ---- | ------ |
| 'g'  | 10     |
| 'o'  | 11     |
| 'p'  | 0100   |
| 'h'  | 0101   |
| 'e'  | 0110   |
| 'r'  | 0111   |
| 's'  | 000    |
| ' '  | 001    |

Then, we assemble our trie to the same way as the describe trie from before.
_NOTE_ : should later implement simple huffman encoding in jpeg/huffman/compress.c
_NOTE 2_ : look at the codes and realize that we need to have a prefix for every "class" of codes, so we don't misread!

### Encoding Logic

We assume every character has a weight equal to the number of times it occurs.

In "go go gophers", g and o have weight 3, space has weight 2, all others have weight 1.

In Huffman encoding:

- All characters have an associated weight, which is the number of times they appear in the file
- We are building a single tree from a bunch of single-node trees with the character's weight

Huffman's algorithm:

1. Start with a "forest" of one-node trees, with their values being character weights
2. Choose two of the smallest-weight trees, $T_1$ and $T_2$.
3. Make a new tree whose root is equal to $T_1 + T_2$ and that has $T_1$ on L and $T_2$ on R.
4. Repeat 2-3 until there is only one tree.
5. The last tree left is the optimal tree.

```sh
('g',3) , ('o',3) , (' ',2), ('e',1) , ('s',1) , ('h',1) , ('p',1) , ('r',1)
                                                            ^ T1       ^ T2
('g',3) , ('o',3) , (' ',2), ('e',1) , ('s',1) , ('h',1) ,     2
                                                              / \
                                                       ('p',1) ('r',1)
('g',3) , ('o',3) , (' ',2), ('e',1) ,     2        ,          2
                                          / \                 / \
                                    ('s',1) ('h',1)     ('p',1) ('r',1)
('g',3) , ('o',3) ,       3          ,     2        ,          2
                         / \             '/ '\                 / \
                   (' ',2) ('e',1)  ('s',1) ('h',1)     ('p',1) ('r',1)
('g',3) , ('o',3) ,       3          ,               4
                                                2   / \      2
                         / \                 '/ '\          / \
                   (' ',2) ('e',1)      ('s',1) ('h',1)('p',1) ('r',1)
      6                 3                           4
     / \               / \                       2 /  \  2
('g',3)('o',3)  (' ',2)  ('e',1)                /\       /\
                                           's'1  'h'1 'p'1 'r'1
   / 6 \                        3       / 7 \       4
'g'3  'o'3                 ' '2 /\ 'e'1'        2 /  \    2
                                              /  \      /  \
                                            's'1 'h'1 'p'1 'r'1
```

# JPEG File

- Up to 4 Huffman tables that define mapping of VLCs that take btwn 1-16 bits and code values (8bit byte)
- Normally ur supposed to count how frequently a certain symbol (DCT code word) appears in image
- But most ppl just use JPEG standard Huffman tables (tho some encoders allow u 2 optimize)
