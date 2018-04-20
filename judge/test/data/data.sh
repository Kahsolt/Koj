#!/bin/bash

case $1 in
  generate|g)
    for i in {1..10}
    do
      a=$RANDOM
      b=$RANDOM
      echo "$a $b" > $i.in
      echo -n "$[a+b]" > $i.out
    done
  ;;
  clean|c)
    rm *.in *.out
  ;;
  *)
    echo "Usage $0 generate(g)|clean(c)"
  ;;
esac
