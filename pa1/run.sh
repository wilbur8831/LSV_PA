true > log.txt
for f in ./EPFL-benchmark/*/*.aig; do
    true > run.dofile
    echo "$f" >> log.txt
    echo "read $f" >> run.dofile
    echo "lsv_print_msfc" >>  run.dofile
    ./abc -f run.dofile >> log.txt 
done
rm run.dofile