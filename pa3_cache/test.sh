#!/bin/bash
sim -bs 64 -is 8192 -ds 8192 -a 2 -wb -wa tex.trace >> tex.txt
sim -bs 64 -is 8192 -ds 8192 -a 2 -wb -nw tex.trace >> tex.txt
sim -bs 128 -is 8192 -ds 8192 -a 2 -wb -wa tex.trace >> tex.txt
sim -bs 128 -is 8192 -ds 8192 -a 2 -wb -nw tex.trace >> tex.txt
sim -bs 64 -is 8192 -ds 8192 -a 4 -wb -wa tex.trace >> tex.txt
sim -bs 64 -is 8192 -ds 8192 -a 4 -wb -nw tex.trace >> tex.txt
sim -bs 64 -is 16384 -ds 16384 -a 2 -wb -wa tex.trace >> tex.txt
sim -bs 64 -is 16384 -ds 16384 -a 2 -wb -nw tex.trace >> tex.txt
