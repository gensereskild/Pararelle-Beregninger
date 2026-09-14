#! /usr/bin/env bash
for i in $(seq 1 10); do
    time ./sequential &
done

wait
echo "all procceses finished"