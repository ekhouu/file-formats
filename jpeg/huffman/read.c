/* READING */

/* Pseudo-EOF
 * When the internal buffer is full, the output is written to the disk. This
 * means that because we have weird units in a lot of files, we will encounter
 * some weird buffering issues if we try to read through bit by bit.
 *
 * We must:
 *  - Account for padding (if we have 11 bits, 5 extra bits will be written to
 * make it a multiple of 8)
 *  - Use a pseudo-EOF so that we don't read the extra characters
 * */

void main() {
  int bits;

  // root -> tree root from header data
  struct TreeNode current = root;

  while (true) {
    int bits = input.readBits(1);
    if (bits == -1) {
      throw new HuffException("bad input, no PSEUDO_EOF");
    } else {

      // use the zero/one value of the bit to reverse the tree
      // if a leaf is reached, decode and print UNLESS
      // char is pseudo eof, then stop

      if (bits == 0)
        current = current.left; // read a 0 go left
      else {
        if (!(current.left) && !(current.right)) { // no left or right -> leaf
          if (/* leaf - node stores pseudo - eof char*/)
            break;
          else {
            output.writeBits(/* leaf node value*/);
            current = root;
          }
        }
      }
    }
  }
}
