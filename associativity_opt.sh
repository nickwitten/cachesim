filen='associativity_opt.txt';
for trace in $(ls ./Traces); do
    for associativity in {1..100}; do
        echo "${trace} - ${associativity}" >> ${filen};
        ./cachesim ./Traces/${trace} 64 262144 ${associativity} | grep "Global Data Missrate" >> ${filen};
        echo >> ${filen};
    done;
    echo >> ${filen};
done;

