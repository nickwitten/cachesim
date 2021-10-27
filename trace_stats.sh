for trace in $(ls ./Traces); do
    echo "${trace}" >> './trace_stats.txt';
    ./cachesim ./Traces/${trace} 64 262144 8 >> ./trace_stats.txt;
    echo "" >> './trace_stats.txt';
done;

