#!/bin/bash

DICTIONARY=dictionary.txt
DICTIONARY_HASH=dictionary_hash.txt
BIN_SEQUENTIAL=des-finder-seq
BIN_PARALLEL=des-finder-parallel
OUTPUT_FILE=output.log

if [[ ! -f "$DICTIONARY" ]]; then
    echo "Dictionary file not found!"
    exit 1
fi

if [[ ! -f "$DICTIONARY_HASH" ]]; then
    echo "Complete dictionary file not found!"
    exit 1
fi

if [[ ! -f "$BIN_SEQUENTIAL" ]]; then
    echo "Sequential binary not found!"
    exit 1
fi

if [[ ! -f "$BIN_PARALLEL" ]]; then
    echo "Parallel binary not found!"
    exit 1
fi

rm $OUTPUT_FILE

# The 1 is separated otherwhise I would get 101, 201 and so on..
SMALL_POSITIONS_ARRAY=( 1 $(seq 100 100 10000))
MEDIUM_POSITIONS_ARRAY=$(seq 10000 2000 200000)
BIG_POSITIONS_ARRAY=(200000 400000 700000 1000000 3000000 5000000)
POSITIONS=( ${SMALL_POSITIONS_ARRAY[@]} ${MEDIUM_POSITIONS_ARRAY[@]} ${BIG_POSITIONS_ARRAY[@]} )

MAX_THREADS=$((10+$(grep -c ^processor /proc/cpuinfo)))

echo "Testing on $MAX_THREADS threads.."

# Printf header to output file
printf "POSITION;SEQUENTIAL;" >> $OUTPUT_FILE
for THREADS in $(seq $MAX_THREADS); do
    printf "PARALLEL@%d;" $THREADS >> $OUTPUT_FILE
done
printf "\n" >> $OUTPUT_FILE

for POSITION in ${POSITIONS[@]}; do
    echo "Starting test for position $POSITION.."
    printf "$POSITION;" >> $OUTPUT_FILE

    # Print the "position" line and get the hash and the salt
    HASH=$(sed $POSITION"q;d" $DICTIONARY_HASH | awk -F ';' '{print $2}')
    SALT=$(sed $POSITION"q;d" $DICTIONARY_HASH | awk -F ';' '{print $3}')

    # Run the program with stopwatch and get the elapsed time by using tail
    # to extract the last line; the same is done for parallel
    echo "Starting search with sequential program"
    SEQUENTIAL_COMMAND="./$BIN_SEQUENTIAL $HASH $SALT -s"
    printf "%s;" "$($SEQUENTIAL_COMMAND | tail -1)" >> $OUTPUT_FILE

    for THREADS in $(seq $MAX_THREADS); do
        echo "Starting search with parallel program @ $THREADS threads"
        PARALLEL_COMMAND="./$BIN_PARALLEL $HASH $SALT -s -t $THREADS"
        printf "%s;" "$($PARALLEL_COMMAND | tail -1)" >> $OUTPUT_FILE
    done

    printf "\n" >> $OUTPUT_FILE
done
