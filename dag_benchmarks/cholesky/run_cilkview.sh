#!/bin/bash
# For base case size of 32*32
echo 
echo "Size n = 1500"
~/cilktools-linux-004421/bin/cilkview ./ch_cilk_cv 1 n 1500

echo
echo "Size n = 3000"
~/cilktools-linux-004421/bin/cilkview ./ch_cilk_cv 1 n 3000

echo 
echo "Size n = 4000"
~/cilktools-linux-004421/bin/cilkview ./ch_cilk_cv 1 n 4000

echo 
echo "Size n = 5000"
~/cilktools-linux-004421/bin/cilkview ./ch_cilk_cv 1 n 5000

echo 
echo "Size n = 6000"
~/cilktools-linux-004421/bin/cilkview ./ch_cilk_cv 1 n 6000