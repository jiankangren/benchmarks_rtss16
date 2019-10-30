#!/bin/bash
# For base case size of 32*32
echo 
echo "Size n = 1024"
~/cilktools-linux-004421/bin/cilkview ./lu_cilk_cv 1 n 1024

echo 
echo "Size n = 2048"
~/cilktools-linux-004421/bin/cilkview ./lu_cilk_cv 1 n 2048

echo 
echo "Size n = 4096"
~/cilktools-linux-004421/bin/cilkview ./lu_cilk_cv 1 n 4096